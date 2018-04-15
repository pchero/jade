/*
 * ami_action_handler.c
 *
 *  Created on: Dec 15, 2017
 *      Author: pchero
 */

#include <stdio.h>
#include <jansson.h>

#include "slog.h"
#include "common.h"
#include "utils.h"
#include "ami_handler.h"
#include "resource_handler.h"
#include "action_handler.h"
#include "call_handler.h"

#include "ami_action_handler.h"


/**
 * AMI action handler.
 * Action: AGI
 */
bool ami_action_agi(const char* channel, const char* cmd, const char* cmd_id)
{
  json_t* j_tmp;
  int ret;

  if((channel == NULL) || (cmd == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_agi. channel[%s], cmd[%s], cmd_id[%s]", channel, cmd, cmd_id? : "");


  // create request
  j_tmp = json_pack("{s:s, s:s, s:s, s:s}",
      "Action",     "AGI",
      "Channel",    channel,
      "Command",    cmd,
      "CommandID",  cmd_id? : ""
      );

  // send action request
  ret = ami_send_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action");
    return false;
  }

  return true;
}

/**
 * AMI action handler.
 * Action: hangup
 * @param j_msg
 */
bool ami_action_hangup(const char* channel)
{
  json_t* j_tmp;
  int ret;

  if(channel == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_hangup. channel[%s]", channel);

  // create hangup request
  j_tmp = json_pack("{s:s, s:s}",
      "Action",   "Hangup",
      "Channel",  channel
      );

  // send hangup request
  ret = ami_send_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action");
    return false;
  }

  return true;
}

/**
 * AMI action handler.
 * Action: hangup
 * @param j_msg
 */
bool ami_action_hangup_by_uniqueid(const char* unique_id)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_hangup_by_uniqueid. unique_id[%s]", unique_id);

  j_tmp = call_get_channel_info(unique_id);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get channel info.");
    return false;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "channel"));
  ret = ami_action_hangup(tmp_const);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * AMI action handler.
 * Action: ModuleCheck
 */
bool ami_action_modulecheck(const char* name)
{
  json_t* j_data;
  char* action_id;
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_modulecheck. name[%s]", name);

  action_id = utils_gen_uuid();

  // create request
  j_data = json_pack("{s:s, s:s, s:s}",
      "Action",     "ModuleCheck",
      "Module",     name,
      "ActionID",   action_id
      );
  sfree(action_id);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not create action request.");
    return false;
  }

  // send action request
  ret = ami_send_cmd(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action");
    json_decref(j_data);
    return false;
  }

  // insert action
  ret = action_insert(json_string_value(json_object_get(j_data, "ActionID")), "modulecheck", j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert action.");
    return false;
  }

  return true;
}

/**
 * AMI action handler.
 * Action: ModuleLoad
 */
bool ami_action_moduleload(const char* name, const char* type)
{
  json_t* j_data;
  char* action_id;
  int ret;

  if((name == NULL) || (type == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_moduleload. name[%s], type[%s]", name, type);

  action_id = utils_gen_uuid();

  // create request
  j_data = json_pack("{s:s, s:s, s:s, s:s}",
      "Action",     "ModuleLoad",
      "Module",     name,
      "LoadType",   type,
      "ActionID",   action_id
      );
  sfree(action_id);

  // send action request
  ret = ami_send_cmd(j_data);
  if(ret == false) {
    json_decref(j_data);
    slog(LOG_ERR, "Could not send ami action");
    return false;
  }

  // insert action
  ret = action_insert(json_string_value(json_object_get(j_data, "ActionID")), "moduleload", j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert action.");
    return false;
  }

  return true;
}

/**
 * AMI action handler.
 * Action: SetVar
 */
bool ami_action_setvar(const char* channel, const char* key, const char* value)
{
  json_t* j_data;
  int ret;

  if((channel == NULL) || (key == NULL) || (value == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_setvar. channel[%s], key[%s], value[%s]", channel, key, value);

  // create request
  j_data = json_pack("{s:s, s:s, s:s, s:s}",
      "Action",     "SetVar",
      "Channel",    channel,
      "Variable",   key,
      "Value",      value
      );

  // send action request
  ret = ami_send_cmd(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action");
    return false;
  }

  return true;
}

/**
 * AMI action handler.
 * Action: Originate
 */
bool ami_action_originate_application(const char* channel, const char* application, const char* data)
{
  json_t* j_cmd;
  int ret;

  if((channel == NULL) || (application == NULL) || (data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_originate_application. channel[%s], application[%s], data[%s]", channel, application, data);

  // create request
  //  Action: Originate
  //  ActionID: <value>
  //  Channel: <value>
  //  Exten: <value>
  //  Context: <value>
  //  Priority: <value>
  //  Application: <value>
  //  Data: <value>
  //  Timeout: <value>
  //  CallerID: <value>
  //  Variable: <value>
  //  Account: <value>
  //  EarlyMedia: <value>
  //  Async: <value>
  //  Codecs: <value>
  //  ChannelId: <value>
  //  OtherChannelId: <value>
  j_cmd = json_pack("{s:s, s:s, s:s, s:s, s:s}",
      "Action",         "Originate",
      "Async",          "true",

      "Channel",        channel,
      "Application",    application,
      "Data",           data
      );

  // send action request
  ret = ami_send_cmd(j_cmd);
  json_decref(j_cmd);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action");
    return false;
  }

  return true;
}
