
#define _GNU_SOURCE

#include <stdio.h>
#include <jansson.h>

#include "slog.h"
#include "resource_handler.h"
#include "http_handler.h"
#include "ami_action_handler.h"
#include "utils.h"

#include "core_handler.h"



/**
 * GET ^/core/channels request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_channels(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_core_channels_detail.");

  j_tmp = get_core_channels_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}


/**
 * GET ^/core/channels/ request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_channels_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* unique_id;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_core_channels_detail.");

  // get channel unique_id
  unique_id = utils_uri_decode(req->uri->path->file);
  if(unique_id == NULL) {
    slog(LOG_NOTICE, "Could not get uuid info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get channel info.
  j_tmp = get_core_channel_info(unique_id);
  sfree(unique_id);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get channel info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * DELETE ^/core/channels/(.*) request handler.
 * @param req
 * @param data
 */
void core_htp_delete_core_channels_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_delete_core_channels_detail.");

  // get detail
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_NOTICE, "Could not get uuid info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // send hangup
  ret = ami_action_hangup_by_uniqueid(detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not send hangup.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/core/agis request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_agis(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_core_agis.");

  j_tmp = get_core_agis_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/core/agis/<detail> request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_agis_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* unique_id;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_core_agis_detail.");

  // get agi unique_id
  unique_id = utils_uri_decode(req->uri->path->file);
  if(unique_id == NULL) {
    slog(LOG_NOTICE, "Could not get uuid info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get agi info.
  j_tmp = get_core_agi_info(unique_id);
  sfree(unique_id);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get agi info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/core/systems/ request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_systems_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_core_systems_detail.");

  // get detail uuid
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_NOTICE, "Could not get id info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get channel info.
  j_tmp = get_core_system_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get system info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/core/systems request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_systems(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_core_systems.");

  j_tmp = get_core_systems_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/core/modules$ request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_modules(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_core_modules.");

  j_tmp = get_core_modules_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void core_htp_get_core_modules_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_gett_core_modules_detail.");

  // get module name
  name = utils_uri_decode(req->uri->path->file);
  if(name == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get module info
  j_tmp = get_core_module_info(name);
  sfree(name);

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;

  return;
}

/**
 * POST ^/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void core_htp_post_core_modules_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* name;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_post_core_modules_detail.");

  // get module name
  name = utils_uri_decode(req->uri->path->file);
  if(name == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // send request
  ret = ami_action_moduleload(name, "load");
  sfree(name);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * PUT ^/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void core_htp_put_core_modules_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* name;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_put_core_modules_detail.");

  // get module name
  name = utils_uri_decode(req->uri->path->file);
  if(name == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // send request
  ret = ami_action_moduleload(name, "reload");
  sfree(name);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * DELETE ^/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void core_htp_delete_core_modules_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* name;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_delete_core_modules_detail.");

  // get module name
  name = utils_uri_decode(req->uri->path->file);
  if(name == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // send request
  ret = ami_action_moduleload(name, "unload");
  sfree(name);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}
