/*
 * pjsip_handler.c
 *
 *  Created on: Dec 13, 2017
 *      Author: pchero
 */


#include <jansson.h>

#include "slog.h"
#include "utils.h"
#include "resource_handler.h"
#include "http_handler.h"
#include "ami_handler.h"
#include "conf_handler.h"
#include "publish_handler.h"

#include "pjsip_handler.h"

#define DEF_PJSIP_CONFNAME  "pjsip.conf"

bool init_pjsip_handler(void)
{
  int ret;
  json_t* j_tmp;

  slog(LOG_DEBUG, "Fired init_pjsip_handler.");

  // pjsip
  j_tmp = json_pack("{s:s}",
      "Action", "PJSIPShowEndpoints"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "PJSIPShowEndpoints");
    return false;
  }

  return true;
}

bool term_pjsip_handler(void)
{
  int ret;

  ret = clear_pjsip();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear pjsip info.");
    return false;
  }

  return true;
}

bool reload_pjsip_handler(void)
{
  int ret;

  ret = term_pjsip_handler();
  if(ret == false) {
    return false;
  }

  ret = init_pjsip_handler();
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * htp request handler.
 * request: GET ^/pjsip/endpoints
 * @param req
 * @param data
 */
void htp_get_pjsip_endpoints(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_endpoints.");

  j_tmp = get_pjsip_endpoints_all();

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/pjsip/endpoints/(*) request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_endpoints_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_endpoints_detail.");

  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_endpoint_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET ^/pjsip/aors$
 * @param req
 * @param data
 */
void htp_get_pjsip_aors(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_aors.");

  j_tmp = get_pjsip_aors_all();

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/pjsip/aors//(*) request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_aors_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_aors_detail.");

  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_aor_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET ^/pjsip/auths$
 * @param req
 * @param data
 */
void htp_get_pjsip_auths(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_auths.");

  j_tmp = get_pjsip_auths_all();

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/pjsip/auths/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_auths_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_auths_detail.");

  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_auth_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET ^/pjsip/contacts$
 * @param req
 * @param data
 */
void htp_get_pjsip_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_contacts.");

  j_tmp = get_pjsip_contacts_all();

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/pjsip/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_contacts_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_contact_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/pjsip/config request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_config(evhtp_request_t *req, void *data)
{
  char* res;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_config.");

  // get info
  res = get_ast_current_config_info_text(DEF_PJSIP_CONFNAME);
  if(res == NULL) {
    slog(LOG_ERR, "Could not get pjsip conf.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_string(res));
  sfree(res);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * PUT ^/pjsip/config request handler.
 * @param req
 * @param data
 */
void htp_put_pjsip_config(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* req_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_pjsip_config.");

  req_data = http_get_text_from_request_data(req);
  if(req_data == NULL) {
    slog(LOG_ERR, "Could not get data.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update config
  ret = update_ast_current_config_info_text(DEF_PJSIP_CONFNAME, req_data);
  sfree(req_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update pjsip config info.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/pjsip/configs request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_configs(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_configs.");

  // get info
  j_tmp = get_ast_backup_configs_info_all(DEF_PJSIP_CONFNAME);
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
* GET ^/pjsip/configs/(.*) request handler.
* @param req
* @param data
*/
void htp_get_pjsip_configs_detail(evhtp_request_t *req, void *data)
{
 json_t* j_res;
 char* res;
 char* detail;

 if(req == NULL) {
   slog(LOG_WARNING, "Wrong input parameter.");
   return;
 }
 slog(LOG_DEBUG, "Fired htp_get_pjsip_configs_detail.");

 // detail parse
 detail = http_get_parsed_detail(req);
 if(detail == NULL) {
   slog(LOG_ERR, "Could not get detail info.");
   http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
   return;
 }

 // get info
 res = get_ast_backup_config_info_text_valid(detail, DEF_PJSIP_CONFNAME);
 sfree(detail);
 if(res == NULL) {
   slog(LOG_NOTICE, "Could not find config info.");
   http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
   return;
 }

 // create result
 j_res = http_create_default_result(EVHTP_RES_OK);
 json_object_set_new(j_res, "result", json_string(res));
 sfree(res);

 // response
 http_simple_response_normal(req, j_res);
 json_decref(j_res);

 return;
}

/**
 * DELETE ^/pjsip/configs/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_pjsip_configs_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_pjsip_configs_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // remove it
  ret = remove_ast_backup_config_info_valid(detail, DEF_PJSIP_CONFNAME);
  sfree(detail);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete backup config file.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * GET ^/pjsip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_settings_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get setting
  j_tmp =get_ast_setting(DEF_PJSIP_CONFNAME, detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * PUT ^/pjsip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_put_pjsip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_pjsip_settings_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    sfree(detail);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update setting
  ret = update_ast_setting(DEF_PJSIP_CONFNAME, detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update pjsip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * DELETE ^/pjsip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_pjsip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_pjsip_settings_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete setting
  ret = remove_ast_setting(DEF_PJSIP_CONFNAME, detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not remove pjsip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET ^/pjsip/settings$
 * @param req
 * @param data
 */
void htp_get_pjsip_settings(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_settings.");

  // get info
  j_tmp = get_ast_settings_all(DEF_PJSIP_CONFNAME);

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/pjsip/settings$
 * @param req
 * @param data
 */
void htp_post_pjsip_settings(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  const char* name;
  json_t* j_setting;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_pjsip_settings.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get data from request.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get name
  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_ERR, "Could not get setting name.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get setting info
  j_setting = json_object_get(j_data, "setting");
  if(j_setting == NULL) {
    slog(LOG_ERR, "Could not get setting.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create setting
  ret = create_ast_setting(DEF_PJSIP_CONFNAME, name, j_setting);
  if(ret == false) {
    slog(LOG_ERR, "Could not create pjsip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * Get corresponding pjsip_endpoint info.
 * @param name
 * @return
 */
json_t* get_pjsip_endpoint_info(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_pjsip_endpoint_info. object_name[%s]", name);

  j_res = get_ast_detail_item_key_string("pjsip_endpoint", "object_name", name);

  return j_res;
}

/**
 * Get all list of pjsip_endpoint info.
 * @param name
 * @return
 */
json_t* get_pjsip_endpoints_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_pjsip_endpoints_all.");

  j_res = get_ast_items("pjsip_endpoint", "*");

  return j_res;
}

/**
 * Get all list of pjsip_aor info.
 * @param name
 * @return
 */
json_t* get_pjsip_aors_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired pjsip_aor.");

  j_res = get_ast_items("pjsip_aor", "*");

  return j_res;
}

/**
 * Get detail info of given pjsip_aor key.
 * @param name
 * @return
 */
json_t* get_pjsip_aor_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_pjsip_aors_info.");

  j_res = get_ast_detail_item_key_string("pjsip_aor", "object_name", key);

  return j_res;
}

/**
 * Get all list of pjsip_auth info.
 * @param name
 * @return
 */
json_t* get_pjsip_auths_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_pjsip_auths_all.");

  j_res = get_ast_items("pjsip_auth", "*");

  return j_res;
}

/**
 * Get detail info of given pjsip_auth key.
 * @param name
 * @return
 */
json_t* get_pjsip_auth_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_pjsip_auth_info.");

  j_res = get_ast_detail_item_key_string("pjsip_auth", "object_name", key);

  return j_res;
}

/**
 * Get all list of pjsip_contact info.
 * @param name
 * @return
 */
json_t* get_pjsip_contacts_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_pjsip_contacts_all.");

  j_res = get_ast_items("pjsip_contact", "*");

  return j_res;
}

/**
 * Get detail info of given pjsip_contact key.
 * @param name
 * @return
 */
json_t* get_pjsip_contact_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_pjsip_contact_info.");

  j_res = get_ast_detail_item_key_string("pjsip_contact", "uri", key);

  return j_res;
}


/**
 * Create pjsip endpoint info.
 * @param j_data
 * @return
 */
bool create_pjsip_endpoint_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_pjsip_endpoint_info.");

  // insert queue info
  ret = insert_ast_item("pjsip_endpoint", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert pjsip endpoint.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  j_tmp = get_pjsip_endpoint_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_endpoint info. id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_endpoint(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update pjsip endpoint info.
 * @param j_data
 * @return
 */
bool update_pjsip_endpoint_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_pjsip_endpoint_info.");

  ret = update_ast_item("pjsip_endpoint", "object_name", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update pjsip endpoint info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  j_tmp = get_pjsip_endpoint_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_endpoint info. id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_endpoint(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete pjsip endpoint info.
 * @return
 */
bool delete_pjsip_endpoint_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_pjsip_endpoint_info. key[%s]", key);

  // get info
  j_tmp = get_pjsip_endpoint_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_endpoint info. id[%s]", key);
    return false;
  }

  // delete info
  ret = delete_ast_items_string("pjsip_endpoint", "object_name", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete pjsip_endpoint info. key[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publish_event_pjsip_endpoint(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Create pjsip auth info.
 * @param j_data
 * @return
 */
bool create_pjsip_auth_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_pjsip_auth_info.");

  // insert queue info
  ret = insert_ast_item("pjsip_auth", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert pjsip auth.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  j_tmp = get_pjsip_auth_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_auth info. object_name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_auth(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update pjsip auth info.
 * @param j_data
 * @return
 */
bool update_pjsip_auth_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* tmp_const;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_pjsip_auth_info.");

  // update
  ret = update_ast_item("pjsip_auth", "object_name", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update pjsip_auth info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  j_tmp = get_pjsip_auth_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_auth info. object_name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_auth(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete pjsip auth info.
 * @return
 */
bool delete_pjsip_auth_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_pjsip_auth_info. key[%s]", key);

  // get info
  j_tmp = get_pjsip_auth_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_auth info. object_name[%s]", key);
    return false;
  }

  ret = delete_ast_items_string("pjsip_auth", "object_name", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete pjsip_auth info. key[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publish_event_pjsip_auth(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Create pjsip aor info.
 * @param j_data
 * @return
 */
bool create_pjsip_aor_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_pjsip_aor_info.");

  // insert queue info
  ret = insert_ast_item("pjsip_aor", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert pjsip aor.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  j_tmp = get_pjsip_aor_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_aor info. object_name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_aor(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update pjsip aor info.
 * @param j_data
 * @return
 */
bool update_pjsip_aor_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_pjsip_aor_info.");

  // update
  ret = update_ast_item("pjsip_aor", "object_name", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update pjsip_aor info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  j_tmp = get_pjsip_aor_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_aor info. object_name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_aor(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }


  return true;
}

/**
 * delete pjsip aor info.
 * @return
 */
bool delete_pjsip_aor_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_pjsip_aor_info. key[%s]", key);

  // publish
  // get info
  j_tmp = get_pjsip_aor_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_aor info. object_name[%s]", key);
    return false;
  }

  ret = delete_ast_items_string("pjsip_aor", "object_name", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete pjsip_aor info. key[%s]", key);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_aor(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Create pjsip contact info.
 * @param j_data
 * @return
 */
bool create_pjsip_contact_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_pjsip_contact_info.");

  // insert info
  ret = insert_ast_item("pjsip_contact", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert pjsip contact.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uri"));
  j_tmp = get_pjsip_contact_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_contact info. uri[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_contact(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update pjsip contact info.
 * @param j_data
 * @return
 */
bool update_pjsip_contact_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_pjsip_contact_info.");

  // update
  ret = update_ast_item("pjsip_contact", "uri", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update pjsip_contact info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uri"));
  j_tmp = get_pjsip_contact_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_contact info. uri[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_pjsip_contact(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete pjsip contact info.
 * @return
 */
bool delete_pjsip_contact_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_pjsip_contact_info. key[%s]", key);

  // get info
  j_tmp = get_pjsip_contact_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip_contact info. uri[%s]", key);
    return false;
  }

  ret = delete_ast_items_string("pjsip_contact", "uri", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete pjsip_contact info. key[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publish_event_pjsip_contact(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }


  return true;
}

/**
 * Clear all pjsip resources.
 * @return
 */
bool clear_pjsip(void)
{
  int ret;

  ret = clear_ast_table("pjsip_endpoint");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear pjsip_endpoint");
    return false;
  }

  ret = clear_ast_table("pjsip_aor");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear pjsip_aor");
    return false;
  }

  ret = clear_ast_table("pjsip_auth");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear pjsip_auth");
    return false;
  }

  ret = clear_ast_table("pjsip_contact");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear pjsip_contact");
    return false;
  }

  return true;

}
