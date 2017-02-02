
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
extern struct json_t* g_app;

static struct termios tin;
static int ami_sock = 0;

static char g_ami_buffer[MAX_AMI_RECV_BUF_LEN];

//static void negotiate(int sock, unsigned char *buf, int len);
static void terminal_reset(void);
static void terminal_set(void);
static bool ami_login(void);
static void ami_message_handler(const char* msg);

static void cb_ami_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
 

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
  slog(LOG_DEBUG, "Fired ami_message_handler. msg[%s]", msg);
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
  
  serv_addr = json_string_value(json_object_get(g_app, "serv_addr"));
  serv_port = json_string_value(json_object_get(g_app, "serv_port"));
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
    printf("Could not login.\n");
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
  
  username = json_string_value(json_object_get(g_app, "username"));
  password = json_string_value(json_object_get(g_app, "password"));
  
  asprintf(&cmd, "Action: Login\r\nUsername: %s\r\nSecret: %s\r\n\r\n", username, password);
    
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
  struct ast_json* j_out;
  struct ast_json* j_tmp;
  int i, j;
  int ret;
  char  tmp[2048];
  char* key;
  char* value;
  char* dump;

  if(msg == NULL) {
    return NULL;
  }

  memset(tmp, 0x00, sizeof(tmp));
  j_out = ast_json_array_create();
  j_tmp = ast_json_object_create();
  for(i = 0, j = 0; i < strlen(msg); i++) {
    if((msg[i] == '\r') && (msg[i + 1] == '\n')) {
      // Check /r/n/r/n
      ret = strlen(tmp);
      if(ret == 0) {
        ret = ast_json_array_append(j_out, ast_json_deep_copy(j_tmp));
        AST_JSON_UNREF(j_tmp);
        j_tmp = NULL;

        j_tmp = ast_json_object_create();
        j = 0;
        i++;
        continue;
      }

      value = ast_strdup(tmp);
      dump = value;
      key = strsep(&value, ":");
      if(key == NULL) {
        ast_free(dump);
        continue;
      }

      trim(key);
      trim(value);
      ast_json_object_set(j_tmp, key, ast_json_string_create(value));

      ast_free(dump);
      memset(tmp, 0x00, sizeof(tmp));
      j = 0;
      i++;
      continue;
    }
    tmp[j] = msg[i];
    j++;
  }

  if(j_tmp != NULL) {
    AST_JSON_UNREF(j_tmp);
  }
  return j_out;
}

