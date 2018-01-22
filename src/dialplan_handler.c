/*
 * dialplan_handler.c
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <jansson.h>

#include "slog.h"
#include "utils.h"
#include "conf_handler.h"
#include "http_handler.h"
#include "resource_handler.h"


#define DEF_DIALPLAN_CONFNAME   "extensions.conf"

static bool create_dpma_info(const json_t* j_data);
static bool delete_dpma_info(const char* uuid);
static bool update_dpma_info(const char* uuid, const json_t* j_data);



/**
 * GET ^/dp/setting request handler.
 * @param req
 * @param data
 */
void htp_get_dp_setting(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* format;
  char* tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_dp_setting.");

  // the only text format support.
  format = evhtp_kv_find(req->uri->query, "format");
  if((format == NULL) || (strcmp(format, "text") != 0)) {
    slog(LOG_NOTICE, "Could not get correct format.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  tmp = get_ast_current_config_info_raw(DEF_DIALPLAN_CONFNAME);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not get dialplan conf.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_string(tmp));
  sfree(tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * PUT ^/dp/setting request handler.
 * @param req
 * @param data
 */
void htp_put_dp_setting(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* tmp;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_dp_setting.");

  tmp = get_text_from_request_data(req);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not get data.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update setting
  ret = update_ast_current_config_info_raw(DEF_DIALPLAN_CONFNAME, tmp);
  sfree(tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dialplan setting info.");
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

/**
 * GET ^/dp/dpmas request handler.
 * @param req
 * @param data
 */
void htp_get_dp_dpmas(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_dp_dpmas.");

  j_tmp = get_dp_dpmas_all();
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/dp/dpmas$
 * @param req
 * @param data
 */
void htp_post_dp_dpmas(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_dp_dpmas.");

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    // no request data
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create dpma info
  ret = create_dpma_info(j_data);
  json_decref(j_data);
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

/**
 * GET ^/dp/dpmas/<detail> request handler.
 * @param req
 * @param data
 */
void htp_get_dp_dpmas_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* key;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_dp_dpmas_detail.");

  // key parse
  tmp_const = req->uri->path->file;
  key = uri_decode(tmp_const);
  if(key == NULL) {
    slog(LOG_ERR, "Could not get key info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_dp_dpma_info(key);
  sfree(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dpma info.");
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
 * PUT ^/dp/dpmas/<detail> request handler.
 * @param req
 * @param data
 */
void htp_put_dp_dpmas_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_dp_dpmas_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    sfree(detail);
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update info
  ret = update_dpma_info(detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dp_dpma info.");
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

/**
 * DELETE ^/dp/dpmas/<detail> request handler.
 * @param req
 * @param data
 */
void htp_delete_dp_dpmas_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_dp_dpmas_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete dpma
  ret = delete_dpma_info(detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete detail info.");
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * Delete dpma.
 * @return
 */
static bool update_dpma_info(const char* uuid, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_dpma_info.");

  j_tmp = json_deep_copy(j_data);
  json_object_del(j_tmp, "uuid");
  json_object_del(j_tmp, "tm_create");
  json_object_del(j_tmp, "tm_update");

  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  timestamp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(timestamp));
  sfree(timestamp);

  ret = update_dp_dpma_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dpma info.");
    return false;
  }

  return true;
}

/**
 * Delete dpma.
 * @return
 */
static bool delete_dpma_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_dpma_info. uuid[%s]", uuid);

  ret = delete_dp_dpma_info(uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete dpma info.");
    return false;
  }

  return true;
}

/**
 * Create dpma.
 * @return
 */
static bool create_dpma_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* uuid;
  char* timestamp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_dp_dpma_info.");

  j_tmp = json_deep_copy(j_data);
  json_object_del(j_tmp, "tm_create");
  json_object_del(j_tmp, "tm_update");
  json_object_del(j_tmp, "tm_uuid");

  timestamp = get_utc_timestamp();
  uuid = gen_uuid();

  json_object_set_new(j_tmp, "uuid", json_string(uuid));
  sfree(uuid);
  json_object_set_new(j_tmp, "tm_create", json_string(timestamp));
  sfree(timestamp);

  // insert info
  ret = create_dp_dpma_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dpma contact.");
    return false;
  }

  return true;
}
