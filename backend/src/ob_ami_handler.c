/*
 * ob_ami_handler.c
 *
 *  Created on: Feb 15, 2017
 *      Author: pchero
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "ami_handler.h"
#include "action_handler.h"
#include "ob_ami_handler.h"
#include "ob_dialing_handler.h"

/**
 *
 * @param j_msg
 */
ACTION_RES ob_ami_response_handler_originate(json_t* j_action, json_t* j_msg)
{
  char* tmp;
  const char* response;
  const char* event;
  const char* action_id;
  const char* uuid;
  const char* channel;
  const char* tmp_const;
  json_t* j_dialing;
  int ret;
  int res_dial;
  bool success;

  if((j_action == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return ACTION_RES_ERROR;
  }
  slog(LOG_DEBUG, "Fired ob_ami_response_handler_originate.");

  // for debug
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Received message. tmp[%s]", tmp);
  sfree(tmp);

  // get action id
  action_id = json_string_value(json_object_get(j_msg, "ActionID"));
  if(action_id == NULL) {
    slog(LOG_ERR, "Could not get action id.");
    return ACTION_RES_ERROR;
  }

  // get dialing info using action id.
  j_dialing = get_ob_dialing_by_action_id(action_id);
  if(j_dialing == NULL) {
    slog(LOG_ERR, "Could not get ob_dialing info. action_id[%s]", action_id);
    return ACTION_RES_ERROR;
  }

  // get event
  // if there's no event, consider response message
  event = json_string_value(json_object_get(j_msg, "Event"));
  if(event == NULL) {
    // get response
    response = json_string_value(json_object_get(j_msg, "Response"));
    if(response == NULL) {
      return ACTION_RES_ERROR;
    }

    ret = strcasecmp(response, "Success");
    if(ret != 0) {
      // update dialing status to dialing failed
      slog(LOG_INFO, "Could not success ob dialing. message[%s]",
          json_string_value(json_object_get(j_msg, "Message"))
          );
      update_ob_dialing_status(json_string_value(json_object_get(j_dialing, "uuid")), E_DIALING_ERROR_ORIGINATE_QUEUED_FAILED);
      json_decref(j_dialing);
      return ACTION_RES_COMPLETE;
    }

    // update dialing status to dialing ok
    update_ob_dialing_status(json_string_value(json_object_get(j_dialing, "uuid")), E_DIALING_ORIGINATE_QUEUED);
    json_decref(j_dialing);
    return ACTION_RES_CONTINUE;
  }

  if(strcasecmp(event, "OriginateResponse") == 0) {
    // Event: OriginateResponse

    // get uuid
    uuid = json_string_value(json_object_get(j_msg, "Uniqueid"));

    // get channel
    channel = json_string_value(json_object_get(j_msg, "Channel"));

    // get res_dial
    tmp_const = json_string_value(json_object_get(j_msg, "Response"));
    if(tmp_const == NULL) {
      slog(LOG_ERR, "Could not get originate response.");
      return ACTION_RES_ERROR;
    }
    success = true;
    if(strcasecmp(tmp_const, "Failure") == 0) {
      success = false;
    }

    // get reason
    tmp_const = json_string_value(json_object_get(j_msg, "Reason"));
    if(tmp_const == NULL) {
      slog(LOG_ERR, "Could not get originate response reason code.");
      return ACTION_RES_ERROR;
    }
    res_dial = atoi(tmp_const);

    // update ob_dialing
    update_ob_dialing_res_dial(uuid, success, res_dial, channel);
    return ACTION_RES_COMPLETE;
  }

  // should not reach to here.
  slog(LOG_ERR, "Something was wrong. Should not reach to here.");
  return ACTION_RES_ERROR;


//  // get dialing info using action id.
//  j_dialing = get_ob_dialing_by_action_id(action_id);
//  if(j_dialing == NULL) {
//    slog(LOG_ERR, "Could not get ob_dialing info. action_id[%s]", action_id);
//    return ACTION_RES_ERROR;
//  }
//
//  if(strcmp(response, "Success") == 0) {
//    // update dialing status to dialing ok
//    update_ob_dialing_status(json_string_value(json_object_get(j_dialing, "uuid")), E_DIALING_ORIGINATE_QUEUED);
//  }
//  else {
//    // update dialing status to dialing failed
//    update_ob_dialing_status(json_string_value(json_object_get(j_dialing, "uuid")), E_DIALING_ORIGINATE_QUEUED_FAILED);
//    slog(LOG_INFO, "Could not outbound dialing. message[%s]",
//        json_string_value(json_object_get(j_msg, "Message"))
//        );
//  }
//  json_decref(j_dialing);
//
//  return ACTION_RES_COMPLETE;
}

/**
 *
 * @param j_msg
 */
ACTION_RES ob_ami_response_handler_status(json_t* j_action, json_t* j_msg)
{
  char* tmp;
  const char* event;
  const char* response;
  const char* unique_id;
  int ret;

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
  event = json_string_value(json_object_get(j_msg, "Event"));
  if(event == NULL) {
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
    return ACTION_RES_CONTINUE;
  }

  // check complete
  ret = strcasecmp(event, "StatusComplete");
  if(ret == 0) {
    return ACTION_RES_COMPLETE;
  }

  // get unique id
  unique_id = json_string_value(json_object_get(j_msg, "Uniqueid"));
  if(unique_id == NULL) {
    slog(LOG_ERR, "Could not get unique id.");
    return ACTION_RES_CONTINUE;
  }

  // update update time
  ret = update_ob_dialing_timestamp(unique_id);
  slog(LOG_DEBUG, "Updated dialing timestamp. uuid[%s], ret[%d]", unique_id, ret);

  return ACTION_RES_CONTINUE;
}

/**
 * Originate the call and send to extension.
 * @param name
 * @return
 */
bool originate_to_exten(json_t* j_dialing)
{
  json_t* j_cmd;
  int ret;
  char* tmp;

  if(j_dialing == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired originate_to_exten.");

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
  j_cmd = json_pack("{s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s}",
      "Action",     "Originate",
      "ActionID",   json_string_value(json_object_get(j_dialing, "action_id"))? : "",
      "Async",      "true",
      "Channel",    json_string_value(json_object_get(j_dialing, "dial_channel"))? : "",
      "Exten",      json_string_value(json_object_get(j_dialing, "dial_exten"))? : "",
      "Context",    json_string_value(json_object_get(j_dialing, "dial_context"))? : "",
      "Priority",   json_string_value(json_object_get(j_dialing, "dial_priority"))? : "",
      "ChannelId",  json_string_value(json_object_get(j_dialing, "uuid"))
      );
  if(j_cmd == NULL) {
    slog(LOG_ERR, "Could not create default originate request info.");
    return false;
  }

  if(json_object_get(j_dialing, "dial_timeout") != NULL) {
    json_object_set(j_cmd, "Timeout", json_incref(json_object_get(j_dialing, "dial_timeout")));
  }

  if(json_object_get(j_dialing, "callerid") != NULL) {
    json_object_set(j_cmd, "CallerID", json_incref(json_object_get(j_dialing, "callerid")));
  }

  if(json_object_get(j_dialing, "account") != NULL) {
    json_object_set(j_cmd, "Account", json_incref(json_object_get(j_dialing, "account")));
  }

  if(json_object_get(j_dialing, "early_media") != NULL) {
    json_object_set(j_cmd, "EarlyMedia", json_incref(json_object_get(j_dialing, "early_media")));
  }

  if(json_object_get(j_dialing, "codecs") != NULL) {
    json_object_set(j_cmd, "Codecs", json_incref(json_object_get(j_dialing, "codecs")));
  }

  if(json_object_get(j_dialing, "channelid") != NULL) {
    json_object_set(j_cmd, "ChannelId", json_incref(json_object_get(j_dialing, "channelid")));
  }

  if(json_object_get(j_dialing, "otherchannelid") != NULL) {
    json_object_set(j_cmd, "OtherChannelId", json_incref(json_object_get(j_dialing, "otherchannelid")));
  }

  // Variables
  if(json_object_get(j_dialing, "variables") != NULL) {
    json_object_set(j_cmd, "Variables", json_incref(json_object_get(j_dialing, "variables")));
  }

  // dump command string
  if(j_cmd == NULL) {
    slog(LOG_ERR, "Could not create ami json.");
    return NULL;
  }
  tmp = json_dumps(j_cmd, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Dialing. tmp[%s]\n", tmp);
  sfree(tmp);

  ret = send_ami_cmd(j_cmd);
  json_decref(j_cmd);
  if(ret == false) {
    slog(LOG_ERR, "Could not send originate extension request.");
    return false;
  }
  insert_action(json_string_value(json_object_get(j_dialing, "action_id")), "ob.originate");

  return true;
}

/**
 * Originate the call and send to application.
 * @param name
 * @return
 */
bool originate_to_application(json_t* j_dialing)
{
  json_t* j_cmd;
  int ret;
  char* tmp;

  if(j_dialing == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired originate_to_application.");

  tmp = json_dumps(j_dialing, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Dialing info. tmp[%s]", tmp);
  sfree(tmp);

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
  j_cmd = json_pack("{s:s, s:s, s:s, s:s, s:s, s:s, s:s}",
      "Action",       "Originate",
      "ActionID",     json_string_value(json_object_get(j_dialing, "action_id")),
      "Async",        "true",
      "Channel",      json_string_value(json_object_get(j_dialing, "dial_channel")),
      "Application",  json_string_value(json_object_get(j_dialing, "dial_application"))? : "",
      "Data",         json_string_value(json_object_get(j_dialing, "dial_data"))? : "",
      "ChannelId",    json_string_value(json_object_get(j_dialing, "uuid"))
      );
  if(j_cmd == NULL) {
    slog(LOG_ERR, "Could not create default originate request info.");
    return false;
  }

  if(json_object_get(j_dialing, "dial_timeout") != NULL) {
    json_object_set(j_cmd, "Timeout", json_incref(json_object_get(j_dialing, "dial_timeout")));
  }

  if(json_object_get(j_dialing, "callerid") != NULL) {
    json_object_set(j_cmd, "CallerID", json_incref(json_object_get(j_dialing, "callerid")));
  }

  if(json_object_get(j_dialing, "variable") != NULL) {
    json_object_set(j_cmd, "Variable", json_incref(json_object_get(j_dialing, "variable")));
  }

  if(json_object_get(j_dialing, "account") != NULL) {
    json_object_set(j_cmd, "Account", json_incref(json_object_get(j_dialing, "account")));
  }

  if(json_object_get(j_dialing, "early_media") != NULL) {
    json_object_set(j_cmd, "EarlyMedia", json_incref(json_object_get(j_dialing, "early_media")));
  }

  if(json_object_get(j_dialing, "codecs") != NULL) {
    json_object_set(j_cmd, "Codecs", json_incref(json_object_get(j_dialing, "codecs")));
  }

  if(json_object_get(j_dialing, "channelid") != NULL) {
    json_object_set(j_cmd, "ChannelId", json_incref(json_object_get(j_dialing, "channelid")));
  }

  if(json_object_get(j_dialing, "otherchannelid") != NULL) {
    json_object_set(j_cmd, "OtherChannelId", json_incref(json_object_get(j_dialing, "otherchannelid")));
  }

  if(j_cmd == NULL) {
    slog(LOG_ERR, "Could not create ami json.");
    return false;
  }
  tmp = json_dumps(j_cmd, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Dialing. tmp[%s]\n", tmp);
  sfree(tmp);

  ret = send_ami_cmd(j_cmd);
  json_decref(j_cmd);
  if(ret == false) {
    slog(LOG_ERR, "Could not send originate application request.");
    return false;
  }
  insert_action(json_string_value(json_object_get(j_dialing, "action_id")), "ob.originate");

  return true;
}
