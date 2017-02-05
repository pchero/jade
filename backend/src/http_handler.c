/*
 * http_handler.c
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE
#include <jansson.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "http_handler.h"
#include "db_handler.h"

#define API_VER "0.1"

extern app* g_app;
extern struct event_base* g_base;
evhtp_t* g_htp = NULL;

static void cb_htp_ping(evhtp_request_t *req, void *a);
static void cb_htp_destinations(evhtp_request_t *req, void *data);
static void cb_htp_plans(evhtp_request_t *req, void *data);

static json_t* get_destinations_all(void);
static json_t* get_plans_all(void);

bool init_http_handler(void)
{
  g_htp = evhtp_new(g_base, NULL);
  evhtp_bind_socket(g_htp, "0.0.0.0", 8081, 1024);

  // register callback
  evhtp_set_cb(g_htp, "/ping", cb_htp_ping, NULL);
  evhtp_set_cb(g_htp, "/destinations", cb_htp_destinations, NULL);
  evhtp_set_cb(g_htp, "/plans", cb_htp_plans, NULL);

  return true;
}

void term_http_handler(void)
{
  slog(LOG_INFO, "Terminate http handler.");
  if(g_htp != NULL) {
    evhtp_unbind_socket(g_htp);
    evhtp_free(g_htp);
  }
}

static json_t* create_default_result(int code)
{
  json_t* j_res;
  char* timestamp;

  timestamp = get_utc_timestamp();

  j_res = json_pack("{s:s, s:s, s:i}",
      "api_ver",    API_VER,
      "timestamp",  timestamp,
      "statuscode", code
      );
  sfree(timestamp);

  return j_res;
}

static void cb_htp_ping(evhtp_request_t *req, void *a)
{
  char* res;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ping");

  // create result
  j_tmp = json_pack("{s:s}",
      "message",  "pong"
      );

  j_res = create_default_result(200);
  json_object_set_new(j_res, "result", j_tmp);

  res = json_dumps(j_res, JSON_ENCODE_ANY);
  json_decref(j_res);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, EVHTP_RES_OK);
  sfree(res);

  return;
}

static void cb_htp_destinations(evhtp_request_t *req, void *data)
{
  char* res;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_destinations.");

  // get all destinations
  j_tmp = get_destinations_all();

  // create result
  j_res = create_default_result(200);
  json_object_set_new(j_res, "result", j_tmp);

  res = json_dumps(j_res, JSON_ENCODE_ANY);
  json_decref(j_res);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, EVHTP_RES_OK);
  sfree(res);

  return;
}

static void cb_htp_plans(evhtp_request_t *req, void *data)
{
  char* res;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_plans.");

  // get all destinations
  j_tmp = get_plans_all();

  // create result
  j_res = create_default_result(200);
  json_object_set_new(j_res, "result", j_tmp);

  res = json_dumps(j_res, JSON_ENCODE_ANY);
  json_decref(j_res);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, EVHTP_RES_OK);
  sfree(res);

  return;
}

static json_t* get_destinations_all(void)
{
  char* sql;
  const char* tmp_const;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_tmp;

  asprintf(&sql, "select * from destination;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct destination.");
    return NULL;
  }

  j_res = json_object();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
    if(tmp_const == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_object_set_new(j_res, tmp_const, j_tmp);
  }
  db_free(db_res);

  return j_res;
}

static json_t* get_plans_all(void)
{
  char* sql;
  const char* tmp_const;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_tmp;

  asprintf(&sql, "select * from plan;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct plan.");
    return NULL;
  }

  j_res = json_object();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
    if(tmp_const == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_object_set_new(j_res, tmp_const, j_tmp);
  }
  db_free(db_res);

  return j_res;
}


