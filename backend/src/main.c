

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <jansson.h>

#include "ami_handler.h"
#include "slog.h"

struct json_t* g_app;

struct event_base*  g_base = NULL;
evhtp_t* g_htp;

void cb_htp_ping(evhtp_request_t *req, void *a);


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

static bool init_event(void)
{
  if(g_base == NULL) {
    printf("New event base.\n");
    g_base = event_base_new();
  }
  g_htp = evhtp_new(g_base, NULL);
  evhtp_bind_socket(g_htp, "0.0.0.0", 8081, 1024);

  evhtp_set_cb(g_htp, "/ping", cb_htp_ping, NULL);

  return true;
}

bool init(void)
{
  int ret;
  
  ret = init_log();
  if(ret == false) {
    printf("Failed initiate log.\n");
    return false;
  }

  ret = init_event();
  if(ret == false) {
    slog(LOG_DEBUG, "Could not initiate event.");
    return false;
  }
  
  ret = init_ami_handler();
  if(ret == false) {
    printf("Failed initiate ami_handle.\n");
    return false;
  }
  
  return true;
}

void cb_htp_ping(evhtp_request_t *req, void *a)
{
  slog(LOG_DEBUG, "cb_htp_ping");

  evbuffer_add_printf(req->buffer_out, "%s", "pong");
  evhtp_send_reply(req, EVHTP_RES_OK);
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

  evhtp_unbind_socket(g_htp);
  evhtp_free(g_htp);
  event_base_free(g_base);


  return 0;
}
