/*
 * queue_handler.c
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */


#include "slog.h"
#include "http_handler.h"
#include "utils.h"
#include "config.h"

#include "queue_handler.h"
#include "resource_handler.h"

#define DEF_QUEUE_CONFNAME  "queues.conf"
#define DEF_SETTING_SECTION "global"

static bool create_queue_info(json_t* j_data);
static bool update_queue_info(const char* name, json_t* j_data);
static bool delete_queue_info(const char* name);

static bool is_setting_section(const char* section);
static bool is_queue_setting_filename(const char* filename);

static json_t* create_queue_info_json(json_t* j_data);

static bool update_queue_current_setting_info(json_t* j_data);
static json_t* get_queue_backup_settings_all(void);
static json_t* get_queue_backup_setting_info(const char* filename);
static bool remove_queue_backup_setting_info(const char* filename);

/**
 * htp request handler.
 * request: GET ^/queue/entries$
 * @param req
 * @param data
 */
void htp_get_queue_entries(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_entries.");

  // get info
  j_tmp = get_queue_entries_all();

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
 * GET ^/queue/entries/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_queue_entries_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_entries_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_entry_info(name);
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
 * request: GET ^/queue/members$
 * @param req
 * @param data
 */
void htp_get_queue_members(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_members.");

  // get info
  j_tmp = get_queue_members_all();

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
 * GET ^/queue/members/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_queue_members_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* key;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_members_detail.");

  // key parse
  tmp_const = req->uri->path->file;
  key = uri_decode(tmp_const);
  if(key == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_member_info(key);
  sfree(key);
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
 * request: GET ^/queue/queues$
 * @param req
 * @param data
 */
void htp_get_queue_queues(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_queues.");

  // get info
  j_tmp = get_queue_params_all();

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
 * request: POST ^/queue/queues$
 * @param req
 * @param data
 */
void htp_post_queue_queues(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_queue_queues.");

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create queue
  ret = create_queue_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not get create queue info.");
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
 * GET ^/queue/queues/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_queue_queues_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_queues_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_param_info(name);
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
 * PUT ^/queue/queues/(.*) request handler.
 * @param req
 * @param data
 */
void htp_put_queue_queues_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  const char* tmp_const;
  char* name;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_queue_queues_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    sfree(name);
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update queue
  ret = update_queue_info(name, j_data);
  sfree(name);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue info.");
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
 * DELETE ^/queue/queues/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_queue_queues_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* name;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_queue_queues_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete queue
  ret = delete_queue_info(name);
  sfree(name);
  if(ret == false) {
    slog(LOG_ERR, "Could not get delete queue info.");
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
 * request: GET ^/queue/statuses$
 * @param req
 * @param data
 */
void htp_get_queue_statuses(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_statuses.");

  // get info
  j_tmp = get_queue_statuses_all();

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
 * request: GET ^/queue/statuses/(.*)
 * @param req
 * @param data
 */
void htp_get_queue_statuses_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  char* name;
  const char* tmp_const;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_statuses_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_status_info(name);
  sfree(name);

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/queue/setting request handler.
 * @param req
 * @param data
 */
void htp_get_queue_setting(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_setting.");

  // get info
  j_tmp = get_ast_current_config_info(DEF_QUEUE_CONFNAME);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue conf.");
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
 * PUT ^/queue/setting request handler.
 * @param req
 * @param data
 */
void htp_put_queue_setting(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_queue_setting.");

  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get data.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update setting
  ret = update_queue_current_setting_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue setting info.");
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
 * GET ^/queue/settings request handler.
 * @param req
 * @param data
 */
void htp_get_queue_settings(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_settings.");

  // get info
  j_tmp = get_queue_backup_settings_all();
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
 * GET ^/queue/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_queue_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_settings_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_backup_setting_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find setting info.");
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * DELETE ^/queue/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_queue_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_queue_settings_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // remove it
  ret = remove_queue_backup_setting_info(detail);
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
 * Create queue info.
 * @param j_data
 * @return
 */
static bool create_queue_info(json_t* j_data)
{
  const char* name;
  json_t* j_tmp;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get queue name
  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_ERR, "Could not get queue name.");
    return false;
  }

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given queue name is setting section. name[%s]", name);
    return false;
  }

  // create json
  j_tmp = create_queue_info_json(j_data);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create queue info json.");
    return false;
  }

  // create queue info
  ret = create_ast_current_config_section_data(DEF_QUEUE_CONFNAME, name, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create queue info.");
    return false;
  }

  return true;
}

/**
 * Update queue info.
 * @param j_data
 * @return
 */
static bool update_queue_info(const char* name, json_t* j_data)
{
  json_t* j_tmp;
  int ret;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_queue_info.");

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given queue name is setting section. name[%s]", name);
    return false;
  }

  // create json
  j_tmp = create_queue_info_json(j_data);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create queue info json.");
    return false;
  }

  // update queue info
  ret = update_ast_current_config_section_data(DEF_QUEUE_CONFNAME, name, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue info.");
    return false;
  }

  return true;
}

/**
 * Update queue info.
 * @param j_data
 * @return
 */
static bool delete_queue_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_queue_info.");

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given queue name is setting section. name[%s]", name);
    return false;
  }

  // delete queue info
  ret = delete_ast_current_config_section(DEF_QUEUE_CONFNAME, name);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue info.");
    return false;
  }

  return true;
}

/**
 * Create json info for queue creation.
 * @param j_data
 * @return
 */
static json_t* create_queue_info_json(json_t* j_data)
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
 * Return true if given section is global setting section
 * @param context
 * @return
 */
static bool is_setting_section(const char* section)
{
  int ret;

  if(section == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = strcmp(section, DEF_SETTING_SECTION);
  if(ret == 0) {
    return true;
  }

  return false;
}

/**
 * Update queue current setting(configuration) info.
 * @param filename
 * @return
 */
static bool update_queue_current_setting_info(json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_queue_current_setting_info");

  // update setting
  ret = update_ast_current_config_info(DEF_QUEUE_CONFNAME, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue setting.");
    return false;
  }

  return true;
}

/**
 * Get all queue setting(configuration)s info.
 * @param filename
 * @return
 */
static json_t* get_queue_backup_settings_all(void)
{
  json_t* j_res;

  j_res = get_ast_backup_configs_info_all(DEF_QUEUE_CONFNAME);

  return j_res;
}

/**
 * Get queue setting(configuration) info.
 * @param filename
 * @return
 */
static json_t* get_queue_backup_setting_info(const char* filename)
{
  json_t* j_res;
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_backup_setting_info. filename[%s]", filename);

  // check is queue config filename
  ret = is_queue_setting_filename(filename);
  if(ret == false) {
    slog(LOG_ERR, "Given filename is not queue config filename.");
    return NULL;
  }

  // get config info
  j_res = get_ast_backup_config_info(filename);

  return j_res;
}

/**
 * Remove queue setting(configuration) info.
 * @param filename
 * @return
 */
static bool remove_queue_backup_setting_info(const char* filename)
{
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired remove_queue_backup_setting_info. filename[%s]", filename);

  // check filename
  ret = is_queue_setting_filename(filename);
  if(ret == false) {
    slog(LOG_NOTICE, "The given filename is not queue conf file.");
    return false;
  }

  // remove
  ret = remove_ast_backup_config_info(filename);
  if(ret == false) {
    slog(LOG_ERR, "Could not remove queue backup conf.");
    return false;
  }

  return true;
}

/**
 * Return true if given filename related with queue setting file.
 * @param filename
 * @return
 */
static bool is_queue_setting_filename(const char* filename)
{
  const char* tmp_const;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  tmp_const = strstr(filename, DEF_QUEUE_CONFNAME);
  if(tmp_const == NULL) {
    return false;
  }

  return true;
}
