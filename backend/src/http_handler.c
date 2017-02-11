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
#include "resource_handler.h"

#include "ob_campaign_handler.h"
#include "ob_destination_handler.h"
#include "ob_dialing_handler.h"
#include "ob_dl_handler.h"
#include "ob_plan_handler.h"

#define API_VER "0.1"

#define DEF_REG_UUID "[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}"

extern app* g_app;
extern struct event_base* g_base;
evhtp_t* g_htp = NULL;

static json_t* create_default_result(int code);
static void simple_response_normal(evhtp_request_t *req, json_t* j_msg);
static void simple_response_error(evhtp_request_t *req, int status_code, int err_code, const char* err_msg);

static void cb_htp_ping(evhtp_request_t *req, void *a);
static void cb_htp_peers(evhtp_request_t *req, void *data);

static void cb_htp_destinations(evhtp_request_t *req, void *data);
static void cb_htp_destinations_uuid(evhtp_request_t *req, void *data);
static void cb_htp_plans(evhtp_request_t *req, void *data);
static void cb_htp_plans_uuid(evhtp_request_t *req, void *data);
static void cb_htp_campaigns(evhtp_request_t *req, void *data);
static void cb_htp_campaigns_uuid(evhtp_request_t *req, void *data);
static void cb_htp_dlmas(evhtp_request_t *req, void *data);

static void htp_get_destinations(evhtp_request_t *req, void *data);
static void htp_post_destinations(evhtp_request_t *req, void *data);
static void htp_get_destinations_uuid(evhtp_request_t *req, void *data);
static void htp_put_destinations_uuid(evhtp_request_t *req, void *data);

static void htp_get_plans(evhtp_request_t *req, void *data);
static void htp_post_plans(evhtp_request_t *req, void *data);
static void htp_get_plans_uuid(evhtp_request_t *req, void *data);
static void htp_put_plans_uuid(evhtp_request_t *req, void *data);

static void htp_get_campaigns(evhtp_request_t *req, void *data);
static void htp_post_campaigns(evhtp_request_t *req, void *data);
static void htp_get_campaigns_uuid(evhtp_request_t *req, void *data);
static void htp_put_campaigns_uuid(evhtp_request_t *req, void *data);



bool init_http_handler(void)
{
  g_htp = evhtp_new(g_base, NULL);
  evhtp_bind_socket(g_htp, "0.0.0.0", 8081, 1024);

  // register callback
  evhtp_set_regex_cb(g_htp, "/ping", cb_htp_ping, NULL);

  evhtp_set_regex_cb(g_htp, "/peers", cb_htp_peers, NULL);

  evhtp_set_regex_cb(g_htp, "/destinations/("DEF_REG_UUID")", cb_htp_destinations_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/destinations", cb_htp_destinations, NULL);

  evhtp_set_regex_cb(g_htp, "/plans/("DEF_REG_UUID")", cb_htp_plans_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/plans", cb_htp_plans, NULL);

  evhtp_set_regex_cb(g_htp, "/campaigns/("DEF_REG_UUID")", cb_htp_campaigns_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/campaigns", cb_htp_campaigns, NULL);

  evhtp_set_cb(g_htp, "/dlmas", cb_htp_dlmas, NULL);

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

static void simple_response_normal(evhtp_request_t *req, json_t* j_msg)
{
  char* res;

  if((req == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired simple_response_normal.");

  res = json_dumps(j_msg, JSON_ENCODE_ANY);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, EVHTP_RES_OK);
  sfree(res);

  return;
}

static void simple_response_error(evhtp_request_t *req, int status_code, int err_code, const char* err_msg)
{
  char* res;
  json_t* j_tmp;
  json_t* j_res;
  int code;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired simple_response_error.");

  // create default result
  j_res = create_default_result(status_code);

  // create error
  if(err_code == 0) {
    code = status_code;
  }
  else {
    code = err_code;
  }
  j_tmp = json_pack("{s:i, s:s}",
      "code",     code,
      "message",  err_msg? : ""
      );
  json_object_set_new(j_res, "error", j_tmp);

  res = json_dumps(j_res, JSON_ENCODE_ANY);
  json_decref(j_res);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, status_code);
  sfree(res);

  return;
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

  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // send response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

static void cb_htp_peers(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_peers.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    j_tmp = get_peers_all_name();
    if(j_tmp == NULL) {
      simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", j_tmp);

    simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  return;
}

/**
 * http request handler.
 * request : ^/destinations
 * @param req
 * @param data
 */
static void cb_htp_destinations(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_destinations.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_destinations(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_destinations(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * request : ^/destinations/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_destinations_uuid(evhtp_request_t *req, void *data)
{
  int method;
  const char* uuid;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_destinations_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // get uuid
  uuid = req->uri->path->match_start;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_destinations_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_destinations_uuid(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * request : ^/plans
 * @param req
 * @param data
 */
static void cb_htp_plans(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_plans.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_plans(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_plans(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * request : ^/plans/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_plans_uuid(evhtp_request_t *req, void *data)
{
  int method;
  const char* uuid;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_plans_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // get uuid
  uuid = req->uri->path->match_start;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_plans_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_plans_uuid(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/campaigns
 * @param req
 * @param data
 */
static void cb_htp_campaigns(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_campaigns.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_campaigns(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_campaigns(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/campaigns/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_campaigns_uuid(evhtp_request_t *req, void *data)
{
  int method;
  const char* uuid;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_campaigns_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // get uuid
  uuid = req->uri->path->match_start;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_campaigns_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_campaigns_uuid(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/dlmas
 * @param req
 * @param data
 */
static void cb_htp_dlmas(evhtp_request_t *req, void *data)
{
  char* res;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_dlmas.");

  // get all destinations
  j_tmp = get_dlmas_all();

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

/**
 * htp request handler.
 * request: GET /destinations
 * @param req
 * @param data
 */
static void htp_get_destinations(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_destinations.");

  // get info
  j_tmp = get_ob_destinations_all_uuid();
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST /destinations
 * @param req
 * @param data
 */
static void htp_post_destinations(evhtp_request_t *req, void *data)
{
  const char* uuid;
  const char* tmp_const;
  char* tmp;
  json_t* j_data;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_destination(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create json
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create destination
  j_tmp = create_ob_destination(j_data);
  json_decref(j_data);
  if(j_tmp == NULL) {
    slog(LOG_INFO, "Could not create ob_destination.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET /destinations/<uuid>
 * @param req
 * @param data
 */
static void htp_get_destinations_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_destination(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get destination info
  j_tmp = get_ob_destination(uuid);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: PUT /destinations/<uuid>
 * @param req
 * @param data
 */
static void htp_put_destinations_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  const char* tmp_const;
  char* tmp;
  json_t* j_data;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_destinations_uuid");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_destination(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create json
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update info
  json_object_set_new(j_data, "uuid", json_string(uuid));
  j_tmp = update_ob_destination(j_data);
  json_decref(j_data);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET /plans
 * @param req
 * @param data
 */
static void htp_get_plans(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_plans.");

  // get info
  j_tmp = get_ob_plans_all_uuid();
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST /plans
 * @param req
 * @param data
 */
static void htp_post_plans(evhtp_request_t *req, void *data)
{
  const char* uuid;
  const char* tmp_const;
  char* tmp;
  json_t* j_data;
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_plans.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create json
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create plan
  j_tmp = create_ob_plan(j_data);
  json_decref(j_data);
  if(j_tmp == NULL) {
    slog(LOG_INFO, "Could not create ob_plan.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET /plans/<uuid>
 * @param req
 * @param data
 */
static void htp_get_plans_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_plans_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_plan(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get plan info
  j_tmp = get_ob_plan(uuid);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: PUT /plans/<uuid>
 * @param req
 * @param data
 */
static void htp_put_plans_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  const char* tmp_const;
  char* tmp;
  json_t* j_data;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_plans_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_plan(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not find correct ob_plan info. uuid[%s]", uuid);
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create json
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update info
  json_object_set_new(j_data, "uuid", json_string(uuid));
  j_tmp = update_ob_plan(j_data);
  json_decref(j_data);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET /campaignns
 * @param req
 * @param data
 */
static void htp_get_campaigns(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_campaigns.");

  // get info
  j_tmp = get_ob_campaigns_all_uuid();
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST /campaigns
 * @param req
 * @param data
 */
static void htp_post_campaigns(evhtp_request_t *req, void *data)
{
  const char* uuid;
  const char* tmp_const;
  char* tmp;
  json_t* j_data;
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_campaigns.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create json
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create plan
  j_tmp = create_ob_campaign(j_data);
  json_decref(j_data);
  if(j_tmp == NULL) {
    slog(LOG_INFO, "Could not create ob_plan.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET /campaigns/<uuid>
 * @param req
 * @param data
 */
static void htp_get_campaigns_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_campaigns_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_campaign(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get plan info
  j_tmp = get_ob_campaign(uuid);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: PUT /campaigns/<uuid>
 * @param req
 * @param data
 */
static void htp_put_campaigns_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  const char* tmp_const;
  char* tmp;
  json_t* j_data;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_campaigns_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_campaign(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not find correct ob_campaign info. uuid[%s]", uuid);
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create json
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update info
  json_object_set_new(j_data, "uuid", json_string(uuid));
  j_tmp = update_ob_plan(j_data);
  json_decref(j_data);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}
