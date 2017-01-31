

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <jansson.h>

#include "ami_handler.h"

struct event_base*  g_base = NULL;

struct json_t* g_app;

/*
static void cb_sample(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  int ret;
  struct event* ev;
  
  ev = event_new(g_base, ami_sock, EV_READ | EV_PERSIST, cb_ami_handler, NULL);
  event_add(ev, NULL);

  tmp_const = "Action: Login\r\nUsername: admin\r\nSecret:admin\r\n";
  ret = send_ami_cmd(tmp_const);
  
  
  return;
}
*/

void set_config(void)
{
  g_app = json_object();
  
  json_object_set(g_app, "username", json_string("admin"));
  json_object_set(g_app, "password", json_string("admin"));
  json_object_set(g_app, "serv_addr", json_string("192.168.200.10"));
  json_object_set(g_app, "serv_port", json_string("5038"));
  
  return;
}


bool init(void)
{
  int ret;
  
  if(g_base == NULL) {
		printf("New event base.\n");
		g_base = event_base_new();
	}
  
  ret = init_ami_handler();
  if(ret == false) {
    printf("Failed initiate ami_handle.\n");
    return false;
  }
  
  return true;
}

int main(int argc, char** argv) 
{
  int ret;
//  const char* tmp_const;
  
  printf("Hello world.\n");
  set_config();
  
  ret = init();
  if(ret == false) {
    printf("Could not initiate.\n");
    return 1;
  }
    
  event_base_loop(g_base, 0);
    
  return 0;
}
