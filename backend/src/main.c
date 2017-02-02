

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <jansson.h>

#include "slog.h"
#include "config.h"
#include "ami_handler.h"
#include "http_handler.h"


struct json_t* g_app;
struct event_base*  g_base = NULL;


static bool init_event(void)
{
  if(g_base == NULL) {
    printf("New event base.\n");
    g_base = event_base_new();
  }

  return true;
}

bool init(void)
{
  int ret;
  
  ret = init_config();
  if(ret == false) {
    printf("Could not initiate config.");
    return false;
  }

  ret = init_log();
  if(ret == false) {
    printf("Failed initiate log.");
    return false;
  }

  ret = init_event();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate event.");
    return false;
  }
  
  ret = init_ami_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate ami_handle.");
    return false;
  }

  ret = init_http_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate http_handler.");
    return false;
  }
  
  return true;
}

bool terminate(void)
{
  slog(LOG_INFO, "Terimnating..");

  term_http_handler();

  event_base_free(g_base);

  return true;
}

int main(int argc, char** argv) 
{
  int ret;
  
  // initiate
  ret = init();
  if(ret == false) {
    printf("Could not initiate.\n");
    return 1;
  }
  slog(LOG_INFO, "Started backend service.");
    
  event_base_loop(g_base, 0);

  terminate();

  return 0;
}
