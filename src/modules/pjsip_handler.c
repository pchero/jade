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
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
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
  const char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_endpoints_detail.");

  name = req->uri->path->file;
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_endpoint_info(name);
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
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
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
  const char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_aors_detail.");

  name = req->uri->path->file;
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_aor_info(name);
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
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
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
  const char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_auths_detail.");

  name = req->uri->path->file;
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_auth_info(name);
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
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
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
  const char* tmp_const;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_contacts_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_decode(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_pjsip_contact_info(name);
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

  req_data = get_text_from_request_data(req);
  if(req_data == NULL) {
    slog(LOG_ERR, "Could not get data.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update config
  ret = update_ast_current_config_info_text(DEF_PJSIP_CONFNAME, req_data);
  sfree(req_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update pjsip config info.");
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
* GET ^/pjsip/configs/(.*) request handler.
* @param req
* @param data
*/
void htp_get_pjsip_configs_detail(evhtp_request_t *req, void *data)
{
 json_t* j_res;
 char* res;
 const char* tmp_const;
 char* detail;

 if(req == NULL) {
   slog(LOG_WARNING, "Wrong input parameter.");
   return;
 }
 slog(LOG_DEBUG, "Fired htp_get_pjsip_configs_detail.");

 // detail parse
 tmp_const = req->uri->path->file;
 detail = uri_decode(tmp_const);
 if(detail == NULL) {
   slog(LOG_ERR, "Could not get detail info.");
   simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
   return;
 }

 // get info
 res = get_ast_backup_config_info_text_valid(detail, DEF_PJSIP_CONFNAME);
 sfree(detail);
 if(res == NULL) {
   slog(LOG_NOTICE, "Could not find config info.");
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
 * DELETE ^/pjsip/configs/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_pjsip_configs_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_pjsip_configs_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // remove it
  ret = remove_ast_backup_config_info_valid(detail, DEF_PJSIP_CONFNAME);
  sfree(detail);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete backup config file.");
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
 * GET ^/pjsip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_pjsip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  const char* tmp_const;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_pjsip_settings_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get setting
  j_tmp =get_ast_setting(DEF_PJSIP_CONFNAME, detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get pjsip setting.");
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
 * PUT ^/pjsip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_put_pjsip_settings_detail(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired htp_put_pjsip_settings_detail.");

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
  ret = update_ast_setting(DEF_PJSIP_CONFNAME, detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update pjsip setting.");
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
 * DELETE ^/pjsip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_pjsip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* tmp_const;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_pjsip_settings_detail.");

  // detail parse
  tmp_const = req->uri->path->file;
  detail = uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete setting
  ret = remove_ast_setting(DEF_PJSIP_CONFNAME, detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not remove pjsip setting.");
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
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get data from request.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get name
  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_ERR, "Could not get setting name.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get setting info
  j_setting = json_object_get(j_data, "setting");
  if(j_setting == NULL) {
    slog(LOG_ERR, "Could not get setting.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create setting
  ret = create_ast_setting(DEF_PJSIP_CONFNAME, name, j_setting);
  if(ret == false) {
    slog(LOG_ERR, "Could not create pjsip setting.");
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
