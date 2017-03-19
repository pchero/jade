
/*!
  \file   ami_handler.c
  \brief  

  \author Sungtae Kim
  \date   Dec 20, 2013

 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>
#include <stdbool.h>
#include <event2/event.h>
#include <jansson.h>
#include <errno.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "ami_handler.h"
#include "action_handler.h"
#include "db_handler.h"
#include "db_sql_create.h"
#include "ob_ami_handler.h"
#include "ob_dialing_handler.h"
#include "ami_event_handler.h"

#define DLE     0x0f  // Data link escape
#define DO      0xfd
#define WONT    0xfc
#define WILL    0xfb
#define DONT    0xfe
#define CMD     0xff

#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31

#define BUFLEN 20
#define MAX_AMI_RECV_BUF_LEN  409600

extern struct event_base* g_base;
extern app* g_app;

static int g_ami_sock = 0;
static char g_ami_buffer[MAX_AMI_RECV_BUF_LEN];
struct event* g_ev_ami_handler = NULL;

static bool is_ev_ami_handler_running(void);
static void free_ev_ami_handler(void);
static void update_ev_ami_handler(struct event* ev);
static void release_ami_connection(void);

static bool ami_connect(void);
static bool ami_get_init_info(void);
static bool ami_login(void);
static bool init_ami_database(void);
static bool init_ami_connect(void);

static void cb_ami_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_ami_connect(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);


static void cb_ami_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  char buf[10];
  char* ami_msg;
  int ret;
  int len;

  ret = is_ev_ami_handler_running();
  if(ret == false) {
    return;
  }

  // receive
  while(1) {
    ret = recv(g_ami_sock, buf, 1, 0);
    if(ret != 1) {
      if(errno == EAGAIN) {
        return;
      }

      // something was wrong. update connected status
      slog(LOG_WARNING, "Could not receive correct message from the Asterisk. err[%d:%s]", errno, strerror(errno));
      bzero(g_ami_buffer, sizeof(g_ami_buffer));
      release_ami_connection();
      return;
    }

    len = strlen(g_ami_buffer);
    if(len >= sizeof(g_ami_buffer)) {
      slog(LOG_ERR, "Too much big data. Just clean up the buffer. size[%d]", len);
      bzero(g_ami_buffer, sizeof(g_ami_buffer));
      continue;
    }

    g_ami_buffer[len] = buf[0];
    if(len < 4) {
      // not ready to check the end of message.
      continue;
    }

    // check received all ami message
    len = strlen(g_ami_buffer);
    if((g_ami_buffer[len-4] == '\r')
        && (g_ami_buffer[len-3] == '\n')
        && (g_ami_buffer[len-2] == '\r')
        && (g_ami_buffer[len-1] == '\n')
        ) {
      ami_msg = strdup(g_ami_buffer);
      bzero(g_ami_buffer, sizeof(g_ami_buffer));

      ami_message_handler(ami_msg);
      sfree(ami_msg);
    }
  }

  return;
}

/**
 * Check the ami connection info.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_ami_connection_check(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  char* cmd;

  slog(LOG_DEBUG, "Fired cb_ami_connection_check.");

  asprintf(&cmd, "Action: Ping\r\n\r\n");
  send_ami_cmd_raw(cmd);
  free(cmd);

  return;
}

/**
 * Connect to ami.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_ami_connect(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  int ret;
  struct event* ev;

  ret = is_ev_ami_handler_running();
  if(ret == true) {
    return;
  }
  slog(LOG_NOTICE, "Fired cb_ami_connect. connected[%d]", ret);

  // ami connect
  ret = ami_connect();
  if(ret == false) {
    slog(LOG_ERR, "Could not connect to asterisk ami.");
    return;
  }

  // add ami event handler
  ev = event_new(g_base, g_ami_sock, EV_READ | EV_PERSIST, cb_ami_handler, NULL);
  event_add(ev, NULL);

  // update event ami handler
  update_ev_ami_handler(ev);

}

/**
 * Initiate ami_handler.
 * @return
 */
bool init_ami_handler(void)
{
  struct timeval tm_event;
  struct event* ev;

  tm_event.tv_sec = 1;
  tm_event.tv_usec = 0;
  
  // ami connect
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_ami_connect, NULL);
  event_add(ev, &tm_event);

  // check ami connection
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_ami_connection_check, NULL);
  event_add(ev, &tm_event);

  return true;
}

/**
 * Terminate ami handler.
 * @return
 */
void term_ami_handler(void)
{
  slog(LOG_DEBUG, "Fired term_ami_handler.");
  release_ami_connection();
  return;
}

/**
 * AMI command msg send handler.
 * send_ami_cmd
 * @param data
 */
bool send_ami_cmd(json_t* j_cmd)
{
  int ret;
  json_t* j_val;
  const char* key;
  int type;
  char* tmp;
  const char* tmp_const;

  char* cmd;
  char* cmd_sub;

  // just for log
  if(j_cmd == NULL) {
    return false;
  }

  // Get action
  j_val = json_object_get(j_cmd, "Action");
  if(j_val == NULL) {
    slog(LOG_ERR, " not get the action.");
    return false;
  }

  asprintf(&cmd, "Action: %s\r\n", json_string_value(j_val));
  slog(LOG_DEBUG, "AMI Action command. action[%s]", json_string_value(j_val));

  json_object_foreach(j_cmd, key, j_val){
    ret = strcmp(key, "Action");
    if(ret == 0) {
      continue;
    }

    ret = strcmp(key, "Variables");
    if(ret == 0) {
      tmp_const = json_string_value(j_val);
      cmd_sub = get_variables_info_ami_str_from_string(tmp_const);
      if(tmp != NULL) {
        asprintf(&tmp, "%s%s\r\n", cmd, cmd_sub);
        sfree(cmd);
        cmd = tmp;
      }
      continue;
    }

    type = json_typeof(j_val);
    switch(type) {
      case JSON_REAL:
      case JSON_INTEGER:
      {
        asprintf(&cmd_sub, "%s: %lld", key, json_integer_value(j_val));
      }
      break;

      case JSON_FALSE:
      case JSON_TRUE:
      case JSON_STRING:
      {
        asprintf(&cmd_sub, "%s: %s", key, json_string_value(j_val));
      }
      break;

      default:
      {
        slog(LOG_WARNING, "Invalid type. Set to <unknown>. type[%d]", type);
        asprintf(&cmd_sub, "%s: %s", key, "<unknown>");
      }
      break;
    }

    asprintf(&tmp, "%s%s\r\n", cmd, cmd_sub);
    sfree(cmd_sub);
    sfree(cmd);
    cmd = tmp;
  }

  asprintf(&tmp, "%s\r\n", cmd);
  sfree(cmd);

  cmd = tmp;
  ret = send_ami_cmd_raw(cmd);
  sfree(cmd);

  return true;
}

int send_ami_cmd_raw(const char* cmd)
{
  int ret;
  
  if(cmd == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return -1;
  }
  
  ret = send(g_ami_sock, cmd, strlen(cmd), 0);
  if(ret < 0) {
    release_ami_connection();
  }
  
  return ret;
}

static bool ami_get_init_info(void)
{
  json_t* j_tmp;
  int ret;
  char* action_id;

  // sip peers
  j_tmp = json_pack("{s:s}",
      "Action", "SipPeers"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "SipPeers");
    return false;
  }

  // queue status
  j_tmp = json_pack("{s:s}",
      "Action", "QueueStatus"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "QueueStatus");
    return false;
  }

  // database
  action_id = gen_uuid();
  j_tmp = json_pack("{s:s, s:s, s:s}",
      "Action",   "Command",
      "Command",  "database show",
      "ActionID", action_id
      );
  sfree(action_id);
  ret = send_ami_cmd(j_tmp);
  if(ret == false) {
    json_decref(j_tmp);
    slog(LOG_ERR, "Could not send ami action. action[%s]", "Command");
    return false;
  }
  insert_action(json_string_value(json_object_get(j_tmp, "ActionID")), "command.databaseshowall");
  json_decref(j_tmp);

  // registry
  j_tmp = json_pack("{s:s}",
      "Action", "SIPshowregistry"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "SIPshowregistry");
    return false;
  }

  return true;
}

/**
 * Return the event ami handler is running.
 * @return
 */
static bool is_ev_ami_handler_running(void)
{
  if(g_ev_ami_handler == NULL) {
    return false;
  }

  return true;
}

/**
 * Release ami connection info.
 */
static void release_ami_connection(void)
{
  slog(LOG_NOTICE, "Fired release_ami_connection.");

  free_ev_ami_handler();
  if(g_ami_sock != -1) {
    close(g_ami_sock);
  }
  g_ami_sock = -1;
  return;
}

/**
 * Free the event ami handler.
 */
static void free_ev_ami_handler(void)
{
  int ret;

  slog(LOG_NOTICE, "Fired free_ev_ami_handler.");

  ret = is_ev_ami_handler_running();
  if(ret == false) {
    return;
  }

  // free
  event_free(g_ev_ami_handler);
  g_ev_ami_handler = NULL;

  return;
}

/**
 * update ev_ami_handler.
 * @param ev
 */
static void update_ev_ami_handler(struct event* ev)
{
  if(ev == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired update_ev_ami_handler.");

  free_ev_ami_handler();

  g_ev_ami_handler = ev;
}


/**
 *
 * @param msg
 * @return
 */
json_t* parse_ami_msg(const char* msg)
{
	json_t* j_tmp;
	char tmp[4096];
	int ret;
	int i;
	int j;
  char* key;
  char* value;
  char* dump;
  int parsed;

	if(msg == NULL) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return NULL;
	}

  j_tmp = json_object();
  bzero(tmp, sizeof(tmp));
  for(i = 0, j = 0; i < strlen(msg); i++) {
    parsed = false;

    if((msg[i] != '\r') || (msg[i+1] != '\n')) {
      tmp[j] = msg[i];
      j++;
      continue;
    }

		// check /r/n/r/n
		ret = strlen(tmp);
		if(ret == 0) {
			break;
		}

		// get key/value
		value = strdup(tmp);
		dump = value;
		key = strsep(&value, ":");
		if(key == NULL) {
			sfree(dump);
			continue;
		}
		trim(key);
		trim(value);

		// check Variable
		ret = strcasecmp(key, "Variable");
		if(ret == 0) {
			if(json_object_get(j_tmp, "Variable") == NULL) {
				json_object_set_new(j_tmp, "Variable", json_array());
			}
			json_array_append_new(json_object_get(j_tmp, "Variable"), json_string(value));
			parsed = true;
		}

		// check Output
		ret = strcasecmp(key, "Output");
		if(ret == 0) {
		  if(json_object_get(j_tmp, "Output") == NULL) {
		    json_object_set_new(j_tmp, "Output", json_array());
		  }
		  json_array_append_new(json_object_get(j_tmp, "Output"), json_string(value));
		  parsed = true;
		}

		if(parsed != true) {
			json_object_set_new(j_tmp, key, json_string(value));
		}


		sfree(dump);
		memset(tmp, 0x00, sizeof(tmp));
		j = 0;
		i++;
		continue;
	}

  return j_tmp;
}

static bool ami_login(void)
{
  int ret;
  char* cmd;
  const char* username;
  const char* password;

  if(g_app == NULL) {
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_login");

  username = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_username"));
  password = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_password"));

  asprintf(&cmd, "Action: Login\r\nUsername: %s\r\nSecret: %s\r\n\r\n", username, password);

  ret = send_ami_cmd_raw(cmd);
  sfree(cmd);
  if(ret == false) {
    slog(LOG_ERR, "Could not login.");
    return false;
  }
  slog(LOG_NOTICE, "The ami login result. res[%d]", ret);

  return true;
}

static bool ami_connect(void)
{
  int ret;

  // init ami connect
  ret = init_ami_connect();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ami connection.");
    return false;
  }

  // init ami database
  ret = init_ami_database();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ami database.");
    return false;
  }

  // login
  ret = ami_login();
  if(ret == false) {
    slog(LOG_ERR, "Could not login.");
    return false;
  }

  // bad idea
  // but after login to asterisk, we need to wait for second before sending a command.
  sleep(1);

  // send get all initial ami request
  ret = ami_get_init_info();
  if(ret == false) {
    slog(LOG_ERR, "Could not send init info.");
    return false;
  }

  return true;
}

static bool init_ami_connect(void)
{
  const char* serv_addr;
  const char* serv_port;
  int port;
  struct sockaddr_in server;
  int ret;
  int flag;

  serv_addr = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_serv_addr"));
  serv_port = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_serv_port"));
  if((serv_addr == NULL) || (serv_port == NULL)) {
    return false;
  }
  port = atoi(serv_port);
  slog(LOG_INFO, "Connecting to the Asterisk. addr[%s], port[%d]", serv_addr, port);

  if(g_ami_sock != -1) {
    close(g_ami_sock);
  }

  // create socket
  g_ami_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(g_ami_sock == -1) {
    printf("Could not create socket.\n");
    return false;
  }
  slog(LOG_DEBUG, "Created socket to Asterisk.");

  // get server info
  server.sin_addr.s_addr = inet_addr(serv_addr);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  //Connect to remote server
  bzero(g_ami_buffer, sizeof(g_ami_buffer));
  ret = connect(g_ami_sock , (struct sockaddr *)&server, sizeof(server));
  if(ret < 0) {
    slog(LOG_WARNING, "Could not connect to the Asterisk. err[%d:%s]", errno, strerror(errno));
    return false;
  }
  slog(LOG_DEBUG, "Connected to Asterisk.");

  // get/set socket option
  flag = fcntl(g_ami_sock, F_GETFL, 0);
  flag = flag|O_NONBLOCK;
  ret = fcntl(g_ami_sock, F_SETFL, flag);
  slog(LOG_DEBUG, "Set the non-block option for the Asterisk socket. ret[%d]", ret);

  return true;
}

static bool init_ami_database(void)
{
  int ret;

  // action
  db_exec(g_sql_drop_action);
  ret = db_exec(g_sql_create_action);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "action");
    return false;
  }

  // channel
  db_exec(g_sql_drop_channel);
  ret = db_exec(g_sql_create_channel);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "channel");
    return false;
  }

  // peer
  db_exec(g_sql_drop_peer);
  ret = db_exec(g_sql_create_peer);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "peer");
    return false;
  }

  // queue_param
  db_exec(g_sql_drop_queue_param);
  ret = db_exec(g_sql_create_queue_param);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_param");
    return false;
  }

  // queue_member
  db_exec(g_sql_drop_queue_member);
  ret = db_exec(g_sql_create_queue_member);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_member");
    return false;
  }

  // queue_member
  db_exec(g_sql_drop_queue_entry);
  ret = db_exec(g_sql_create_queue_entry);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_entry");
    return false;
  }

  // asterisk database
  db_exec(g_sql_drop_database);
  ret = db_exec(g_sql_create_database);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "database");
    return false;
  }

  db_exec(g_sql_drop_registry);
  ret = db_exec(g_sql_create_registry);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "registry");
    return false;
  }

  return true;
}


