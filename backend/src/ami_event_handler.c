/*
 * ami_event_handler.c
 *
 *  Created on: Mar 4, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <string.h>
#include <jansson.h>

#include "utils.h"
#include "slog.h"
#include "db_handler.h"
#include "ami_handler.h"
#include "action_handler.h"

#include "ami_response_handler.h"
#include "ob_ami_handler.h"
#include "ob_dialing_handler.h"

extern app* g_app;

static void ami_response_handler(json_t* j_msg);


static void ami_event_agentlogin(json_t* j_msg);
static void ami_event_agentlogoff(json_t* j_msg);
static void ami_event_agents(json_t* j_msg);
static void ami_event_dialbegin(json_t* j_msg);
static void ami_event_dialend(json_t* j_msg);
static void ami_event_hangup(json_t* j_msg);
static void ami_event_newchannel(json_t* j_msg);
static void ami_event_originateresponse(json_t* j_msg);
static void ami_event_parkinglot(json_t* j_msg);
static void ami_event_peerentry(json_t* j_msg);
static void ami_event_queuecallerabandon(json_t* j_msg);
static void ami_event_queuecallerjoin(json_t* j_msg);
static void ami_event_queuecallerleave(json_t* j_msg);
static void ami_event_queueentry(json_t* j_msg);
static void ami_event_queuemember(json_t* j_msg);
static void ami_event_queuememberadded(json_t* j_msg);
static void ami_event_queuememberstatus(json_t* j_msg);
static void ami_event_queuememberpause(json_t* j_msg);
static void ami_event_queuememberpenalty(json_t* j_msg);
static void ami_event_queuememberremoved(json_t* j_msg);
static void ami_event_queuememberringinuse(json_t* j_msg);
static void ami_event_queueparams(json_t* j_msg);
static void ami_event_registryentry(json_t* j_msg);

// action response handlers
//static ACTION_RES ami_response_handler_databaseshowall(json_t* j_action, json_t* j_msg);

/**
 * Event message handler
 * @param msg
 */
void ami_message_handler(const char* msg)
{
  json_t* j_msg;
  char* tmp;
  const char* event;
  const char* action_id;

  if(msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_message_handler.");

  // message parse
  j_msg = parse_ami_msg(msg);
  if(j_msg == NULL) {
    slog(LOG_NOTICE, "Could not parse message. msg[%s]", msg);
    return;
  }

  // get action id
  // if there's action id,
  // consider the response of action request.
  action_id = json_string_value(json_object_get(j_msg, "ActionID"));
  if(action_id != NULL) {
    ami_response_handler(j_msg);
    json_decref(j_msg);
    return;
  }

  event = json_string_value(json_object_get(j_msg, "Event"));
  if(event == NULL) {
    json_decref(j_msg);
    return;
  }
  slog(LOG_DEBUG, "Get event info. event[%s]", event);

  if(strcmp(event, "AgentCalled") == 0) {
    // need to do something later
  }
  else if(strcasecmp(event, "AgentLogin") == 0) {
    ami_event_agentlogin(j_msg);
  }
  else if(strcasecmp(event, "AgentLogoff") == 0) {
    ami_event_agentlogoff(j_msg);
  }
  else if(strcasecmp(event, "Agents") == 0) {
    ami_event_agents(j_msg);
  }
  else if(strcasecmp(event, "DialBegin") == 0) {
    ami_event_dialbegin(j_msg);
  }
  else if(strcasecmp(event, "DialEnd") == 0) {
    ami_event_dialend(j_msg);
  }
  else if(strcasecmp(event, "Hangup") == 0) {
    ami_event_hangup(j_msg);
  }
  else if(strcasecmp(event, "NewChannel") == 0) {
    ami_event_newchannel(j_msg);
  }
  else if(strcasecmp(event, "OriginateResponse") == 0) {
    ami_event_originateresponse(j_msg);
  }
  else if(strcasecmp(event, "ParkingLot")) {
    ami_event_parkinglot(j_msg);
  }
  else if(strcasecmp(event, "PeerEntry") == 0) {
    ami_event_peerentry(j_msg);
  }
  else if(strcasecmp(event, "QueueCallerAbandon") == 0) {
    ami_event_queuecallerabandon(j_msg);
  }
  else if(strcasecmp(event, "QueueCallerJoin") == 0) {
    ami_event_queuecallerjoin(j_msg);
  }
  else if(strcasecmp(event, "QueueCallerLeave") == 0) {
    ami_event_queuecallerleave(j_msg);
  }
  else if(strcasecmp(event, "QueueEntry") == 0) {
    ami_event_queueentry(j_msg);
  }
  else if(strcasecmp(event, "QueueMember") == 0) {
    ami_event_queuemember(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberAdded") == 0) {
    ami_event_queuememberadded(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberStatus") == 0) {
    ami_event_queuememberstatus(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberPause") == 0) {
    ami_event_queuememberpause(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberPenalty") == 0) {
    ami_event_queuememberpenalty(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberRemoved") == 0) {
    ami_event_queuememberremoved(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberRinginuse") == 0) {
    ami_event_queuememberringinuse(j_msg);
  }
  else if(strcasecmp(event, "QueueParams") == 0) {
    ami_event_queueparams(j_msg);
  }
  else if(strcasecmp(event, "RegistryEntry") == 0) {
    ami_event_registryentry(j_msg);
  }
  else {
    tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
    slog(LOG_DEBUG, "Could not find correct message parser. msg[%s]", tmp);
    sfree(tmp);
  }

  json_decref(j_msg);

  return;
}

static void ami_response_handler(json_t* j_msg)
{
  const char* action_id;
  const char* type;
  json_t* j_action;
  ACTION_RES res_action;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_response_handler.");

  action_id = json_string_value(json_object_get(j_msg, "ActionID"));
  if(action_id == NULL) {
    slog(LOG_NOTICE, "Could not get ActionID.");
    return;
  }

  // get action
  j_action = get_action(action_id);
  if(j_action == NULL) {
    slog(LOG_NOTICE, "Could not get action info. id[%s]", action_id);
    return;
  }

  type = json_string_value(json_object_get(j_action, "type"));
  if(type == NULL) {
    slog(LOG_ERR, "Could not get action type info. id[%s]", action_id);
    json_decref(j_action);
    return;
  }

//  // action response parse
//  if(strcasecmp(type, "command.databaseshowall") == 0) {
//    res_action = ami_response_handler_databaseshowall(j_action, j_msg);
//  }

  if(strcasecmp(type, "coresettings") == 0) {
    res_action = ami_response_handler_coresettings(j_action, j_msg);
  }
  else if(strcasecmp(type, "corestatus") == 0) {
    res_action = ami_response_handler_corestatus(j_action, j_msg);
  }

  // outbound
  else if(strcasecmp(type, "ob.originate") == 0) {
    res_action = ob_ami_response_handler_originate(j_action, j_msg);
  }
  else if(strcasecmp(type, "ob.status") == 0) {
    res_action = ob_ami_response_handler_status(j_action, j_msg);
  }
  else {
    slog(LOG_ERR, "Could not find correct action response handler. action_id[%s], type[%s]", action_id, type);
    res_action = ACTION_RES_ERROR;
  }
  json_decref(j_action);

  if(res_action == ACTION_RES_CONTINUE) {
    slog(LOG_DEBUG, "The action response is not finished. Waiting for next event. action_id[%s]", action_id);
    // continue
    return;
  }
  slog(LOG_DEBUG, "The action response if finished. action_id[%s], res[%d]", action_id, res_action);

  // delete action
  delete_action(action_id);
  return;
}

//static ACTION_RES ami_response_handler_databaseshowall(json_t* j_action, json_t* j_msg)
//{
//  char* tmp;
//  const char* tmp_const;
//  int idx;
//  json_t* j_val;
//  json_t* j_tmp;
//  char* value;
//  char* dump;
//  char* key;
//  int ret;
//  char* sql;
//
//  if(j_msg == NULL) {
//    slog(LOG_WARNING, "Wrong input parameter.");
//    return ACTION_RES_ERROR;
//  }
//  slog(LOG_DEBUG, "Fired ami_response_handler_databaseshow.");
//
//  // delete all database info.
//  asprintf(&sql, "delete from database;");
//  ret = db_exec(sql);
//  sfree(sql);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not clean up the database.");
//    return ACTION_RES_ERROR;
//  }
//
//  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
//  slog(LOG_DEBUG, "received message. msg[%s]", tmp);
//  sfree(tmp);
//
//  // need to parse
//  json_array_foreach(json_object_get(j_msg, "Output"), idx, j_val) {
//    tmp_const = json_string_value(j_val);
//
//    // get key/value
//    value = strdup(tmp_const);
//    dump = value;
//    key = strsep(&value, ":");
//    if((key == NULL) || (value == NULL)) {
//      sfree(dump);
//      continue;
//    }
//    trim(key);
//    trim(value);
//
//    slog(LOG_DEBUG, "Check database key/value. key[%s], value[%s]", key, value);
//    j_tmp = json_pack("{s:s, s:s}",
//        "key",    key,
//        "value",  value
//        );
//
//    ret = db_insert_or_replace("database", j_tmp);
//    json_decref(j_tmp);
//    if(ret == false) {
//      slog(LOG_ERR, "Could not insert/replace the database data. key[%s], value[%s]", key, value);
//    }
//
//    sfree(dump);
//  }
//
//  return ACTION_RES_COMPLETE;
//}

/**
 * AMI event handler.
 * Event: PeerEntry
 * @param j_msg
 */
static void ami_event_peerentry(json_t* j_msg)
{
  json_t* j_tmp;
  char* name;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_peerentry.");

  // create name
  asprintf(&name, "%s/%s",
      json_string_value(json_object_get(j_msg, "Channeltype"))? : "",
      json_string_value(json_object_get(j_msg, "ObjectName"))? : ""
      );

  j_tmp = json_pack("{"
    "s:s, s:s, s:s, "
    "s:s, s:s, s:s, "
    "s:s, s:s, s:s, s:s, s:s, s:s, s:s, "
    "s:s, s:s, s:s, s:s"
    "}",

    "name",         name,
    "channel_type", json_string_value(json_object_get(j_msg, "Channeltype"))? : "",
    "object_name",  json_string_value(json_object_get(j_msg, "ObjectName"))? : "",

    "chan_object_type", json_string_value(json_object_get(j_msg, "ChanObjectType"))? : "",
    "ip_address",       json_string_value(json_object_get(j_msg, "IPaddress"))? : "",
    "ip_port",          json_string_value(json_object_get(j_msg, "IPport"))? : "",

    "dynamic",          json_string_value(json_object_get(j_msg, "Dynamic"))? : "",
    "auto_force_port",  json_string_value(json_object_get(j_msg, "AutoForcerport"))? : "",
    "force_port",       json_string_value(json_object_get(j_msg, "Forcerport"))? : "",
    "auto_comedia",     json_string_value(json_object_get(j_msg, "AutoComedia"))? : "",
    "comedia",          json_string_value(json_object_get(j_msg, "Comedia"))? : "",
    "video_support",    json_string_value(json_object_get(j_msg, "VideoSupport"))? : "",
    "text_support",     json_string_value(json_object_get(j_msg, "TextSupport"))? : "",

    "acl",              json_string_value(json_object_get(j_msg, "ACL"))? : "",
    "status",           json_string_value(json_object_get(j_msg, "Status"))? : "",
    "realtime_device",  json_string_value(json_object_get(j_msg, "RealtimeDevice"))? : "",
    "description",      json_string_value(json_object_get(j_msg, "Description"))? : ""
  );
  sfree(name);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("peer", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert destination.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueParams
 * @param j_msg
 */
static void ami_event_queueparams(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queueparams.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, "
      "s:i, s:s, s:i, s:i, s:i, s:i, s:i, "
      "s:i, s:f, s:i, "
      "s:s"
      "}",

      "name",       json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "max",        json_string_value(json_object_get(j_msg, "Max"))? atoi(json_string_value(json_object_get(j_msg, "Max"))) : 0,
      "strategy",   json_string_value(json_object_get(j_msg, "Strategy"))? : "",
      "calls",      json_string_value(json_object_get(j_msg, "Calls"))? atoi(json_string_value(json_object_get(j_msg, "Calls"))) : 0,
      "hold_time",  json_string_value(json_object_get(j_msg, "Holdtime"))? atoi(json_string_value(json_object_get(j_msg, "Holdtime"))) : 0,
      "talk_time",  json_string_value(json_object_get(j_msg, "TalkTime"))? atoi(json_string_value(json_object_get(j_msg, "TalkTime"))) : 0,
      "completed",  json_string_value(json_object_get(j_msg, "Completed"))? atoi(json_string_value(json_object_get(j_msg, "Completed"))) : 0,
      "abandoned",  json_string_value(json_object_get(j_msg, "Abandoned"))? atoi(json_string_value(json_object_get(j_msg, "Abandoned"))) : 0,

      "service_level",      json_string_value(json_object_get(j_msg, "ServiceLevel"))? atoi(json_string_value(json_object_get(j_msg, "ServiceLevel"))) : 0,
      "service_level_perf", json_string_value(json_object_get(j_msg, "ServicelevelPerf"))? atof(json_string_value(json_object_get(j_msg, "ServicelevelPerf"))) : 0.0,
      "weight",             json_string_value(json_object_get(j_msg, "Weight"))? atoi(json_string_value(json_object_get(j_msg, "Weight"))) : 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_param", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_param.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMember
 * @param j_msg
 */
static void ami_event_queuemember(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuemember.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "Name"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Location"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_member", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberAdded
 * @param j_msg
 */
static void ami_event_queuememberadded(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberadded.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_member", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberPause
 * @param j_msg
 */
static void ami_event_queuememberpause(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberpause.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_member", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberPenalty
 * @param j_msg
 */
static void ami_event_queuememberpenalty(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberpenalty.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_member", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberRemoved
 * @param j_msg
 */
static void ami_event_queuememberremoved(json_t* j_msg)
{
  char* sql;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuecallerabandon.");


  asprintf(&sql, "delete from queue_member where queue_name=\"%s\" and name=\"%s\";",
      json_string_value(json_object_get(j_msg, "Queue"))? : "",
      json_string_value(json_object_get(j_msg, "MemberName"))? : ""
      );

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberRinginuse
 * @param j_msg
 */
static void ami_event_queuememberringinuse(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberringinuse.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_member", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}


/**
 * AMI event handler.
 * Event: QueueEntry
 * @param j_msg
 */
static void ami_event_queueentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queueentry.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:i, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:i, "
      "s:s"
      "}",

      "queue_name", json_string_value(json_object_get(j_msg, "Queue"))? : "",
      "position",   json_string_value(json_object_get(j_msg, "Position"))? atoi(json_string_value(json_object_get(j_msg, "Position"))) : 0,

      "channel",              json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "unique_id",            json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "wait",   json_string_value(json_object_get(j_msg, "Wait"))? atoi(json_string_value(json_object_get(j_msg, "Wait"))) : 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_entry", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueCallerAbandon
 * @param j_msg
 */
static void ami_event_queuecallerabandon(json_t* j_msg)
{
  char* sql;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuecallerabandon.");


  asprintf(&sql, "delete from queue_entry where queue_name=\"%s\" and channel=\"%s\";",
      json_string_value(json_object_get(j_msg, "Queue"))? : "",
      json_string_value(json_object_get(j_msg, "Channel"))? : ""
      );

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue_entry.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueCallerJoin
 * @param j_msg
 */
static void ami_event_queuecallerjoin(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuecallerjoin.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:i, s:s, s:s, s:s, s:s, s:s, "
      "s:s"
      "}",

      "queue_name", json_string_value(json_object_get(j_msg, "Queue"))? : "",
      "channel",              json_string_value(json_object_get(j_msg, "Channel"))? : "",

      "position",   json_string_value(json_object_get(j_msg, "Position"))? atoi(json_string_value(json_object_get(j_msg, "Position"))) : 0,
      "unique_id",            json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("queue_entry", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to queue_entry.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueCallerLeave
 * @param j_msg
 */
static void ami_event_queuecallerleave(json_t* j_msg)
{
  int ret;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired ami_event_queuecallerleave.");

  asprintf(&sql, "delete from queue_entry where queue_name=\"%s\" and position=%s;",
      json_string_value(json_object_get(j_msg, "Queue"))? : "",
      json_string_value(json_object_get(j_msg, "Position"))? : ""
      );

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue_entry.");
    return;
  }
  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberStatus
 * @param j_msg
 */
static void ami_event_queuememberstatus(json_t* j_msg)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired ami_event_queuememberstatus.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:i, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, "
      "s:i, s:s, "
      "s:s"
      "}",

      "queue_name", json_string_value(json_object_get(j_msg, "Queue"))? : "",
      "name",       json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))): 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))): 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))): 0,
      "ring_inuse", json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))): 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))): 0,

      "paused",         json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))): 0,
      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }
  slog(LOG_INFO, "Update queue member status. queue_name[%s], member_name[%s], status[%lld]",
      json_string_value(json_object_get(j_tmp, "queue_name")),
      json_string_value(json_object_get(j_tmp, "name")),
      json_integer_value(json_object_get(j_tmp, "status"))
      );

  ret = db_insert_or_replace("queue_member", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: DialBegin
 * @param j_msg
 */
static void ami_event_dialbegin(json_t* j_msg)
{
//  int hangup;
//  int ret;
//  char* sql;
//  char* tmp;
//  const char* uuid;
//  const char* tmp_const;
//  const char* hangup_detail;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_dialbegin.");

//  // check event
//  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
//  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
//  sfree(tmp);
//
//  // update ob_dialing info
//  uuid = json_string_value(json_object_get(j_msg, "DestUniqueid"));
//  update_ob_dialing_status(uuid, E_DIALING_DIAL_BEGIN);

  return;
}

/**
 * AMI event handler.
 * Event: DialEnd
 * @param j_msg
 */
static void ami_event_dialend(json_t* j_msg)
{
  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_dialend.");

//  // check event
//  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
//  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
//  sfree(tmp);
//
//  // update ob_dialing info
//  uuid = json_string_value(json_object_get(j_msg, "DestUniqueid"));
//  update_ob_dialing_status(uuid, E_DIALING_DIAL_END);

  return;
}

/**
 * AMI event handler.
 * Event: OriginateResponse
 * @param j_msg
 */
static void ami_event_originateresponse(json_t* j_msg)
{
  char* tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_originateresponse.");

  // check event
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
  sfree(tmp);

  return;
}

/**
 * AMI event handler.
 * Event: Hangup
 * @param j_msg
 */
static void ami_event_hangup(json_t* j_msg)
{
  int hangup;
  int ret;
  char* sql;
  char* tmp;
  const char* uuid;
  const char* tmp_const;
  const char* hangup_detail;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_hangup.");

  // check event
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
  sfree(tmp);

  asprintf(&sql, "delete from channel where name=\"%s\";",
      json_string_value(json_object_get(j_msg, "Channel"))
      );

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete channel.");
    return;
  }


  ///// for ob_dialing

  // get values
  uuid = json_string_value(json_object_get(j_msg, "Uniqueid"));
//  if((uuid == NULL) || (strlen(uuid) == 0)) {
//    uuid = json_string_value(json_object_get(j_msg, "Uniqueid"));
//  }
  hangup_detail = json_string_value(json_object_get(j_msg, "Cause-txt"));
  tmp_const = json_string_value(json_object_get(j_msg, "Cause"))? : 0;
  hangup = atoi(tmp_const);

  // update ob_dialing channel
  update_ob_dialing_hangup(uuid, hangup, hangup_detail);

  return;
}

/**
 * AMI event handler.
 * Event: NewChannel
 * @param j_msg
 */
static void ami_event_newchannel(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_newchannel.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, "
      "s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, s:s, s:s, "
      "s:s"
      "}",

      "name",   json_string_value(json_object_get(j_msg, "Channel"))? : "",

      "state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",
      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("channel", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to channel.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: RegistryEntry
 * @param j_msg
 */
static void ami_event_registryentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* account;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_registryentry.");

  // create db data
  asprintf(&account, "%s@%s:%d",
      json_string_value(json_object_get(j_msg, "Username"))? : "",
      json_string_value(json_object_get(j_msg, "Host"))? : "",
      json_string_value(json_object_get(j_msg, "Port"))? atoi(json_string_value(json_object_get(j_msg, "Port"))) : 0
      );
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:i, s:s,"
      "s:s, s:i, s:i, s:s, s:i"
      "s:s"
      "}",
      "account",  account,
      "host",     json_string_value(json_object_get(j_msg, "Host"))? : "",
      "port",     json_string_value(json_object_get(j_msg, "Port"))? atoi(json_string_value(json_object_get(j_msg, "Port"))): 0,
      "username", json_string_value(json_object_get(j_msg, "Username"))? : "",

      "domain",             json_string_value(json_object_get(j_msg, "Domain")),
      "domain_port",        json_string_value(json_object_get(j_msg, "DomainPort"))? atoi(json_string_value(json_object_get(j_msg, "DomainPort"))): 0,
      "refresh",            json_string_value(json_object_get(j_msg, "Refresh"))? atoi(json_string_value(json_object_get(j_msg, "Refresh"))): 0,
      "state",              json_string_value(json_object_get(j_msg, "State")),
      "registration_time",  json_string_value(json_object_get(j_msg, "RegistrationTime"))? atoi(json_string_value(json_object_get(j_msg, "RegistrationTime"))): 0,

      "tm_update",  timestamp
      );
  sfree(account);
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("registry", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to registry.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: Agents
 * @param j_msg
 */
static void ami_event_agents(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_agents.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:i, "
      "s:s, s:i, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      "id",               json_string_value(json_object_get(j_msg, "Agent"))? : "",
      "name",             json_string_value(json_object_get(j_msg, "Name"))? : "",
      "status",           json_string_value(json_object_get(j_msg, "Status"))? : "",
      "logged_in_time",   json_string_value(json_object_get(j_msg, "LoggedInTime"))? atoi(json_string_value(json_object_get(j_msg, "LoggedInTime"))) : 0,

      "channel_name",       json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))) : 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",  json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name", json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",

      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "language",     json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code", json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",  json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",    json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority", json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("agent", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to agent.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AgentLogin
 * @param j_msg
 */
static void ami_event_agentlogin(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* tmp;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_agentlogin.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      "id",               json_string_value(json_object_get(j_msg, "Agent"))? : "",
      "status",           "AGENT_IDLE",

      "channel_name",       json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))) : 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",  json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name", json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",

      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "language",     json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code", json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",  json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",    json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority", json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update agent set %s where id=\"%s\";",
      tmp,
      json_string_value(json_object_get(j_msg, "Agent"))
      );
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update agent info. agent[%s]", json_string_value(json_object_get(j_msg, "Agent")));
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AgentLogoff
 * @param j_msg
 */
static void ami_event_agentlogoff(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* tmp;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_agentlogoff.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      "id",               json_string_value(json_object_get(j_msg, "Agent"))? : "",
      "status",           "AGENT_LOGGEDOFF",

      "channel_name",       json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))) : 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",  json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name", json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",

      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "language",     json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code", json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",  json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",    json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority", json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update agent set %s where id=\"%s\";",
      tmp,
      json_string_value(json_object_get(j_msg, "Agent"))
      );
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update agent info. agent[%s]", json_string_value(json_object_get(j_msg, "Agent")));
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: Parkinglot
 * @param j_msg
 */
static void ami_event_parkinglot(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* tmp;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_parkinglot.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, "
      "s:s, s:s, "
      "s:i, "
      "s:s"
      "}",

      "name", json_string_value(json_object_get(j_msg, "Name"))? : "",

      "start_space",  json_string_value(json_object_get(j_msg, "StartSpace"))? : "",
      "stop_spcae",   json_string_value(json_object_get(j_msg, "StopSpace"))? : "",

      "timeout",  json_string_value(json_object_get(j_msg, "Timeout"))? atoi(json_string_value(json_object_get(j_msg, "Timeout"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("parking_lot", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parking_lot.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: ParkedCall
 * @param j_msg
 */
static void ami_event_parkedcall(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* tmp;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_parkedcall.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      // parked channel info
      "parkee_channel",             json_string_value(json_object_get(j_msg, "ParkeeChannel"))? : "",
      "parkee_channel_state",       json_string_value(json_object_get(j_msg, "ParkeeChannelState"))? : "",
      "parkee_channel_state_desc",  json_string_value(json_object_get(j_msg, "ParkeeChannelStateDesc"))? : "",

      // parked channel caller info
      "parkee_caller_id_num",   json_string_value(json_object_get(j_msg, "ParkeeCallerIDNum"))? : "",
      "parkee_caller_id_name",  json_string_value(json_object_get(j_msg, "ParkeeCallerIDName"))? : "",

      // parked channel connected line info
      "parkee_connected_line_num",  json_string_value(json_object_get(j_msg, "ParkeeConnectedLineNum"))? : "",
      "parkee_connected_line_name", json_string_value(json_object_get(j_msg, "ParkeeConnectedLineName"))? : "",

      // parked channel account info
      "parkee_account_code",  json_string_value(json_object_get(j_msg, "ParkeeAccountCode"))? : "",

      // parked channel dialplan info
      "parkee_context",   json_string_value(json_object_get(j_msg, "ParkeeContext"))? : "",
      "parkee_exten",     json_string_value(json_object_get(j_msg, "ParkeeExten"))? : "",
      "parkee_priority",  json_string_value(json_object_get(j_msg, "ParkeePriority"))? : "",

      // parked channel id info
      "parkee_unique_id",   json_string_value(json_object_get(j_msg, "ParkeeUniqueid"))? : "",
      "parkee_linked_id",   json_string_value(json_object_get(j_msg, "ParkeeLinkedid"))? : "",

      // parked channel parker info
      "parker_dial_string", json_string_value(json_object_get(j_msg, "ParkerDialString"))? : "",

      // parking lot info
      "parking_lot",      json_string_value(json_object_get(j_msg, "Parkinglot"))? : "",
      "parking_space",    json_string_value(json_object_get(j_msg, "ParkingSpace"))? : "",
      "parking_timeout",  json_string_value(json_object_get(j_msg, "ParkingTimeout"))? atoi(json_string_value(json_object_get(j_msg, "ParkingTimeout"))): 0,
      "parking_duration", json_string_value(json_object_get(j_msg, "ParkingDuration"))? atoi(json_string_value(json_object_get(j_msg, "ParkingDuration"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert_or_replace("parking_lot", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parking_lot.");
    return;
  }

  return;
}


