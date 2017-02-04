
/*!
  \file   ami_handler.c
  \brief  

  \author Sungtae Kim
  \date   Dec 20, 2013

 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
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

static struct termios tin;
static int ami_sock = 0;

static char g_ami_buffer[MAX_AMI_RECV_BUF_LEN];

//static void negotiate(int sock, unsigned char *buf, int len);
static void terminal_reset(void);
static void terminal_set(void);
static bool ami_login(void);
static bool ami_get_init_info(void);
static void ami_message_handler(const char* msg);

static json_t* parse_ami_msg(const char* msg);

static void cb_ami_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);

static void ami_event_peerentry(json_t* j_msg);

static void terminal_set(void) 
{
  // save terminal configuration
  tcgetattr(STDIN_FILENO, &tin);

  static struct termios tlocal;
  memcpy(&tlocal, &tin, sizeof(tin));
  cfmakeraw(&tlocal);
  tcsetattr(STDIN_FILENO,TCSANOW,&tlocal);
}
 
static void terminal_reset(void) 
{
  // restore terminal upon exit
  tcsetattr(STDIN_FILENO,TCSANOW,&tin);
}

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

static void ami_message_handler(const char* msg)
{
	json_t* j_msg;
	char* tmp;
	const char* event;

	if(msg == NULL) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return;
	}
  slog(LOG_DEBUG, "Fired ami_message_handler.");

  // message parse
  j_msg = parse_ami_msg(msg);
  if(j_msg == NULL) {
    slog(LOG_NOTICE, "Could not parse message. msg[%s]", msg);
    return;
  }

  // get event
  event = json_string_value(json_object_get(j_msg, "Event"));
  if(event == NULL) {
    slog(LOG_NOTICE, "Could not get message event. msg[%s]", msg);
    json_decref(j_msg);
    return;
  }
  slog(LOG_DEBUG, "Get event info. event[%s]", event);

  if(strcmp(event, "PeerEntry") == 0) {
    ami_event_peerentry(j_msg);
  }
  else {
    tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
    slog(LOG_DEBUG, "Could not find correct message parser. msg[%s]", tmp);
    sfree(tmp);
  }

  json_decref(j_msg);

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
  slog(LOG_DEBUG, "init_ami_handler");
  
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

  // set terminal
  terminal_set();
  atexit(terminal_reset);
  
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

int send_ami_cmd(const char* cmd)
{
  int ret;
  
  if(cmd == NULL) {
    printf("Wrong input parameter.\n");
    return -1;
  }
  
  ret = send(ami_sock, cmd, strlen(cmd), 0);
  
  return ret;
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
    
  send_ami_cmd(cmd);
  sfree(cmd);
  
  return true;
}

static bool ami_get_init_info(void)
{
  char* cmd;

  // campaign
  asprintf(&cmd, "Action: OutCampaignShow\r\n\r\n");
  send_ami_cmd(cmd);
  sfree(cmd);

  // plan
  asprintf(&cmd, "Action: OutPlanShow\r\n\r\n");
  send_ami_cmd(cmd);
  sfree(cmd);

  // dlma
  asprintf(&cmd, "Action: OutDlmaShow\r\n\r\n");
  send_ami_cmd(cmd);
  sfree(cmd);

  // destination
  asprintf(&cmd, "Action: OutDestinationShow\r\n\r\n");
  send_ami_cmd(cmd);
  sfree(cmd);

  // sip peers
  asprintf(&cmd, "Action: SipPeers\r\n\r\n");
  send_ami_cmd(cmd);
  sfree(cmd);

  return true;
}

/**
 *
 * @param msg
 * @return
 */
static json_t* parse_ami_msg(const char* msg)
{
	json_t* j_tmp;
	char tmp[4096];
	int ret;
	int i;
	int j;
  char* key;
  char* value;
  char* dump;

	if(msg == NULL) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return NULL;
	}

  j_tmp = json_object();
  bzero(tmp, sizeof(tmp));
  for(i = 0, j = 0; i < strlen(msg); i++) {
    tmp[j] = msg[i];
    j++;

    if((msg[i] != '\r') || (msg[i+1] != '\n')) {
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

		ret = strcmp(key, "Variable");
		if(ret == 0) {
			if(json_object_get(j_tmp, "Variable") == NULL) {
				json_object_set_new(j_tmp, "Variable", json_array());
			}
			json_array_append_new(json_object_get(j_tmp, "Variable"), json_string(value));
		}
		else {
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

static void ami_event_peerentry(json_t* j_msg)
{
  slog(LOG_DEBUG, "Fired ami_event_peerentry.");
  return;
}

