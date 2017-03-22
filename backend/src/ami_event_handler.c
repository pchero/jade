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

#include "ob_ami_handler.h"
#include "ob_dialing_handler.h"

extern app* g_app;

static void ami_response_handler(json_t* j_msg);

static void ami_event_dialbegin(json_t* j_msg);
static void ami_event_dialend(json_t* j_msg);
static void ami_event_hangup(json_t* j_msg);
static void ami_event_newchannel(json_t* j_msg);
static void ami_event_originateresponse(json_t* j_msg);
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

static void ami_event_outdestinationentry(json_t* j_msg);
static void ami_event_outdestinationcreate(json_t* j_msg);
static void ami_event_outdestinationupdate(json_t* j_msg);
static void ami_event_outdestinationdelete(json_t* j_msg);
static void ami_event_outplanentry(json_t* j_msg);
static void ami_event_outplancreate(json_t* j_msg);
static void ami_event_outplanupdate(json_t* j_msg);
static void ami_event_outplandelete(json_t* j_msg);
static void ami_event_outcampaignentry(json_t* j_msg);
static void ami_event_outcampaigncreate(json_t* j_msg);
static void ami_event_outcampaignupdate(json_t* j_msg);
static void ami_event_outcampaigndelete(json_t* j_msg);
static void ami_event_outdlmaentry(json_t* j_msg);
static void ami_event_outdlmacreate(json_t* j_msg);
static void ami_event_outdlmaupdate(json_t* j_msg);
static void ami_event_outdlmadelete(json_t* j_msg);
static void ami_event_outdllistentry(json_t* j_msg);
static void ami_event_outdllistcreate(json_t* j_msg);
static void ami_event_outdllistupdate(json_t* j_msg);
static void ami_event_outdllistdelete(json_t* j_msg);

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


  else if(strcmp(event, "OutDestinationEntry") == 0) {
    ami_event_outdestinationentry(j_msg);
  }
  else if(strcmp(event, "OutDestinationCreate") == 0) {
    ami_event_outdestinationcreate(j_msg);
  }
  else if(strcmp(event, "OutDestinationUpdate") == 0) {
    ami_event_outdestinationupdate(j_msg);
  }
  else if(strcmp(event, "OutDestinationDelete") == 0) {
    ami_event_outdestinationdelete(j_msg);
  }
  else if(strcmp(event, "OutPlanEntry") == 0) {
    ami_event_outplanentry(j_msg);
  }
  else if(strcmp(event, "OutPlanCreate") == 0) {
    ami_event_outplancreate(j_msg);
  }
  else if(strcmp(event, "OutPlanUpdate") == 0) {
    ami_event_outplanupdate(j_msg);
  }
  else if(strcmp(event, "OutPlanDelete") == 0) {
    ami_event_outplandelete(j_msg);
  }
  else if(strcmp(event, "OutCampaignEntry") == 0) {
    ami_event_outcampaignentry(j_msg);
  }
  else if(strcmp(event, "OutCampaignCreate") == 0) {
    ami_event_outcampaigncreate(j_msg);
  }
  else if(strcmp(event, "OutCampaignUpdate") == 0) {
    ami_event_outcampaignupdate(j_msg);
  }
  else if(strcmp(event, "OutCampaignDelete") == 0) {
    ami_event_outcampaigndelete(j_msg);
  }
  else if(strcmp(event, "OutDlmaEntry") == 0) {
    ami_event_outdlmaentry(j_msg);
  }
  else if(strcmp(event, "OutDlmaCreate") == 0) {
    ami_event_outdlmacreate(j_msg);
  }
  else if(strcmp(event, "OutDlmaUpdate") == 0) {
    ami_event_outdlmaupdate(j_msg);
  }
  else if(strcmp(event, "OutDlmaDelete") == 0) {
    ami_event_outdlmadelete(j_msg);
  }
  else if(strcmp(event, "OutDlListEntry") == 0) {
    ami_event_outdllistentry(j_msg);
  }
  else if(strcmp(event, "OutDlListCreate") == 0) {
    ami_event_outdllistcreate(j_msg);
  }
  else if(strcmp(event, "OutDlListUpdate") == 0) {
    ami_event_outdllistupdate(j_msg);
  }
  else if(strcmp(event, "OutDlListDelete") == 0) {
    ami_event_outdllistdelete(j_msg);
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
  if(strcasecmp(type, "ob.originate") == 0) {
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
 * Event: OutDestinationEntry
 * @param j_msg
 */
static void ami_event_outdestinationentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdestinationentry.");

  j_tmp = json_pack("{"
    "s:s, s:s, s:s, s:i, s:s, "
    "s:s, s:s, s:s, s:s, s:s, "
    "s:s, s:s, s:s"
    "}",

    "uuid",         json_string_value(json_object_get(j_msg, "Uuid"))? : "",
    "name",         json_string_value(json_object_get(j_msg, "Name"))? : "",
    "detail",       json_string_value(json_object_get(j_msg, "Detail"))? : "",
    "type",         json_string_value(json_object_get(j_msg, "Type"))? atoi(json_string_value(json_object_get(j_msg, "Type"))) : 0,
    "exten",        json_string_value(json_object_get(j_msg, "Exten"))? : "",

    "context",      json_string_value(json_object_get(j_msg, "Context"))? : "",
    "priority",     json_string_value(json_object_get(j_msg, "Priority"))? : "",
    "variables",    json_string_value(json_object_get(j_msg, "Variable"))? : "",
    "application",  json_string_value(json_object_get(j_msg, "Application"))? : "",
    "data",         json_string_value(json_object_get(j_msg, "Data"))? : "",

    "tm_create",    json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
    "tm_update",    json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
    "tm_delete",    json_string_value(json_object_get(j_msg, "TmpDelete"))? : ""
  );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_destination", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert destination.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDestinationCreate
 * @param j_msg
 */
static void ami_event_outdestinationcreate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdestinationcreate.");

  j_tmp = json_pack("{"
    "s:s, s:s, s:s, s:i, s:s, "
    "s:s, s:s, s:s, s:s, s:s, "
    "s:s, s:s, s:s"
    "}",

    "uuid",         json_string_value(json_object_get(j_msg, "Uuid"))? : "",
    "name",         json_string_value(json_object_get(j_msg, "Name"))? : "",
    "detail",       json_string_value(json_object_get(j_msg, "Detail"))? : "",
    "type",         json_string_value(json_object_get(j_msg, "Type"))? atoi(json_string_value(json_object_get(j_msg, "Type"))) : 0,
    "exten",        json_string_value(json_object_get(j_msg, "Exten"))? : "",

    "context",      json_string_value(json_object_get(j_msg, "Context"))? : "",
    "priority",     json_string_value(json_object_get(j_msg, "Priority"))? : "",
    "variables",    json_string_value(json_object_get(j_msg, "Variable"))? : "",
    "application",  json_string_value(json_object_get(j_msg, "Application"))? : "",
    "data",         json_string_value(json_object_get(j_msg, "Data"))? : "",

    "tm_create",    json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
    "tm_update",    json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
    "tm_delete",    json_string_value(json_object_get(j_msg, "TmpDelete"))? : ""
  );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_destination", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert destination.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDestinationUpdate
 * @param j_msg
 */
static void ami_event_outdestinationupdate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdestinationupdate.");

  j_tmp = json_pack("{"
    "s:s, s:s, s:s, s:i, s:s, "
    "s:s, s:s, s:s, s:s, s:s, "
    "s:s, s:s, s:s"
    "}",

    "uuid",         json_string_value(json_object_get(j_msg, "Uuid"))? : "",
    "name",         json_string_value(json_object_get(j_msg, "Name"))? : "",
    "detail",       json_string_value(json_object_get(j_msg, "Detail"))? : "",
    "type",         json_string_value(json_object_get(j_msg, "Type"))? atoi(json_string_value(json_object_get(j_msg, "Type"))) : 0,
    "exten",        json_string_value(json_object_get(j_msg, "Exten"))? : "",

    "context",      json_string_value(json_object_get(j_msg, "Context"))? : "",
    "priority",     json_string_value(json_object_get(j_msg, "Priority"))? : "",
    "variables",    json_string_value(json_object_get(j_msg, "Variable"))? : "",
    "application",  json_string_value(json_object_get(j_msg, "Application"))? : "",
    "data",         json_string_value(json_object_get(j_msg, "Data"))? : "",

    "tm_create",    json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
    "tm_update",    json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
    "tm_delete",    json_string_value(json_object_get(j_msg, "TmpDelete"))? : ""
  );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_destination", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert destination.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDestinationDelete
 * @param j_msg
 */
static void ami_event_outdestinationdelete(json_t* j_msg)
{
  int ret;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdestinationdelete.");

  asprintf(&sql, "delete from ob_destination where uuid=\"%s\";",
      json_string_value(json_object_get(j_msg, "Uuid"))? : ""
      );
  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete destination.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutPlanEntry
 * @param j_msg
 */
static void ami_event_outplanentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outplanentry.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:i, s:i, s:s, s:i, s:i, "
      "s:s, s:s, s:i, s:s, s:s, s:s, "
      "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:s, s:s"
      "}",

      "uuid",             json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",             json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",           json_string_value(json_object_get(j_msg, "Detail"))? : "",

      "dial_mode",      json_string_value(json_object_get(j_msg, "DialMode"))? atoi(json_string_value(json_object_get(j_msg, "DialMode"))): 0,
      "dial_timeout",   json_string_value(json_object_get(j_msg, "DialTimeout"))? atoi(json_string_value(json_object_get(j_msg, "DialTimeout"))): 0,
      "caller_id",      json_string_value(json_object_get(j_msg, "CallerId"))? : "",
      "dl_end_handle",  json_string_value(json_object_get(j_msg, "DlEndHandle"))? atoi(json_string_value(json_object_get(j_msg, "DlEndHandle"))): 0,
      "retry_delay",    json_string_value(json_object_get(j_msg, "RetryDelay"))? atoi(json_string_value(json_object_get(j_msg, "RetryDelay"))): 0,

      "trunk_name",     json_string_value(json_object_get(j_msg, "TrunkName"))? : "",
      "tech_name",      json_string_value(json_object_get(j_msg, "TechName"))? : "",
      "service_level",  json_string_value(json_object_get(j_msg, "ServiceLevel"))? atoi(json_string_value(json_object_get(j_msg, "ServiceLevel"))): 0,
      "early_media",    json_string_value(json_object_get(j_msg, "EarlyMedia"))? : "",
      "codecs",         json_string_value(json_object_get(j_msg, "Codecs"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "max_retry_cnt_1",  json_string_value(json_object_get(j_msg, "MaxRetryCnt1"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt1"))): 0,
      "max_retry_cnt_2",  json_string_value(json_object_get(j_msg, "MaxRetryCnt2"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt2"))): 0,
      "max_retry_cnt_3",  json_string_value(json_object_get(j_msg, "MaxRetryCnt3"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt3"))): 0,
      "max_retry_cnt_4",  json_string_value(json_object_get(j_msg, "MaxRetryCnt4"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt4"))): 0,
      "max_retry_cnt_5",  json_string_value(json_object_get(j_msg, "MaxRetryCnt5"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt5"))): 0,
      "max_retry_cnt_6",  json_string_value(json_object_get(j_msg, "MaxRetryCnt6"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt6"))): 0,
      "max_retry_cnt_7",  json_string_value(json_object_get(j_msg, "MaxRetryCnt7"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt7"))): 0,
      "max_retry_cnt_8",  json_string_value(json_object_get(j_msg, "MaxRetryCnt8"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt8"))): 0,

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_plan", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert plan.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutPlanCreate
 * @param j_msg
 */
static void ami_event_outplancreate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outplancreate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:i, s:i, s:s, s:i, s:i, "
      "s:s, s:s, s:i, s:s, s:s, s:s, "
      "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:s, s:s"
      "}",

      "uuid",             json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",             json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",           json_string_value(json_object_get(j_msg, "Detail"))? : "",

      "dial_mode",      json_string_value(json_object_get(j_msg, "DialMode"))? atoi(json_string_value(json_object_get(j_msg, "DialMode"))): 0,
      "dial_timeout",   json_string_value(json_object_get(j_msg, "DialTimeout"))? atoi(json_string_value(json_object_get(j_msg, "DialTimeout"))): 0,
      "caller_id",      json_string_value(json_object_get(j_msg, "CallerId"))? : "",
      "dl_end_handle",  json_string_value(json_object_get(j_msg, "DlEndHandle"))? atoi(json_string_value(json_object_get(j_msg, "DlEndHandle"))): 0,
      "retry_delay",    json_string_value(json_object_get(j_msg, "RetryDelay"))? atoi(json_string_value(json_object_get(j_msg, "RetryDelay"))): 0,

      "trunk_name",     json_string_value(json_object_get(j_msg, "TrunkName"))? : "",
      "tech_name",      json_string_value(json_object_get(j_msg, "TechName"))? : "",
      "service_level",  json_string_value(json_object_get(j_msg, "ServiceLevel"))? atoi(json_string_value(json_object_get(j_msg, "ServiceLevel"))): 0,
      "early_media",    json_string_value(json_object_get(j_msg, "EarlyMedia"))? : "",
      "codecs",         json_string_value(json_object_get(j_msg, "Codecs"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "max_retry_cnt_1",  json_string_value(json_object_get(j_msg, "MaxRetryCnt1"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt1"))): 0,
      "max_retry_cnt_2",  json_string_value(json_object_get(j_msg, "MaxRetryCnt2"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt2"))): 0,
      "max_retry_cnt_3",  json_string_value(json_object_get(j_msg, "MaxRetryCnt3"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt3"))): 0,
      "max_retry_cnt_4",  json_string_value(json_object_get(j_msg, "MaxRetryCnt4"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt4"))): 0,
      "max_retry_cnt_5",  json_string_value(json_object_get(j_msg, "MaxRetryCnt5"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt5"))): 0,
      "max_retry_cnt_6",  json_string_value(json_object_get(j_msg, "MaxRetryCnt6"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt6"))): 0,
      "max_retry_cnt_7",  json_string_value(json_object_get(j_msg, "MaxRetryCnt7"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt7"))): 0,
      "max_retry_cnt_8",  json_string_value(json_object_get(j_msg, "MaxRetryCnt8"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt8"))): 0,

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_plan", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert plan.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutPlanUpdate
 * @param j_msg
 */
static void ami_event_outplanupdate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outplanupdate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:i, s:i, s:s, s:i, s:i, "
      "s:s, s:s, s:i, s:s, s:s, s:s, "
      "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:s, s:s"
      "}",

      "uuid",             json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",             json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",           json_string_value(json_object_get(j_msg, "Detail"))? : "",

      "dial_mode",      json_string_value(json_object_get(j_msg, "DialMode"))? atoi(json_string_value(json_object_get(j_msg, "DialMode"))): 0,
      "dial_timeout",   json_string_value(json_object_get(j_msg, "DialTimeout"))? atoi(json_string_value(json_object_get(j_msg, "DialTimeout"))): 0,
      "caller_id",      json_string_value(json_object_get(j_msg, "CallerId"))? : "",
      "dl_end_handle",  json_string_value(json_object_get(j_msg, "DlEndHandle"))? atoi(json_string_value(json_object_get(j_msg, "DlEndHandle"))): 0,
      "retry_delay",    json_string_value(json_object_get(j_msg, "RetryDelay"))? atoi(json_string_value(json_object_get(j_msg, "RetryDelay"))): 0,

      "trunk_name",     json_string_value(json_object_get(j_msg, "TrunkName"))? : "",
      "tech_name",      json_string_value(json_object_get(j_msg, "TechName"))? : "",
      "service_level",  json_string_value(json_object_get(j_msg, "ServiceLevel"))? atoi(json_string_value(json_object_get(j_msg, "ServiceLevel"))): 0,
      "early_media",    json_string_value(json_object_get(j_msg, "EarlyMedia"))? : "",
      "codecs",         json_string_value(json_object_get(j_msg, "Codecs"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "max_retry_cnt_1",  json_string_value(json_object_get(j_msg, "MaxRetryCnt1"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt1"))): 0,
      "max_retry_cnt_2",  json_string_value(json_object_get(j_msg, "MaxRetryCnt2"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt2"))): 0,
      "max_retry_cnt_3",  json_string_value(json_object_get(j_msg, "MaxRetryCnt3"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt3"))): 0,
      "max_retry_cnt_4",  json_string_value(json_object_get(j_msg, "MaxRetryCnt4"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt4"))): 0,
      "max_retry_cnt_5",  json_string_value(json_object_get(j_msg, "MaxRetryCnt5"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt5"))): 0,
      "max_retry_cnt_6",  json_string_value(json_object_get(j_msg, "MaxRetryCnt6"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt6"))): 0,
      "max_retry_cnt_7",  json_string_value(json_object_get(j_msg, "MaxRetryCnt7"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt7"))): 0,
      "max_retry_cnt_8",  json_string_value(json_object_get(j_msg, "MaxRetryCnt8"))? atoi(json_string_value(json_object_get(j_msg, "MaxRetryCnt8"))): 0,

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_plan", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert plan.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutPlanDelete
 * @param j_msg
 */
static void ami_event_outplandelete(json_t* j_msg)
{
  int ret;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outplandelete.");

  asprintf(&sql, "delete from ob_plan where uuid=\"%s\";",
      json_string_value(json_object_get(j_msg, "Uuid"))? : ""
      );
  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete plan.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutCampaignEntry
 * @param j_msg
 */
static void ami_event_outcampaignentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outcampaignentry.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, "
      "s:i, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "status",         json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,
      "next_campaign",  json_string_value(json_object_get(j_msg, "NextCampaign"))? : "",

      "dlma",       json_string_value(json_object_get(j_msg, "Dlma"))? : "",
      "dest",       json_string_value(json_object_get(j_msg, "Dest"))? : "",
      "plan",       json_string_value(json_object_get(j_msg, "Plan"))? : "",
      "variables",  json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "sc_mode",              json_string_value(json_object_get(j_msg, "ScMode"))? atoi(json_string_value(json_object_get(j_msg, "ScMode"))): 0,
      "sc_date_start",        json_string_value(json_object_get(j_msg, "ScDateStart"))? : "",
      "sc_date_end",          json_string_value(json_object_get(j_msg, "ScDateEnd"))? : "",
      "sc_date_list",         json_string_value(json_object_get(j_msg, "ScDateList"))? : "",
      "sc_date_list_except",  json_string_value(json_object_get(j_msg, "ScDateListExcept"))? : "",

      "sc_time_start",  json_string_value(json_object_get(j_msg, "ScTimeStart"))? : "",
      "sc_time_end",  json_string_value(json_object_get(j_msg, "ScTimeEnd"))? : "",
      "sc_day_list",  json_string_value(json_object_get(j_msg, "ScDayList"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_campaign", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert campaign.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutCampaignCreate
 * @param j_msg
 */
static void ami_event_outcampaigncreate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outcampaigncreate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, "
      "s:i, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "status",         json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,
      "next_campaign",  json_string_value(json_object_get(j_msg, "NextCampaign"))? : "",

      "dlma",       json_string_value(json_object_get(j_msg, "Dlma"))? : "",
      "dest",       json_string_value(json_object_get(j_msg, "Dest"))? : "",
      "plan",       json_string_value(json_object_get(j_msg, "Plan"))? : "",
      "variables",  json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "sc_mode",              json_string_value(json_object_get(j_msg, "ScMode"))? atoi(json_string_value(json_object_get(j_msg, "ScMode"))): 0,
      "sc_date_start",        json_string_value(json_object_get(j_msg, "ScDateStart"))? : "",
      "sc_date_end",          json_string_value(json_object_get(j_msg, "ScDateEnd"))? : "",
      "sc_date_list",         json_string_value(json_object_get(j_msg, "ScDateList"))? : "",
      "sc_date_list_except",  json_string_value(json_object_get(j_msg, "ScDateListExcept"))? : "",

      "sc_time_start",  json_string_value(json_object_get(j_msg, "ScTimeStart"))? : "",
      "sc_time_end",  json_string_value(json_object_get(j_msg, "ScTimeEnd"))? : "",
      "sc_day_list",  json_string_value(json_object_get(j_msg, "ScDayList"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_campaign", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert campaign.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutCampaignUpdate
 * @param j_msg
 */
static void ami_event_outcampaignupdate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outcampaignupdate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, "
      "s:i, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "status",         json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,
      "next_campaign",  json_string_value(json_object_get(j_msg, "NextCampaign"))? : "",

      "dlma",       json_string_value(json_object_get(j_msg, "Dlma"))? : "",
      "dest",       json_string_value(json_object_get(j_msg, "Dest"))? : "",
      "plan",       json_string_value(json_object_get(j_msg, "Plan"))? : "",
      "variables",  json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "sc_mode",              json_string_value(json_object_get(j_msg, "ScMode"))? atoi(json_string_value(json_object_get(j_msg, "ScMode"))): 0,
      "sc_date_start",        json_string_value(json_object_get(j_msg, "ScDateStart"))? : "",
      "sc_date_end",          json_string_value(json_object_get(j_msg, "ScDateEnd"))? : "",
      "sc_date_list",         json_string_value(json_object_get(j_msg, "ScDateList"))? : "",
      "sc_date_list_except",  json_string_value(json_object_get(j_msg, "ScDateListExcept"))? : "",

      "sc_time_start",  json_string_value(json_object_get(j_msg, "ScTimeStart"))? : "",
      "sc_time_end",  json_string_value(json_object_get(j_msg, "ScTimeEnd"))? : "",
      "sc_day_list",  json_string_value(json_object_get(j_msg, "ScDayList"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_campaign", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert campaign.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutCampaignDelete
 * @param j_msg
 */
static void ami_event_outcampaigndelete(json_t* j_msg)
{
  int ret;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outcampaigndelete.");

  asprintf(&sql, "delete from ob_campaign where uuid=\"%s\";",
      json_string_value(json_object_get(j_msg, "Uuid"))? : ""
      );
  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete campaign.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlmaEntry
 * @param j_msg
 */
static void ami_event_outdlmaentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdlmaentry.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "dl_table",  json_string_value(json_object_get(j_msg, "DlTable"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_dl_list_ma", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dlma.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlmaCreate
 * @param j_msg
 */
static void ami_event_outdlmacreate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdlmacreate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "dl_table",  json_string_value(json_object_get(j_msg, "DlTable"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_dl_list_ma", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dlma.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlmaUpdate
 * @param j_msg
 */
static void ami_event_outdlmaupdate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdlmaupdate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "dl_table",  json_string_value(json_object_get(j_msg, "DlTable"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_dl_list_ma", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dlma.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlmaDelete
 * @param j_msg
 */
static void ami_event_outdlmadelete(json_t* j_msg)
{
  int ret;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdlmadelete.");

  asprintf(&sql, "delete from ob_dlma where uuid=\"%s\";",
      json_string_value(json_object_get(j_msg, "Uuid"))? : ""
      );
  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete dlma.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlListEntry
 * @param j_msg
 */
static void ami_event_outdllistentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdllistentry.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, s:i, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, "
      "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, "
      "s:i, s:s, s:i, s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "dlma_uuid",      json_string_value(json_object_get(j_msg, "DlmaUuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "status",         json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,
      "ukey",           json_string_value(json_object_get(j_msg, "UKey"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "dialing_uuid",       json_string_value(json_object_get(j_msg, "DialingUuid"))? : "",
      "dialing_camp_uuid",  json_string_value(json_object_get(j_msg, "DialingCampUuid"))? : "",
      "dialing_plan_uuid",  json_string_value(json_object_get(j_msg, "DialingPlanUuid"))? : "",

      "number_1",       json_string_value(json_object_get(j_msg, "Number1"))? : "",
      "number_2",       json_string_value(json_object_get(j_msg, "Number2"))? : "",
      "number_3",       json_string_value(json_object_get(j_msg, "Number3"))? : "",
      "number_4",       json_string_value(json_object_get(j_msg, "Number4"))? : "",
      "number_5",       json_string_value(json_object_get(j_msg, "Number5"))? : "",
      "number_6",       json_string_value(json_object_get(j_msg, "Number6"))? : "",
      "number_7",       json_string_value(json_object_get(j_msg, "Number7"))? : "",
      "number_8",       json_string_value(json_object_get(j_msg, "Number8"))? : "",

      "email",       json_string_value(json_object_get(j_msg, "Email"))? : "",

      "trycnt_1",       json_string_value(json_object_get(j_msg, "TryCnt1"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt1"))): 0,
      "trycnt_2",       json_string_value(json_object_get(j_msg, "TryCnt2"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt2"))): 0,
      "trycnt_3",       json_string_value(json_object_get(j_msg, "TryCnt3"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt3"))): 0,
      "trycnt_4",       json_string_value(json_object_get(j_msg, "TryCnt4"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt4"))): 0,
      "trycnt_5",       json_string_value(json_object_get(j_msg, "TryCnt5"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt5"))): 0,
      "trycnt_6",       json_string_value(json_object_get(j_msg, "TryCnt6"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt6"))): 0,
      "trycnt_7",       json_string_value(json_object_get(j_msg, "TryCnt7"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt7"))): 0,
      "trycnt_8",       json_string_value(json_object_get(j_msg, "TryCnt8"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt8"))): 0,

      "res_dial",           json_string_value(json_object_get(j_msg, "ResDial"))? atoi(json_string_value(json_object_get(j_msg, "ResDial"))): 0,
      "res_dial_detail",    json_string_value(json_object_get(j_msg, "ResDialDetail"))? : "",
      "res_hangup",         json_string_value(json_object_get(j_msg, "ResHangup"))? atoi(json_string_value(json_object_get(j_msg, "ResHangup"))): 0,
      "res_hangup_detail",  json_string_value(json_object_get(j_msg, "ResHangupDetail"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_dl_list", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dl list.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlListCreate
 * @param j_msg
 */
static void ami_event_outdllistcreate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdllistcreate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, s:i, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, "
      "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, "
      "s:i, s:s, s:i, s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "dlma_uuid",      json_string_value(json_object_get(j_msg, "DlmaUuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "status",         json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,
      "ukey",           json_string_value(json_object_get(j_msg, "UKey"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "dialing_uuid",       json_string_value(json_object_get(j_msg, "DialingUuid"))? : "",
      "dialing_camp_uuid",  json_string_value(json_object_get(j_msg, "DialingCampUuid"))? : "",
      "dialing_plan_uuid",  json_string_value(json_object_get(j_msg, "DialingPlanUuid"))? : "",

      "number_1",       json_string_value(json_object_get(j_msg, "Number1"))? : "",
      "number_2",       json_string_value(json_object_get(j_msg, "Number2"))? : "",
      "number_3",       json_string_value(json_object_get(j_msg, "Number3"))? : "",
      "number_4",       json_string_value(json_object_get(j_msg, "Number4"))? : "",
      "number_5",       json_string_value(json_object_get(j_msg, "Number5"))? : "",
      "number_6",       json_string_value(json_object_get(j_msg, "Number6"))? : "",
      "number_7",       json_string_value(json_object_get(j_msg, "Number7"))? : "",
      "number_8",       json_string_value(json_object_get(j_msg, "Number8"))? : "",

      "email",       json_string_value(json_object_get(j_msg, "Email"))? : "",

      "trycnt_1",       json_string_value(json_object_get(j_msg, "TryCnt1"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt1"))): 0,
      "trycnt_2",       json_string_value(json_object_get(j_msg, "TryCnt2"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt2"))): 0,
      "trycnt_3",       json_string_value(json_object_get(j_msg, "TryCnt3"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt3"))): 0,
      "trycnt_4",       json_string_value(json_object_get(j_msg, "TryCnt4"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt4"))): 0,
      "trycnt_5",       json_string_value(json_object_get(j_msg, "TryCnt5"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt5"))): 0,
      "trycnt_6",       json_string_value(json_object_get(j_msg, "TryCnt6"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt6"))): 0,
      "trycnt_7",       json_string_value(json_object_get(j_msg, "TryCnt7"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt7"))): 0,
      "trycnt_8",       json_string_value(json_object_get(j_msg, "TryCnt8"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt8"))): 0,

      "res_dial",           json_string_value(json_object_get(j_msg, "ResDial"))? atoi(json_string_value(json_object_get(j_msg, "ResDial"))): 0,
      "res_dial_detail",    json_string_value(json_object_get(j_msg, "ResDialDetail"))? : "",
      "res_hangup",         json_string_value(json_object_get(j_msg, "ResHangup"))? atoi(json_string_value(json_object_get(j_msg, "ResHangup"))): 0,
      "res_hangup_detail",  json_string_value(json_object_get(j_msg, "ResHangupDetail"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_dl_list", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dl list.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlListUpdate
 * @param j_msg
 */
static void ami_event_outdllistupdate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdllistupdate.");

  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, s:i, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, "
      "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, "
      "s:i, s:s, s:i, s:s, "
      "s:s, s:s, s:s"
      "}",

      "uuid",           json_string_value(json_object_get(j_msg, "Uuid"))? : "",
      "dlma_uuid",      json_string_value(json_object_get(j_msg, "DlmaUuid"))? : "",
      "name",           json_string_value(json_object_get(j_msg, "Name"))? : "",
      "detail",         json_string_value(json_object_get(j_msg, "Detail"))? : "",
      "status",         json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,
      "ukey",           json_string_value(json_object_get(j_msg, "UKey"))? : "",
      "variables",      json_string_value(json_object_get(j_msg, "Variable"))? : "",

      "dialing_uuid",       json_string_value(json_object_get(j_msg, "DialingUuid"))? : "",
      "dialing_camp_uuid",  json_string_value(json_object_get(j_msg, "DialingCampUuid"))? : "",
      "dialing_plan_uuid",  json_string_value(json_object_get(j_msg, "DialingPlanUuid"))? : "",

      "number_1",       json_string_value(json_object_get(j_msg, "Number1"))? : "",
      "number_2",       json_string_value(json_object_get(j_msg, "Number2"))? : "",
      "number_3",       json_string_value(json_object_get(j_msg, "Number3"))? : "",
      "number_4",       json_string_value(json_object_get(j_msg, "Number4"))? : "",
      "number_5",       json_string_value(json_object_get(j_msg, "Number5"))? : "",
      "number_6",       json_string_value(json_object_get(j_msg, "Number6"))? : "",
      "number_7",       json_string_value(json_object_get(j_msg, "Number7"))? : "",
      "number_8",       json_string_value(json_object_get(j_msg, "Number8"))? : "",

      "email",       json_string_value(json_object_get(j_msg, "Email"))? : "",

      "trycnt_1",       json_string_value(json_object_get(j_msg, "TryCnt1"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt1"))): 0,
      "trycnt_2",       json_string_value(json_object_get(j_msg, "TryCnt2"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt2"))): 0,
      "trycnt_3",       json_string_value(json_object_get(j_msg, "TryCnt3"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt3"))): 0,
      "trycnt_4",       json_string_value(json_object_get(j_msg, "TryCnt4"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt4"))): 0,
      "trycnt_5",       json_string_value(json_object_get(j_msg, "TryCnt5"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt5"))): 0,
      "trycnt_6",       json_string_value(json_object_get(j_msg, "TryCnt6"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt6"))): 0,
      "trycnt_7",       json_string_value(json_object_get(j_msg, "TryCnt7"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt7"))): 0,
      "trycnt_8",       json_string_value(json_object_get(j_msg, "TryCnt8"))? atoi(json_string_value(json_object_get(j_msg, "TryCnt8"))): 0,

      "res_dial",           json_string_value(json_object_get(j_msg, "ResDial"))? atoi(json_string_value(json_object_get(j_msg, "ResDial"))): 0,
      "res_dial_detail",    json_string_value(json_object_get(j_msg, "ResDialDetail"))? : "",
      "res_hangup",         json_string_value(json_object_get(j_msg, "ResHangup"))? atoi(json_string_value(json_object_get(j_msg, "ResHangup"))): 0,
      "res_hangup_detail",  json_string_value(json_object_get(j_msg, "ResHangupDetail"))? : "",

      "tm_create",  json_string_value(json_object_get(j_msg, "TmCreate"))? : "",
      "tm_update",  json_string_value(json_object_get(j_msg, "TmUpdate"))? : "",
      "tm_delete",  json_string_value(json_object_get(j_msg, "TmDelete"))? : ""
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_insert("ob_dl_list", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dl list.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: OutDlListDelete
 * @param j_msg
 */
static void ami_event_outdllistdelete(json_t* j_msg)
{
  int ret;
  char* sql;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_outdllistdelete.");

  asprintf(&sql, "delete from ob_dl_list where uuid=\"%s\";",
      json_string_value(json_object_get(j_msg, "Uuid"))? : ""
      );
  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete dl list.");
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

