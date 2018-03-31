/*
 * ami_response_handler.c
 *
 *  Created on: Mar 31, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <jansson.h>

#include "slog.h"
#include "utils.h"
#include "common.h"
#include "action_handler.h"
#include "resource_handler.h"


ACTION_RES ami_response_handler_corestatus(json_t* j_action, json_t* j_msg)
{
  char* tmp;
  char* sql;
  char* timestamp;
  const char* response;
  int ret;
  json_t* j_tmp;

  if((j_action == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return ACTION_RES_ERROR;
  }
  slog(LOG_DEBUG, "Fired ob_ami_response_handler_status.");

  // for debug
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Received message. tmp[%s]", tmp);
  sfree(tmp);

  // get event
  // if could not get event, consider it's the event
  // get response
  response = json_string_value(json_object_get(j_msg, "Response"));
  if(response == NULL) {
    slog(LOG_ERR, "Could not get response.");
    return ACTION_RES_ERROR;
  }

  ret = strcasecmp(response, "Success");
  if(ret != 0) {
    slog(LOG_ERR, "Could not get correct response.");
    return ACTION_RES_ERROR;
  }

  timestamp = utils_get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, s:i, "
      "s:s"
      "}",

      "startup_date",   json_string_value(json_object_get(j_msg, "CoreStartupDate")),
      "startup_time",   json_string_value(json_object_get(j_msg, "CoreStartupTime")),
      "reload_date",    json_string_value(json_object_get(j_msg, "CoreReloadDate")),
      "reload_time",    json_string_value(json_object_get(j_msg, "CoreReloadTime")),
      "current_calls",  json_string_value(json_object_get(j_msg, "CoreCurrentCalls"))? atoi(json_string_value(json_object_get(j_msg, "CoreCurrentCalls"))) : 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);


  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update system set %s where id=\"%s\";",
      tmp,
      json_string_value(json_object_get(json_object_get(j_action, "data"), "id"))
      );
  sfree(tmp);

  ret = db_ctx_exec(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update system info. id[%s]", json_string_value(json_object_get(json_object_get(j_action, "data"), "id")));
    return ACTION_RES_ERROR;
  }

  return ACTION_RES_COMPLETE;
}

ACTION_RES ami_response_handler_coresettings(json_t* j_action, json_t* j_msg)
{
  char* tmp;
  char* sql;
  char* timestamp;
  const char* response;
  int ret;
  json_t* j_tmp;

  if((j_action == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return ACTION_RES_ERROR;
  }
  slog(LOG_DEBUG, "Fired ob_ami_response_handler_status.");

  // for debug
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Received message. tmp[%s]", tmp);
  sfree(tmp);

  // get event
  // if could not get event, consider it's the event
  // get response
  response = json_string_value(json_object_get(j_msg, "Response"));
  if(response == NULL) {
    slog(LOG_ERR, "Could not get response.");
    return ACTION_RES_ERROR;
  }

  ret = strcasecmp(response, "Success");
  if(ret != 0) {
    slog(LOG_ERR, "Could not get correct response.");
    return ACTION_RES_ERROR;
  }

  timestamp = utils_get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:i, s:f, s:i"
      "s:s, s:s,"
      "s:s, s:s, s:s"
      "}",

      "ami_version",   json_string_value(json_object_get(j_msg, "AMIversion")),
      "ast_version",   json_string_value(json_object_get(j_msg, "AsteriskVersion")),
      "system_name",    json_string_value(json_object_get(j_msg, "SystemName")),

      "max_calls",        json_string_value(json_object_get(j_msg, "CoreMaxCalls"))? atoi(json_string_value(json_object_get(j_msg, "CoreMaxCalls"))) : 0,
      "max_load_avg",     json_string_value(json_object_get(j_msg, "CoreMaxLoadAvg"))? atof(json_string_value(json_object_get(j_msg, "CoreMaxCalls"))) : 0.0,
      "max_file_handles", json_string_value(json_object_get(j_msg, "CoreMaxFilehandles"))? atoi(json_string_value(json_object_get(j_msg, "CoreMaxFilehandles"))) : 0,

      "run_user",         json_string_value(json_object_get(j_msg, "CoreRunUser")),
      "run_group",        json_string_value(json_object_get(j_msg, "CoreRunGroup")),

      "real_time_enabled",  json_string_value(json_object_get(j_msg, "CoreRealTimeEnabled")),
      "cdr_enabled",        json_string_value(json_object_get(j_msg, "CoreCDRenabled")),
      "http_enabled",       json_string_value(json_object_get(j_msg, "CoreHTTPenabled")),


      "tm_update",  timestamp
      );
  sfree(timestamp);


  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update system set %s where id=\"%s\";",
      tmp,
      json_string_value(json_object_get(json_object_get(j_action, "data"), "id"))
      );
  sfree(tmp);

  ret = db_ctx_exec(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update system info. id[%s]", json_string_value(json_object_get(json_object_get(j_action, "data"), "id")));
    return ACTION_RES_ERROR;
  }

  return ACTION_RES_COMPLETE;
}

/**
 * Action request response handle
 * name: modulecheck
 * @param j_action
 * @param j_msg
 * @return
 */
ACTION_RES ami_response_handler_modulecheck(json_t* j_action, json_t* j_msg)
{
  char* tmp;
  char* timestamp;
  const char* response;
  const char* load;
  int ret;
  json_t* j_tmp;
  json_t* j_action_data;

  if((j_action == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return ACTION_RES_ERROR;
  }
  slog(LOG_DEBUG, "Fired ami_response_handler_modulecheck.");

  j_action_data = json_object_get(j_action, "data");
  if(j_action_data == NULL) {
    slog(LOG_ERR, "Could not get action data.");
    return ACTION_RES_ERROR;
  }

  // for debug
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Received message. tmp[%s]", tmp);
  sfree(tmp);

  // get event
  // if could not get event, consider it's the event
  // get response
  response = json_string_value(json_object_get(j_msg, "Response"));
  if(response == NULL) {
    slog(LOG_ERR, "Could not get response.");
    return ACTION_RES_ERROR;
  }

  // set load
  load = "not loaded";
  ret = strcasecmp(response, "Success");
  if(ret == 0) {
    load = "loaded";
  }

  // create update
  timestamp = utils_get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s "
      "}",

      "name",       json_string_value(json_object_get(j_action_data, "Module"))? : "",
      "load",       load,
      "version",    json_string_value(json_object_get(j_msg, "Version"))? : "unknown",
      "tm_update",  timestamp
      );
  sfree(timestamp);

  // update module info
  ret = update_core_module_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core module info.");
  }

  return ACTION_RES_COMPLETE;
}

/**
 * Action request response handle
 * name: moduleload
 * @param j_action
 * @param j_msg
 * @return
 */
ACTION_RES ami_response_handler_moduleload(json_t* j_action, json_t* j_msg)
{
  char* tmp;
  char* timestamp;
  const char* response;
  const char* load;
  const char* type;
  int ret;
  json_t* j_tmp;
  json_t* j_action_data;

  if((j_action == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return ACTION_RES_ERROR;
  }
  slog(LOG_DEBUG, "Fired ami_response_handler_moduleload.");

  j_action_data = json_object_get(j_action, "data");
  if(j_action_data == NULL) {
    slog(LOG_ERR, "Could not get action data.");
    return ACTION_RES_ERROR;
  }

  // for debug
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Received message. tmp[%s]", tmp);
  sfree(tmp);

  // get event
  // if could not get event, consider it's the event
  // get response
  response = json_string_value(json_object_get(j_msg, "Response"));
  if(response == NULL) {
    slog(LOG_ERR, "Could not get response.");
    return ACTION_RES_ERROR;
  }

  // check response
  ret = strcasecmp(response, "Success");
  if(ret != 0) {
    return ACTION_RES_COMPLETE;
  }

  // get load type
  type = json_string_value(json_object_get(j_action_data, "LoadType"));
  if(type == NULL) {
    slog(LOG_ERR, "Could not get loadtype info.");
    return ACTION_RES_ERROR;
  }

  // check loadtype
  if(strcasecmp(type, "load") == 0) {
    load = "loaded";
  }
  else if(strcasecmp(type, "unload") == 0) {
    load = "unload";
  }
  else {
    slog(LOG_NOTICE, "Not supported load type. type[%s]", type);
    return ACTION_RES_COMPLETE;
  }

  // create update
  timestamp = utils_get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s "
      "}",

      "name",       json_string_value(json_object_get(j_action_data, "Module"))? : "",
      "load",       load,
      "tm_update",  timestamp
      );
  sfree(timestamp);

  // update module info
  ret = update_core_module_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core module info.");
  }

  return ACTION_RES_COMPLETE;
}
