

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <jansson.h>

#include "ami_handler.h"
#include "slog.h"

struct json_t* g_app;
struct event_base*  g_base = NULL;

void set_config(void)
{
  g_app = json_object();
  
  json_object_set(g_app, "username", json_string("admin"));
  json_object_set(g_app, "password", json_string("admin"));
  json_object_set(g_app, "serv_addr", json_string("10.12.118.11"));
  json_object_set(g_app, "serv_port", json_string("5038"));
  json_object_set(g_app, "loglevel", json_string("7"));
  
  return;
}


bool init(void)
{
  int ret;
  
  ret = init_log();
  if(ret == false) {
    printf("Failed initiate log.\n");
    return false;
  }

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
  
  printf("Hello world.\n");
  set_config();
  
  ret = init();
  if(ret == false) {
    printf("Could not initiate.\n");
    return 1;
  }
  slog(LOG_INFO, "Started backend service.");
    
  event_base_loop(g_base, 0);
    
  return 0;
}
