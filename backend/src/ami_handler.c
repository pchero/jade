
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

extern struct event_base* g_base;
extern struct json_t* g_app;

static struct termios tin;
static int ami_sock = 0;

static int flg_login = false;

//static void negotiate(int sock, unsigned char *buf, int len);
static void terminal_reset(void);
static void terminal_set(void);
static bool ami_login(void);

static void cb_ami_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);


void FREE(char* tmp)
{
  if(tmp == NULL) {
    return;
  }
  
  free(tmp);
  
  return;
}
 
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
  int ret;
    
  // recv 1
  ret = recv(ami_sock, buf, 1, 0);
  if(ret == -1) {
    printf("Error!\n");
    return;
  }
  printf("%c", buf[0]);
  if(buf[0] == DLE) {
    // reconnect
    init_ami_handler();
    return;
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
  
  if(g_app == NULL) {
    return false;
  }
  
  serv_addr = json_string_value(json_object_get(g_app, "serv_addr"));
  serv_port = json_string_value(json_object_get(g_app, "serv_port"));
  if((serv_addr == NULL) || (serv_port == NULL)) {
    return false;
  }
  port = atoi(serv_port);

  if(ami_sock != 0) {
    close(ami_sock);
  }
  
  ami_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(ami_sock == -1) {
    printf("Could not create socket.\n");
    return false;
  }
  
  server.sin_addr.s_addr = inet_addr(serv_addr);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
 
  //Connect to remote server
  ret = connect(ami_sock , (struct sockaddr *)&server, sizeof(server));
  if(ret < 0) {
      perror("connect failed. Error");
      return 1;
  }
  printf("Connected.\n");

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
  
  username = json_string_value(json_object_get(g_app, "username"));
  password = json_string_value(json_object_get(g_app, "password"));
  
  asprintf(&cmd, "Action: Login\r\nUsername: %s\r\nSecret: %s\r\n\r\n", username, password);
    
  send_ami_cmd(cmd);
  FREE(cmd);
  
  return true;
}

