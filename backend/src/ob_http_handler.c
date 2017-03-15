/*
 * ob_http_handler.c
 *
 *  Created on: Mar 15, 2017
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
#include "ob_http_handler.h"
#include "db_handler.h"
#include "resource_handler.h"

#include "ob_campaign_handler.h"
#include "ob_destination_handler.h"
#include "ob_dialing_handler.h"
#include "ob_dl_handler.h"
#include "ob_plan_handler.h"

extern evhtp_t* g_htp;

///// outbound modules

static void cb_htp_ob_plans(evhtp_request_t *req, void *data);
static void cb_htp_ob_plans_uuid(evhtp_request_t *req, void *data);
static void cb_htp_ob_campaigns(evhtp_request_t *req, void *data);
static void cb_htp_ob_campaigns_uuid(evhtp_request_t *req, void *data);
static void cb_htp_ob_dlmas(evhtp_request_t *req, void *data);
static void cb_htp_ob_dlmas_uuid(evhtp_request_t *req, void *data);
static void cb_htp_ob_dls(evhtp_request_t *req, void *data);
static void cb_htp_ob_dls_uuid(evhtp_request_t *req, void *data);
static void cb_htp_ob_dialings(evhtp_request_t *req, void *data);
static void cb_htp_ob_dialings_uuid(evhtp_request_t *req, void *data);

// ob/destinations
static void cb_htp_ob_destinations(evhtp_request_t *req, void *data);
static void cb_htp_ob_destinations_uuid(evhtp_request_t *req, void *data);
static void htp_get_ob_destinations(evhtp_request_t *req, void *data);
static void htp_post_ob_destinations(evhtp_request_t *req, void *data);
static void htp_get_ob_destinations_uuid(evhtp_request_t *req, void *data);
static void htp_put_ob_destinations_uuid(evhtp_request_t *req, void *data);
static void htp_delete_ob_destinations_uuid(evhtp_request_t *req, void *data);

// ob/plans
static void htp_get_ob_plans(evhtp_request_t *req, void *data);
static void htp_post_ob_plans(evhtp_request_t *req, void *data);
static void htp_get_ob_plans_uuid(evhtp_request_t *req, void *data);
static void htp_put_ob_plans_uuid(evhtp_request_t *req, void *data);
static void htp_delete_ob_plans_uuid(evhtp_request_t *req, void *data);

// ob/campaigns
static void htp_get_ob_campaigns(evhtp_request_t *req, void *data);
static void htp_post_ob_campaigns(evhtp_request_t *req, void *data);
static void htp_get_ob_campaigns_uuid(evhtp_request_t *req, void *data);
static void htp_put_ob_campaigns_uuid(evhtp_request_t *req, void *data);
static void htp_delete_ob_campaigns_uuid(evhtp_request_t *req, void *data);

// ob/dlmas
static void htp_get_ob_dlmas(evhtp_request_t *req, void *data);
static void htp_post_ob_dlmas(evhtp_request_t *req, void *data);
static void htp_get_ob_dlmas_uuid(evhtp_request_t *req, void *data);
static void htp_put_ob_dlmas_uuid(evhtp_request_t *req, void *data);
static void htp_delete_ob_dlmas_uuid(evhtp_request_t *req, void *data);

// ob/dls
static void htp_get_ob_dls(evhtp_request_t *req, void *data);
static void htp_post_ob_dls(evhtp_request_t *req, void *data);
static void htp_get_ob_dls_uuid(evhtp_request_t *req, void *data);
static void htp_put_ob_dls_uuid(evhtp_request_t *req, void *data);
static void htp_delete_ob_dls_uuid(evhtp_request_t *req, void *data);

// ob/dialings
static void htp_get_ob_dialings(evhtp_request_t *req, void *data);
static void htp_get_ob_dialings_uuid(evhtp_request_t *req, void *data);
static void htp_delete_ob_dialings_uuid(evhtp_request_t *req, void *data);

bool init_ob_http_handler(void)
{

  ////// outbound modules
  // destinations
  evhtp_set_regex_cb(g_htp, "/ob/destinations/("DEF_REG_UUID")", cb_htp_ob_destinations_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/ob/destinations", cb_htp_ob_destinations, NULL);

  // plans
  evhtp_set_regex_cb(g_htp, "/ob/plans/("DEF_REG_UUID")", cb_htp_ob_plans_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/ob/plans", cb_htp_ob_plans, NULL);

  // campaigns
  evhtp_set_regex_cb(g_htp, "/ob/campaigns/("DEF_REG_UUID")", cb_htp_ob_campaigns_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/ob/campaigns", cb_htp_ob_campaigns, NULL);

  // dlmas
  evhtp_set_regex_cb(g_htp, "/ob/dlmas/("DEF_REG_UUID")", cb_htp_ob_dlmas_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/ob/dlmas", cb_htp_ob_dlmas, NULL);

  // dls
  evhtp_set_regex_cb(g_htp, "/ob/dls/("DEF_REG_UUID")", cb_htp_ob_dls_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/ob/dls", cb_htp_ob_dls, NULL);

  // dialings
  evhtp_set_regex_cb(g_htp, "/ob/dialings/("DEF_REG_UUID")", cb_htp_ob_dialings_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "/ob/dialings", cb_htp_ob_dialings, NULL);

  return true;

  return true;
}


/**
 * http request handler.
 * request : ^/ob/destinations
 * @param req
 * @param data
 */
static void cb_htp_ob_destinations(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_destinations.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_destinations(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_ob_destinations(req, data);
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
 * request : ^/ob/destinations/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_ob_destinations_uuid(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_destinations_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_destinations_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_ob_destinations_uuid(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_ob_destinations_uuid(req, data);
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
 * request : ^/ob/plans
 * @param req
 * @param data
 */
static void cb_htp_ob_plans(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_plans.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_plans(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_ob_plans(req, data);
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
 * request : ^/ob/plans/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_ob_plans_uuid(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_plans_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    slog(LOG_ERR, "Wrong method request. method[%d]", method);
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_plans_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_ob_plans_uuid(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_ob_plans_uuid(req, data);
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
 * request : ^/ob/campaigns
 * @param req
 * @param data
 */
static void cb_htp_ob_campaigns(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_campaigns.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_campaigns(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_ob_campaigns(req, data);
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
 * request : ^/ob/campaigns/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_ob_campaigns_uuid(evhtp_request_t *req, void *data)
{
  int method;
  const char* uuid;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_campaigns_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
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
    htp_get_ob_campaigns_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_ob_campaigns_uuid(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_ob_campaigns_uuid(req, data);
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
 * request : ^/ob/dlmas
 * @param req
 * @param data
 */
static void cb_htp_ob_dlmas(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_dlmas.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_dlmas(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_ob_dlmas(req, data);
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
 * request : ^/ob/dlmas/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_ob_dlmas_uuid(evhtp_request_t *req, void *data)
{
  int method;
  const char* uuid;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_dlmas_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
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
    htp_get_ob_dlmas_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_ob_dlmas_uuid(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_ob_dlmas_uuid(req, data);
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
 * request : ^/ob/dls
 * @param req
 * @param data
 */
static void cb_htp_ob_dls(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_dls.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_dls(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_ob_dls(req, data);
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
 * request : ^/ob/dls/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_ob_dls_uuid(evhtp_request_t *req, void *data)
{
  int method;
  const char* uuid;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_dls_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
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
    htp_get_ob_dls_uuid(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_ob_dls_uuid(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_ob_dls_uuid(req, data);
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
 * htp request handler.
 * request: GET ^/ob/destinations
 * @param req
 * @param data
 */
static void htp_get_ob_destinations(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_destinations.");

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
 * request: POST ^/ob/destinations
 * @param req
 * @param data
 */
static void htp_post_ob_destinations(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_post_ob_destinations.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get data from request.");
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
static void htp_get_ob_destinations_uuid(evhtp_request_t *req, void *data)
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
 * request: PUT ^/ob/destinations/<uuid>
 * @param req
 * @param data
 */
static void htp_put_ob_destinations_uuid(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_ob_destinations_uuid");

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
 * request: DELETE ^/ob/destinations/<uuid>
 * @param req
 * @param data
 */
static void htp_delete_ob_destinations_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_ob_destinations_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_destination(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not find correct ob_destination info. uuid[%s]", uuid);
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // delete info
  j_tmp = delete_ob_destination(uuid);
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
 * request: GET ^/ob/plans
 * @param req
 * @param data
 */
static void htp_get_ob_plans(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_plans.");

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
 * request: POST ^/ob/plans
 * @param req
 * @param data
 */
static void htp_post_ob_plans(evhtp_request_t *req, void *data)
{
  int ret;
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
  slog(LOG_DEBUG, "Fired htp_post_ob_plans.");

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

  // validate plan
  ret = validate_ob_plan(j_data);
  if(ret == false) {
    slog(LOG_DEBUG, "Could not pass the validation.");
    json_decref(j_data);
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
 * request: GET ^/ob/plans/<uuid>
 * @param req
 * @param data
 */
static void htp_get_ob_plans_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_plans_uuid.");

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
 * request: PUT ^/ob/plans/<uuid>
 * @param req
 * @param data
 */
static void htp_put_ob_plans_uuid(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_ob_plans_uuid.");

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
 * request: DELETE ^/ob/plans/<uuid>
 * @param req
 * @param data
 */
static void htp_delete_ob_plans_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_ob_plans_uuid.");

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

  // delete info
  j_tmp = delete_ob_plan(uuid);
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
 * request: GET ^/ob/campaignns
 * @param req
 * @param data
 */
static void htp_get_ob_campaigns(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_campaigns.");

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
 * request: POST ^/ob/campaigns
 * @param req
 * @param data
 */
static void htp_post_ob_campaigns(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_post_ob_campaigns.");

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
 * request: GET ^/ob/campaigns/<uuid>
 * @param req
 * @param data
 */
static void htp_get_ob_campaigns_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_campaigns_uuid.");

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
 * request: PUT ^/ob/campaigns/<uuid>
 * @param req
 * @param data
 */
static void htp_put_ob_campaigns_uuid(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_ob_campaigns_uuid.");

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
  j_tmp = update_ob_campaign(j_data);
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
 * request: DELETE ^/ob/campaigns/<uuid>
 * @param req
 * @param data
 */
static void htp_delete_ob_campaigns_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_ob_campaigns_uuid.");

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

  // delete info
  j_tmp = delete_ob_campaign(uuid);
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
 * request: GET ^/ob/dlmas
 * @param req
 * @param data
 */
static void htp_get_ob_dlmas(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_dlmas.");

  // get info
  j_tmp = get_ob_dlmas_all_uuid();
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
 * request: POST ^/ob/dlmas
 * @param req
 * @param data
 */
static void htp_post_ob_dlmas(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_post_ob_dlmas.");

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
  j_tmp = create_ob_dlma(j_data);
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
 * request: GET ^/ob/dlmas/<uuid>
 * @param req
 * @param data
 */
static void htp_get_ob_dlmas_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_dlmas_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_dlma(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get plan info
  j_tmp = get_ob_dlma(uuid);
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
 * request: PUT ^/ob/dlmas/<uuid>
 * @param req
 * @param data
 */
static void htp_put_ob_dlmas_uuid(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_ob_dlmas_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_dlma(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not find correct ob_dlma info. uuid[%s]", uuid);
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
  j_tmp = update_ob_dlma(j_data);
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
 * request: DELETE ^/ob/dlmas/<uuid>
 * @param req
 * @param data
 */
static void htp_delete_ob_dlmas_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_ob_dlmas_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_dlma(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not find correct ob_dlma info. uuid[%s]", uuid);
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // delete info
  j_tmp = delete_ob_dlma(uuid);
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
 * request: GET ^/ob/dls
 * @param req
 * @param data
 */
static void htp_get_ob_dls(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* dlma_uuid;
  const char* tmp_const;
  int count;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_dls.");

  // get params
  dlma_uuid = evhtp_kv_find(req->uri->query, "dlma_uuid");
  if(dlma_uuid == NULL) {
    slog(LOG_NOTICE, "Could not get correct dlma_uuid.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  count = 10000;  /// default count
  tmp_const = evhtp_kv_find(req->uri->query, "count");
  if(tmp_const != NULL) {
    count = atoi(tmp_const);
  }

  // get info
  j_tmp = get_ob_dls_all_uuid_by_dlma_count(dlma_uuid, count);
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
 * request: POST ^/ob/dls
 * @param req
 * @param data
 */
static void htp_post_ob_dls(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_post_ob_dls.");

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
  j_tmp = create_ob_dl(j_data);
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
 * request: GET ^/ob/dls/<uuid>
 * @param req
 * @param data
 */
static void htp_get_ob_dls_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_dls_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_dl(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get plan info
  j_tmp = get_ob_dl(uuid);
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
 * request: PUT ^/ob/dls/<uuid>
 * @param req
 * @param data
 */
static void htp_put_ob_dls_uuid(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_ob_dls_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_dl(uuid);
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
  j_tmp = update_ob_dl(j_data);
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
 * request: DELETE ^/ob/dls/<uuid>
 * @param req
 * @param data
 */
static void htp_delete_ob_dls_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_ob_dls_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_dl(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not find correct ob_dl info. uuid[%s]", uuid);
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // delete info
  j_tmp = delete_ob_dl(uuid);
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
 * http request handler.
 * request : ^/ob/dialings
 * @param req
 * @param data
 */
static void cb_htp_ob_dialings(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_dialings.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) ) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_ob_dialings(req, data);
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
 * htp request handler.
 * request: GET ^/ob/dialings
 * @param req
 * @param data
 */
static void htp_get_ob_dialings(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_ob_dialings.");

  // get info
  j_tmp = get_ob_dialings_all_uuid();
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
 * http request handler.
 * request : ^/ob/dialings/<uuid>
 * @param req
 * @param data
 */
static void cb_htp_ob_dialings_uuid(evhtp_request_t *req, void *data)
{
  int method;
  const char* uuid;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired cb_htp_ob_dialings_uuid.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
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
    htp_get_ob_dialings_uuid(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_ob_dialings_uuid(req, data);
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
 * htp request handler.
 * request: GET /ob/dialings/<uuid>
 * @param req
 * @param data
 */
static void htp_get_ob_dialings_uuid(evhtp_request_t *req, void *data)
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
  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get destination info
  j_tmp = get_ob_dialing(uuid);
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
 * request: DELETE ^/ob/dialings/<uuid>
 * @param req
 * @param data
 */
static void htp_delete_ob_dialings_uuid(evhtp_request_t *req, void *data)
{
  const char* uuid;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_ob_dialings_uuid.");

  // get uuid
  uuid = req->uri->path->file;
  if(uuid == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // check existence
  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not find correct ob_dlma info. uuid[%s]", uuid);
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // send hangup request of ob_dialing
  ret = send_ob_dialing_hangup_request(uuid);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}
