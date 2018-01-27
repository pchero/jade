/*
 * park_handler.c
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <string.h>

#include "slog.h"
#include "http_handler.h"
#include "utils.h"
#include "conf_handler.h"
#include "ami_handler.h"

#include "resource_handler.h"

#include "park_handler.h"

#define DEF_PARK_CONFNAME   "res_parking.conf"

#define DEF_SETTING_CONTEXT_1   "general"
#define DEF_SETTING_CONTEXT_2   "default"


static bool delete_parkinglot_info(const char* parkinglot);
static bool create_parkinglot_info(json_t* j_data);
static bool update_parkinglot_info(const char* name, json_t* j_data);

static char* get_park_backup_config_info_text(const char* filename);
static bool remove_park_backup_config_info(const char* filename);

static json_t* create_parkinglot_info_json(json_t* j_data);

static bool is_setting_section(const char* context);
static bool is_park_config_filename(const char* filename);

static json_t* get_park_settings_all(void);
static bool create_park_setting(const json_t* j_data);
static json_t* get_park_setting(const char* name);
static bool update_park_setting(const char* name, const json_t* j_data);
static bool remove_park_setting(const char* name);


/**
 * htp request handler.
 * request: GET ^/park/parkinglots$
 * @param req
 * @param data
 */
void htp_get_park_parkinglots(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_parkinglots.");

  j_tmp = get_park_parkinglots_all();

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/park/parkinglots$
 * @param req
 * @param data
 */
void htp_post_park_parkinglots(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_park_parkinglots.");

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    // no request data
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create parking lot info
  ret = create_parkinglot_info(j_data);
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
 * GET ^/park/parkinglots/<detail> request handler.
 * @param req
 * @param data
 */
void htp_get_park_parkinglots_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_parking_lots_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_park_parkinglot_info(name);
  sfree(name);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
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
 * PUT ^/park/parkinglots/<detail> request handler.
 * @param req
 * @param data
 */
void htp_put_park_parkinglots_detail(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_park_parkinglots_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
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

  // update parkinglot
  ret = update_parkinglot_info(detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update parkinglot info.");
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
 * DELETE ^/park/parkinglots/<detail> request handler.
 * @param req
 * @param data
 */
void htp_delete_park_parkinglots_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_park_parkinglots_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete parking lot
  ret = delete_parkinglot_info(detail);
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
 * htp request handler.
 * request: GET ^/park/parkedcalls$
 * @param req
 * @param data
 */
void htp_get_park_parkedcalls(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_parkedcalls.");

  j_tmp = get_park_parkedcalls_all();

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/park/parkedcalls/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_park_parkedcalls_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input htp_get_park_parkedcalls_detail.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_parking_lots_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_park_parkedcall_info(name);
  sfree(name);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
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
 * GET ^/park/config request handler.
 * @param req
 * @param data
 */
void htp_get_park_config(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_config.");

  // get info
  res = get_ast_current_config_info_text(DEF_PARK_CONFNAME);
  if(res == NULL) {
    slog(LOG_ERR, "Could not get park conf.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_string(res));
  sfree(res);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * PUT ^/park/config request handler.
 * @param req
 * @param data
 */
void htp_put_park_config(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* req_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_park_config.");

  // get data
  req_data = get_text_from_request_data(req);
  if(data == NULL) {
    slog(LOG_ERR, "Could not get data.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update config
  ret = update_ast_current_config_info_text(DEF_PARK_CONFNAME, req_data);
  sfree(req_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update park config info.");
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
 * GET ^/park/configs request handler.
 * @param req
 * @param data
 */
void htp_get_park_configs(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_configs.");

  // get info
  j_tmp = get_ast_backup_configs_info_all(DEF_PARK_CONFNAME);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/park/configs/<detail> request handler.
 * @param req
 * @param data
 */
void htp_get_park_configs_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_configs_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  res = get_park_backup_config_info_text(detail);
  sfree(detail);
  if(res == NULL) {
  	slog(LOG_NOTICE, "Could not get backup config info.");
		simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
		return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_string(res));
  sfree(res);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * DELETE ^/park/configs/<detail> request handler.
 * @param req
 * @param data
 */
void htp_delete_park_configs_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_park_configs_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // remove it
  ret = remove_park_backup_config_info(detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete setting file.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
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
 * htp request handler.
 * request: GET ^/park/settings$
 * @param req
 * @param data
 */
void htp_get_park_settings(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_settings.");

  // get info
  j_tmp = get_park_settings_all();

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/park/settings$
 * @param req
 * @param data
 */
void htp_post_park_settings(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_park_settings.");

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
  	slog(LOG_ERR, "Could not get data from request.");
  	simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
  	return;
  }

  // create setting
  ret = create_park_setting(j_data);
  json_decref(j_data);
  if(ret == false) {
  	slog(LOG_ERR, "Could not create park setting.");
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
 * htp request handler.
 * GET ^/park/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_park_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  const char* tmp_const;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_park_settings_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get setting
  j_tmp = get_park_setting(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get park setting.");
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
 * PUT ^/park/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_put_park_settings_detail(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_park_settings_detail.");

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

  // update setting
  ret = update_park_setting(detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update park setting.");
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
 * htp request handler.
 * DELETE ^/park/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_park_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_park_settings_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete setting
  ret = remove_park_setting(detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not remove park setting.");
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

static bool is_setting_section(const char* context)
{
  int ret;

  if(context == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = strcmp(context, DEF_SETTING_CONTEXT_1);
  if(ret == 0) {
    return true;
  }

  ret = strcmp(context, DEF_SETTING_CONTEXT_2);
  if(ret == 0) {
    return true;
  }

  return false;
}

/**
 * Delete parkinglot.
 * @param parkinglot
 * @return
 */
static bool delete_parkinglot_info(const char* parkinglot)
{
  int ret;

  if(parkinglot == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_parkinglot. parkinglot[%s]", parkinglot);

  ret = is_setting_section(parkinglot);
  if(ret == true) {
    slog(LOG_ERR, "Given context is setting context.");
    return false;
  }

  // delete parkinglot
  ret = remove_ast_setting(DEF_PARK_CONFNAME, parkinglot);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete parkinglot.");
    return false;
  }

  return true;
}

/**
 * Create parking lot info.
 * @return
 */
static bool create_parkinglot_info(json_t* j_data)
{
  const char* name;
  int ret;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_parkinglot");

  // get mandatory item
  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    return false;
  }

  // is setting context?
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_NOTICE, "Given context is setting context.");
    return false;
  }

  // create parking lot info json
  j_tmp = create_parkinglot_info_json(j_data);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create parkinglot info json.");
    return false;
  }

  // create new parkinglot
  ret = create_ast_setting(DEF_PARK_CONFNAME, name, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create new parkinglot.");
    return false;
  }

  return true;
}

/**
 * Create json info for parkinglot creation.
 * @param j_data
 * @return
 */
static json_t* create_parkinglot_info_json(json_t* j_data)
{
  json_t* j_res;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // currently, just copy the data.
  // may need to validate all the options later.
  j_res = json_deep_copy(j_data);

  // remove the name
  json_object_del(j_res, "name");

  return j_res;
}

/**
 * Update parkinglot info.
 * @param name
 * @param j_data
 * @return
 */
static bool update_parkinglot_info(const char* name, json_t* j_data)
{
  json_t* j_tmp;
  int ret;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_parkinglot_info.");

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given parkinglot name is setting section. name[%s]", name);
    return false;
  }

  // create json
  j_tmp = create_parkinglot_info_json(j_data);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create park info json.");
    return false;
  }

  // update parkinglot info
  ret = update_ast_setting(DEF_PARK_CONFNAME, name, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update park info.");
    return false;
  }

  return true;
}

/**
 * Get park backup configuration info.
 * @param filename
 * @return
 */
static char* get_park_backup_config_info_text(const char* filename)
{
  char* res;
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_park_backup_config_info_text. filename[%s]", filename);

  // check is park config filename
  ret = is_park_config_filename(filename);
  if(ret == false) {
    slog(LOG_ERR, "Given filename is not park config filename.");
    return NULL;
  }

  // get config info
  res = get_ast_backup_config_info_text(filename);

  return res;
}

/**
 * Return true if given filename related with park setting file.
 * @param filename
 * @return
 */
static bool is_park_config_filename(const char* filename)
{
  const char* tmp_const;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  tmp_const = strstr(filename, DEF_PARK_CONFNAME);
  if(tmp_const == NULL) {
    return false;
  }

  return true;
}

/**
 * Remove park setting(configuration) info.
 * @param filename
 * @return
 */
static bool remove_park_backup_config_info(const char* filename)
{
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired remove_park_backup_config_info. filename[%s]", filename);

  // check filename
  ret = is_park_config_filename(filename);
  if(ret == false) {
    slog(LOG_NOTICE, "The given filename is not park conf file.");
    return false;
  }

  // remove
  ret = remove_ast_backup_config_info(filename);
  if(ret == false) {
    slog(LOG_ERR, "Could not remove park backup conf.");
    return false;
  }

  return true;
}

static json_t* get_park_settings_all(void)
{
	json_t* j_res;

	j_res = get_ast_settings_all(DEF_PARK_CONFNAME);

	return j_res;
}

static bool create_park_setting(const json_t* j_data)
{
	const char* name;
	int ret;
	const json_t* j_setting;

	if(j_data == NULL) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return false;
	}

	name = json_string_value(json_object_get(j_data, "name"));
	if(name == NULL) {
		slog(LOG_ERR, "Could not get setting name.");
		return false;
	}

	j_setting = json_object_get(j_data, "setting");
	if(j_setting == NULL) {
		slog(LOG_ERR, "Could not get setting.");
		return false;
	}

	ret = create_ast_setting(DEF_PARK_CONFNAME, name, j_setting);
	if(ret == false) {
		slog(LOG_ERR, "Could not create park setting.");
		return false;
	}

	return true;
}

/**
 * Get park setting
 * @param name
 * @param j_data
 * @return
 */
static json_t* get_park_setting(const char* name)
{
	json_t* j_res;

	if(name == NULL) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return false;
	}
	slog(LOG_DEBUG, "Fired get_park_setting. name[%s]", name);

	j_res = get_ast_setting(DEF_PARK_CONFNAME, name);
	if(j_res == NULL) {
		slog(LOG_ERR, "Could not get park setting info. name[%s]", name);
		return NULL;
	}

	return j_res;
}

/**
 * Update park setting
 * @param name
 * @param j_data
 * @return
 */
static bool update_park_setting(const char* name, const json_t* j_data)
{
	int ret;

	if((name == NULL) || (j_data == NULL)) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return false;
	}
	slog(LOG_DEBUG, "Fired update_park_setting. name[%s]", name);

	ret = update_ast_setting(DEF_PARK_CONFNAME, name, j_data);
	if(ret == false) {
		slog(LOG_ERR, "Could not update park setting.");
		return false;
	}

	return true;
}

/**
 * Remove given park setting
 * @param name
 * @param j_data
 * @return
 */
static bool remove_park_setting(const char* name)
{
	int ret;

	if(name == NULL) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return false;
	}
	slog(LOG_DEBUG, "Fired remove_park_setting. name[%s]", name);

	ret = remove_ast_setting(DEF_PARK_CONFNAME, name);
	if(ret == false) {
		slog(LOG_ERR, "Could not remove park setting info. name[%s]", name);
		return false;
	}

	return true;
}

bool init_park_handler(void)
{
  int ret;
  json_t* j_tmp;

  slog(LOG_DEBUG, "Fired init_park_handler.");

  // parking_lot
  j_tmp = json_pack("{s:s}",
      "Action", "ParkingLots"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "Parkinglosts");
    return false;
  }

  // parked_call
  j_tmp = json_pack("{s:s}",
      "Action", "ParkedCalls"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "ParkedCalls");
    return false;
  }

  return true;
}

bool term_park_handler(void)
{
  int ret;

  ret = clear_park_parkinglot();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear parkinglot info.");
    return false;
  }

  ret = clear_park_parkedcall();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear parkedcall info.");
    return false;
  }

  return true;
}

bool reload_park_handler(void)
{
  int ret;

  ret = term_park_handler();
  if(ret == false) {
    return false;
  }

  ret = init_park_handler();
  if(ret == false) {
    return false;
  }

  return true;
}
