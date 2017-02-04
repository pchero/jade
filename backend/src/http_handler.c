/*
 * http_handler.c
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#include <jansson.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "http_handler.h"

#define API_VER "0.1"

extern app* g_app;
extern struct event_base* g_base;
evhtp_t* g_htp = NULL;

static void cb_htp_ping(evhtp_request_t *req, void *a);


bool init_http_handler(void)
{
  g_htp = evhtp_new(g_base, NULL);
  evhtp_bind_socket(g_htp, "0.0.0.0", 8081, 1024);

  // register callback
  evhtp_set_cb(g_htp, "/ping", cb_htp_ping, NULL);

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

  return;
}
