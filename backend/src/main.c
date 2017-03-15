

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "config.h"
#include "ami_handler.h"
#include "db_handler.h"
//#include "db_sql_create.h"
#include "http_handler.h"
#include "event_handler.h"

#include "ob_event_handler.h"


app* g_app;
struct event_base*  g_base = NULL;

static bool init_event(void);
//static bool init_db(void);

static bool init_event(void)
{
  if(g_base == NULL) {
    printf("New event base.\n");
    g_base = event_base_new();
  }

  return true;
}

//static bool init_db(void)
//{
//  int ret;
//
//  // action
//  db_exec(g_sql_drop_action);
//  ret = db_exec(g_sql_create_action);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create table. table[%s]", "action");
//    return false;
//  }
//
//  // channel
//  db_exec(g_sql_drop_channel);
//  ret = db_exec(g_sql_create_channel);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create table. table[%s]", "channel");
//    return false;
//  }
//
//  // peer
//  db_exec(g_sql_drop_peer);
//  ret = db_exec(g_sql_create_peer);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create table. table[%s]", "peer");
//    return false;
//  }
//
//  // queue_param
//  db_exec(g_sql_drop_queue_param);
//  ret = db_exec(g_sql_create_queue_param);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create table. table[%s]", "queue_param");
//    return false;
//  }
//
//  // queue_member
//  db_exec(g_sql_drop_queue_member);
//  ret = db_exec(g_sql_create_queue_member);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create table. table[%s]", "queue_member");
//    return false;
//  }
//
//  // queue_member
//  db_exec(g_sql_drop_queue_entry);
//  ret = db_exec(g_sql_create_queue_entry);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create table. table[%s]", "queue_entry");
//    return false;
//  }
//
//  // asterisk database
//  db_exec(g_sql_drop_database);
//  ret = db_exec(g_sql_create_database);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create table. table[%s]", "database");
//    return false;
//  }
//
//  return true;
//}

bool init(void)
{
  int ret;
  
  g_app = calloc(sizeof(app), 1);
  g_app->j_conf = NULL;

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
  slog(LOG_DEBUG, "Finished init_log.");

  ret = init_event();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate event.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_event.");

  ret = db_init();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate db.");
    return false;
  }
  slog(LOG_DEBUG, "Finished db_init.");

//  ret = init_db();
//  if(ret == false) {
//    slog(LOG_WARNING, "Could not initiate database.");
//    return false;
//  }
//  slog(LOG_DEBUG, "Finished init_db.");

  ret = init_ami_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate ami_handle.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_ami_handler.");

  ret = init_http_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate http_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_http_handler.");
  
  ret = init_event_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate event_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_event_handler.");

  ret = init_outbound();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate ob_handler.");
    return false;
  }

  return true;
}

bool terminate(void)
{
  slog(LOG_INFO, "Terimnating..");

  term_http_handler();
  db_term();

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
