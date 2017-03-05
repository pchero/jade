
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
#define MAX_AMI_RECV_BUF_LEN  4096

extern struct event_base* g_base;
extern app* g_app;

//static struct termios tin;
static int ami_sock = 0;

static char g_ami_buffer[MAX_AMI_RECV_BUF_LEN];

//static void negotiate(int sock, unsigned char *buf, int len);
//static void terminal_reset(void);
//static void terminal_set(void);
static bool ami_get_init_info(void);
static bool ami_login(void);


static void cb_ami_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);

//static void terminal_set(void)
//{
//  // save terminal configuration
//  tcgetattr(STDIN_FILENO, &tin);
//
//  static struct termios tlocal;
//  memcpy(&tlocal, &tin, sizeof(tin));
//  cfmakeraw(&tlocal);
//  tcsetattr(STDIN_FILENO,TCSANOW,&tlocal);
//}
//
//static void terminal_reset(void)
//{
//  // restore terminal upon exit
//  tcsetattr(STDIN_FILENO,TCSANOW,&tin);
//}

static void cb_ami_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  char buf[10];
  char* ami_msg;
  int ret;
  int len;

  // receive
  while(1) {
    ret = recv(ami_sock, buf, 1, 0);
    if(ret == -1) {
      if(errno == EAGAIN) {
        return;
      }

      slog(LOG_WARNING, "Could not receive correct message from the Asterisk. err[%d:%s]",
          errno, strerror(errno));
      return;
    }

    len = strlen(g_ami_buffer);
    g_ami_buffer[len] = buf[0];

    if(len < 4) {
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

bool init_ami_handler(void)
{
  struct sockaddr_in server;
  struct event* ev;
  const char* serv_addr;
  const char* serv_port;
  int port;
  int ret;
  int flag;
  
  if(g_app == NULL) {
    return false;
  }
  slog(LOG_DEBUG, "Fired init_ami_handler");
  
  serv_addr = json_string_value(json_object_get(g_app->j_conf, "serv_addr"));
  serv_port = json_string_value(json_object_get(g_app->j_conf, "serv_port"));
  if((serv_addr == NULL) || (serv_port == NULL)) {
    return false;
  }
  port = atoi(serv_port);
  slog(LOG_INFO, "Connecting to the Asterisk. addr[%s], port[%d]", serv_addr, port);

  if(ami_sock != 0) {
    close(ami_sock);
  }

  // create socket
  ami_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(ami_sock == -1) {
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
  ret = connect(ami_sock , (struct sockaddr *)&server, sizeof(server));
  if(ret < 0) {
    slog(LOG_WARNING, "Could not connect to the Asterisk. err[%d:%s]", errno, strerror(errno));
    return false;
  }
  slog(LOG_DEBUG, "Connected to Asterisk.");

  // get/set socket option
  flag = fcntl(ami_sock, F_GETFL, 0);
  flag = flag|O_NONBLOCK;
  ret = fcntl(ami_sock, F_SETFL, flag);
  slog(LOG_DEBUG, "Set the non-block option for the Asterisk socket. ret[%d]", ret);

//  // set terminal
//  terminal_set();
//  atexit(terminal_reset);
  
  // login
  ret = ami_login();
  if(ret == false) {
    slog(LOG_ERR, "Could not login.");
    return false;
  }
  
  ret = ami_get_init_info();
  if(ret == false) {
    slog(LOG_ERR, "Could not send init.");
    return false;
  }

  // add event
  ev = event_new(g_base, ami_sock, EV_READ | EV_PERSIST, cb_ami_handler, NULL);
  event_add(ev, NULL);
  
  return true;
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
    return NULL;
  }

  // Get action
  j_val = json_object_get(j_cmd, "Action");
  if(j_val == NULL) {
    slog(LOG_ERR, " not get the action.");
    return NULL;
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
  
  ret = send(ami_sock, cmd, strlen(cmd), 0);
  
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
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "SipPeers");
    return false;
  }
  json_decref(j_tmp);

  // queue status
  j_tmp = json_pack("{s:s}",
      "Action", "QueueStatus"
      );
  ret = send_ami_cmd(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "QueueStatus");
    return false;
  }
  json_decref(j_tmp);

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
    slog(LOG_ERR, "Could not send ami action. action[%s]", "Command");
    return false;
  }
  insert_action(json_string_value(json_object_get(j_tmp, "ActionID")), "command.databaseshow");
  json_decref(j_tmp);

  return true;
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
  char* cmd;
  const char* username;
  const char* password;

  if(g_app == NULL) {
    return false;
  }
  slog(LOG_DEBUG, "ami_login");

  username = json_string_value(json_object_get(g_app->j_conf, "username"));
  password = json_string_value(json_object_get(g_app->j_conf, "password"));

  asprintf(&cmd, "Action: Login\r\nUsername: %s\r\nSecret: %s\r\n\r\n", username, password);

  send_ami_cmd_raw(cmd);
  sfree(cmd);

  return true;
}




