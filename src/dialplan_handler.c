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

#define DEF_DIALPLAN_CONFNAME   "extensions.conf"

/**
 * GET ^/dialplan/setting request handler.
 * @param req
 * @param data
 */
void htp_get_dialplan_setting(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* format;
  char* tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_dialplan_setting.");

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
 * PUT ^/dialplan/setting request handler.
 * @param req
 * @param data
 */
void htp_put_dialplan_setting(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* tmp;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_dialplan_setting.");

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
