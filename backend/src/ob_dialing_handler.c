/*
 * dialing_handler.c
 *
 *  Created on: Nov 21, 2015
 *    Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "db_handler.h"
#include "ast_header.h"
#include "ami_handler.h"

#include "ob_dialing_handler.h"
#include "ob_event_handler.h"
#include "ob_dl_handler.h"

extern app* g_app;


#define DEF_MIN_DIALING_UPDATE_TIMEOUT  20  // minimum dialing tm_update timeout

typedef struct _map_res_dial {
  int res_dial;
  const char* res_dial_detail;
} map_res_dial;

map_res_dial g_map_res_dial[] =
{
  (map_res_dial){.res_dial = AST_CONTROL_HANGUP,        .res_dial_detail = "Other end has hungup"},
  (map_res_dial){.res_dial = AST_CONTROL_RING,          .res_dial_detail = "Local ring"},
  (map_res_dial){.res_dial = AST_CONTROL_RINGING,       .res_dial_detail = "Remote end is ringing"},
  (map_res_dial){.res_dial = AST_CONTROL_ANSWER,        .res_dial_detail = "Remote end has answered"},
  (map_res_dial){.res_dial = AST_CONTROL_BUSY,          .res_dial_detail = "Remote end is busy"},

  (map_res_dial){.res_dial = AST_CONTROL_TAKEOFFHOOK,       .res_dial_detail = "Make it go off hook"},
  (map_res_dial){.res_dial = AST_CONTROL_OFFHOOK,           .res_dial_detail = "Line is off hook"},
  (map_res_dial){.res_dial = AST_CONTROL_CONGESTION,        .res_dial_detail = "Congestion (circuits busy)"},
  (map_res_dial){.res_dial = AST_CONTROL_FLASH,             .res_dial_detail = "Flash hook"},
  (map_res_dial){.res_dial = AST_CONTROL_WINK,              .res_dial_detail = "Wink"},
  (map_res_dial){.res_dial = AST_CONTROL_OPTION,            .res_dial_detail = "Set a low-level option"},
  (map_res_dial){.res_dial = AST_CONTROL_RADIO_KEY,         .res_dial_detail = "Key Radio"},
  (map_res_dial){.res_dial = AST_CONTROL_RADIO_UNKEY,       .res_dial_detail = "Un-Key Radio"},
  (map_res_dial){.res_dial = AST_CONTROL_PROGRESS,          .res_dial_detail = "Indicate PROGRESS"},
  (map_res_dial){.res_dial = AST_CONTROL_PROCEEDING,        .res_dial_detail = "Indicate CALL PROCEEDING"},
  (map_res_dial){.res_dial = AST_CONTROL_HOLD,              .res_dial_detail = "Indicate call is placed on hold"},
  (map_res_dial){.res_dial = AST_CONTROL_UNHOLD,            .res_dial_detail = "Indicate call is left from hold"},
  (map_res_dial){.res_dial = AST_CONTROL_VIDUPDATE,         .res_dial_detail = "Indicate video frame update"},
  (map_res_dial){.res_dial = _XXX_AST_CONTROL_T38,          .res_dial_detail = "T38 state change request/notification. This is no longer supported. Use AST_CONTROL_T38_PARAMETERS instead"},
  (map_res_dial){.res_dial = AST_CONTROL_SRCUPDATE,         .res_dial_detail = "Indicate source of media has changed"},
  (map_res_dial){.res_dial = AST_CONTROL_TRANSFER,          .res_dial_detail = "Indicate status of a transfer request"},
  (map_res_dial){.res_dial = AST_CONTROL_CONNECTED_LINE,    .res_dial_detail = "Indicate connected line has changed"},
  (map_res_dial){.res_dial = AST_CONTROL_REDIRECTING,       .res_dial_detail = "Indicate redirecting id has changed"},
  (map_res_dial){.res_dial = AST_CONTROL_T38_PARAMETERS,    .res_dial_detail = "T38 state change request/notification with parameters"},
  (map_res_dial){.res_dial = AST_CONTROL_CC,                .res_dial_detail = "Indication that Call completion service is possible"},
  (map_res_dial){.res_dial = AST_CONTROL_SRCCHANGE,         .res_dial_detail = "Media source has changed and requires a new RTP SSRC"},
  (map_res_dial){.res_dial = AST_CONTROL_READ_ACTION,       .res_dial_detail = "Tell ast_read to take a specific action"},
  (map_res_dial){.res_dial = AST_CONTROL_AOC,               .res_dial_detail = "Advice of Charge with encoded generic AOC payload"},
  (map_res_dial){.res_dial = AST_CONTROL_END_OF_Q,          .res_dial_detail = "Indicate that this position was the end of the channel queue for a softhangup."},
  (map_res_dial){.res_dial = AST_CONTROL_INCOMPLETE,        .res_dial_detail = "Indication that the extension dialed is incomplete"},
  (map_res_dial){.res_dial = AST_CONTROL_MCID,              .res_dial_detail = "Indicate that the caller is being malicious"},
  (map_res_dial){.res_dial = AST_CONTROL_UPDATE_RTP_PEER,   .res_dial_detail = "Interrupt the bridge and have it update the peer"},
  (map_res_dial){.res_dial = AST_CONTROL_PVT_CAUSE_CODE,    .res_dial_detail = "Contains an update to the protocol-specific cause-code stored for branching dials"},
  (map_res_dial){.res_dial = AST_CONTROL_MASQUERADE_NOTIFY, .res_dial_detail = "A masquerade is about to begin/end. (Never sent as a frame but directly with ast_indicate_data().)"},

  /*
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   *
   * IAX2 sends these values out over the wire.  To prevent future
   * incompatibilities, pick the next value in the enum from whatever
   * is on the current trunk.  If you lose the merge race you need to
   * fix the previous branches to match what is on trunk.  In addition
   * you need to change chan_iax2 to explicitly allow the control
   * frame over the wire if it makes sense for the frame to be passed
   * to another Asterisk instance.
   *
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   */

  /* Control frames used to manipulate a stream on a channel. The values for these
   * must be greater than the allowed value for a 8-bit char, so that they avoid
   * conflicts with DTMF values. */
  (map_res_dial){.res_dial = AST_CONTROL_STREAM_STOP,       .res_dial_detail = "Indicate to a channel in playback to stop the stream"},
  (map_res_dial){.res_dial = AST_CONTROL_STREAM_SUSPEND,    .res_dial_detail = "Indicate to a channel in playback to suspend the stream"},
  (map_res_dial){.res_dial = AST_CONTROL_STREAM_RESTART,    .res_dial_detail = "Indicate to a channel in playback to restart the stream"},
  (map_res_dial){.res_dial = AST_CONTROL_STREAM_REVERSE,    .res_dial_detail = "Indicate to a channel in playback to rewind"},
  (map_res_dial){.res_dial = AST_CONTROL_STREAM_FORWARD,    .res_dial_detail = "Indicate to a channel in playback to fast forward"},

  (map_res_dial){.res_dial = AST_CONTROL_RECORD_CANCEL,     .res_dial_detail = "Indicated to a channel in record to stop recording and discard the file"},
  (map_res_dial){.res_dial = AST_CONTROL_RECORD_STOP,       .res_dial_detail = "Indicated to a channel in record to stop recording"},
  (map_res_dial){.res_dial = AST_CONTROL_RECORD_SUSPEND,    .res_dial_detail = "Indicated to a channel in record to suspend/unsuspend recording"},
  (map_res_dial){.res_dial = AST_CONTROL_RECORD_MUTE,       .res_dial_detail = "Indicated to a channel in record to mute/unmute (i.e. write silence) recording"},
};


static const char* get_res_dial_detail_string(int res_dial);

/**
 * Create dialing obj.
 * @param j_camp
 * @param j_plan
 * @param j_dl
 * @return
 */
json_t* create_ob_dialing(
    const char* dialing_uuid,
    json_t* j_camp,
    json_t* j_plan,
    json_t* j_dlma,
    json_t* j_dest,
    json_t* j_dl_list,
    json_t* j_dial
    )
{
  json_t* j_dialing;
  char* tmp;
  const char* tmp_const;
  int ret;

  if((dialing_uuid == NULL)
      || (j_camp == NULL)
      || (j_plan == NULL)
      || (j_dlma == NULL)
      || (j_dest == NULL)
      || (j_dl_list == NULL)
      || (j_dial == NULL)
      ) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired create_ob_dialing. uuid[%s]", dialing_uuid);

  j_dialing = json_object();

  ///// identity
  // uuid
  json_object_set_new(j_dialing, "uuid", json_string(dialing_uuid));

  // status
  json_object_set_new(j_dialing, "status", json_integer(E_DIALING_NONE));

  // action
  tmp = gen_uuid();
  json_object_set_new(j_dialing, "action_id", json_string(tmp));
  sfree(tmp);

  ///// dial info
  // dial_channel
  tmp_const = json_string_value(json_object_get(j_dial, "dial_channel"));
  json_object_set_new(j_dialing, "dial_channel", json_string(tmp_const));

  // dial_channel
  tmp_const = json_string_value(json_object_get(j_dial, "dial_addr"));
  json_object_set_new(j_dialing, "dial_addr", json_string(tmp_const));

  // dial_index
  ret = json_integer_value(json_object_get(j_dial, "dial_index"));
  json_object_set_new(j_dialing, "dial_index", json_integer(ret));

  // dial_trycnt
  ret = json_integer_value(json_object_get(j_dial, "dial_trycnt"));
  json_object_set_new(j_dialing, "dial_trycnt", json_integer(ret));

  // dial_timeout
  ret = json_integer_value(json_object_get(j_dial, "dial_timeout"));
  json_object_set_new(j_dialing, "dial_timeout", json_integer(ret));

  // dial_type
  ret = json_integer_value(json_object_get(j_dial, "dial_type"));
  json_object_set_new(j_dialing, "dial_type", json_integer(ret));

  // dial_context
  tmp_const = json_string_value(json_object_get(j_dial, "dial_context"));
  json_object_set_new(j_dialing, "dial_context", json_string(tmp_const));

  // dial_exten
  tmp_const = json_string_value(json_object_get(j_dial, "dial_exten"));
  json_object_set_new(j_dialing, "dial_exten", json_string(tmp_const));

  // dial_priority
  tmp_const = json_string_value(json_object_get(j_dial, "dial_priority"));
  json_object_set_new(j_dialing, "dial_priority", json_string(tmp_const));

  // dial_application
  tmp_const = json_string_value(json_object_get(j_dial, "dial_application"));
  json_object_set_new(j_dialing, "dial_application", json_string(tmp_const));

  // dial_data
  tmp_const = json_string_value(json_object_get(j_dial, "dial_data"));
  json_object_set_new(j_dialing, "dial_data", json_string(tmp_const));

  // dial_variables
  json_object_set_new(j_dialing, "dial_variables", json_object());
  json_object_update(json_object_get(j_dialing, "dial_variables"), json_object_get(j_camp, "variables"));
  json_object_update(json_object_get(j_dialing, "dial_variables"), json_object_get(j_plan, "variables"));
  json_object_update(json_object_get(j_dialing, "dial_variables"), json_object_get(j_dest, "variables"));
  json_object_update(json_object_get(j_dialing, "dial_variables"), json_object_get(j_dlma, "variables"));
  json_object_update(json_object_get(j_dialing, "dial_variables"), json_object_get(j_dl_list, "variables"));

  ///// uuid info
  // uuid_camp
  tmp_const = json_string_value(json_object_get(j_camp, "uuid"));
  json_object_set_new(j_dialing, "uuid_camp", json_string(tmp_const));

  // uuid_plan
  tmp_const = json_string_value(json_object_get(j_plan, "uuid"));
  json_object_set_new(j_dialing, "uuid_plan", json_string(tmp_const));

  // uuid_dlma
  tmp_const = json_string_value(json_object_get(j_dlma, "uuid"));
  json_object_set_new(j_dialing, "uuid_dlma", json_string(tmp_const));

  // uuid_dest
  tmp_const = json_string_value(json_object_get(j_dest, "uuid"));
  json_object_set_new(j_dialing, "uuid_dest", json_string(tmp_const));

  // uuid_dl_list
  tmp_const = json_string_value(json_object_get(j_dl_list, "uuid"));
  json_object_set_new(j_dialing, "uuid_dl_list", json_string(tmp_const));


  ///// referenced info
  // info_camp
  tmp = json_dumps(j_camp, JSON_ENCODE_ANY);
  json_object_set_new(j_dialing, "info_camp", json_string(tmp));
  sfree(tmp);

  // info_plan
  tmp = json_dumps(j_plan, JSON_ENCODE_ANY);
  json_object_set_new(j_dialing, "info_plan", json_string(tmp));
  sfree(tmp);

  // info_dlma
  tmp = json_dumps(j_dlma, JSON_ENCODE_ANY);
  json_object_set_new(j_dialing, "info_dlma", json_string(tmp));
  sfree(tmp);

  // info_dest
  tmp = json_dumps(j_dest, JSON_ENCODE_ANY);
  json_object_set_new(j_dialing, "info_dest", json_string(tmp));
  sfree(tmp);

  // info_dl_list
  tmp = json_dumps(j_dl_list, JSON_ENCODE_ANY);
  json_object_set_new(j_dialing, "info_dl_list", json_string(tmp));
  sfree(tmp);

  // info_dial
  tmp = json_dumps(j_dial, JSON_ENCODE_ANY);
  json_object_set_new(j_dialing, "info_dial", json_string(tmp));
  sfree(tmp);


  //// timestamp
  // tm_create
  tmp = get_utc_timestamp();
  json_object_set_new(j_dialing, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_DEBUG, "Check value. dial_channel[%s], dial_addr[%s], dial_index[%lld], dial_trycnt[%lld], dial_timeout[%lld], dial_type[%lld], dial_exten[%s], dial_application[%s]",
      json_string_value(json_object_get(j_dial, "dial_channel"))? : "",
      json_string_value(json_object_get(j_dial, "dial_addr"))? : "",
      json_integer_value(json_object_get(j_dial, "dial_index")),
      json_integer_value(json_object_get(j_dial, "dial_trycnt")),
      json_integer_value(json_object_get(j_dial, "dial_timeout")),
      json_integer_value(json_object_get(j_dial, "dial_type")),
      json_string_value(json_object_get(j_dial, "dial_exten"))? : "",
      json_string_value(json_object_get(j_dial, "dial_application"))? : ""
      );

  return j_dialing;
}

bool update_ob_dialing_timestamp(const char* uuid)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;
  char* tmp;
  char* sql;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    // non exist
    // already deleted dialing or wrong dialing uuid
    return false;
  }

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{s:s}",
      "tm_update",  timestamp
      );
  sfree(timestamp);

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not create update sql.");
    return false;
  }
  asprintf(&sql, "update ob_dialing set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not update ob_dialing status. uuid[%s]", uuid);
    return false;
  }

  return true;
}

/**
 * Delete ob_dialing record.
 * @param uuid
 * @return
 */
bool delete_ob_dialing(const char* uuid)
{
  char* sql;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  asprintf(&sql, "delete from ob_dialing where uuid=\"%s\";", uuid);
  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete ob_dialing info. uuid[%s]", uuid);
    return false;
  }

  return true;
}

bool insert_ob_dialing(json_t* j_dialing)
{
  int ret;

  if(j_dialing == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert data
  ret = db_insert("ob_dialing", j_dialing);
  if(ret == false) {
    slog(LOG_ERR, "Could not create ob_dialing info.");
    return false;
  }

  return true;
}

void rb_dialing_destory(rb_dialing* dialing)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
//  ast_mutex_lock(&g_rb_dialing_mutex);
//
//  slog(LOG_DEBUG, "Destroying dialing.");
//  ao2_unlink(g_rb_dialings, dialing);
//  ao2_ref(dialing, -1);
//
//  ast_mutex_unlock(&g_rb_dialing_mutex);

  return;
}

//static void rb_dialing_destructor(void* obj)
//{
//  rb_dialing* dialing;
//
//  dialing = (rb_dialing*)obj;
//
////  // send destroy
////  send_manager_evt_out_dialing_delete(dialing);
//
//  if(dialing->uuid != NULL)       sfree(dialing->uuid);
//  if(dialing->name != NULL)       sfree(dialing->name);
//  if(dialing->j_dialing != NULL)    json_decref(dialing->j_dialing);
//  if(dialing->j_event != NULL)    json_decref(dialing->j_event);
//  if(dialing->j_events != NULL)    json_decref(dialing->j_events);
//  if(dialing->tm_create != NULL)  sfree(dialing->tm_create);
//  if(dialing->tm_update != NULL)  sfree(dialing->tm_update);
//  if(dialing->tm_delete != NULL)  sfree(dialing->tm_delete);
//
//  slog(LOG_DEBUG, "Called destroyer.");
//}

///**
// * There's no mutex lock here.
// * locking is caller's responsibility.
// * @param dialing
// * @return
// */
//static bool rb_dialing_update(rb_dialing* dialing)
//{
//  if(dialing == NULL) {
//    return false;
//  }
//
//  if(dialing->tm_update != NULL) {
//    sfree(dialing->tm_update);
//  }
//  dialing->tm_update = get_utc_timestamp();
//
////  clock_gettime(CLOCK_REALTIME, &dialing->timeptr_update);
////
////  send_manager_evt_out_dialing_update(dialing);
//
//  return true;
//}

bool rb_dialing_update_name(rb_dialing* dialing, const char* name)
{
  // todo
//  slog(LOG_ERR, "Need to fix.");
  return false;
//  if((dialing == NULL) || (name == NULL)) {
//    return false;
//  }
//
//  ast_mutex_lock(&g_rb_dialing_mutex);
//
//  dialing->name = strdup(name);
//
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//
//  return true;
}

bool rb_dialing_update_events_append(rb_dialing* dialing, json_t* j_evt)
{
  // todo
//  slog(LOG_ERR, "Need to fix.");
  return false;
//  const char* tmp_const;
//  int ret;
//
//  if((dialing == NULL) || (j_evt == NULL)) {
//    return false;
//  }
//
//  // debug only.
//  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "history_events_enable"));
//  if(tmp_const == NULL) {
//    return true;
//  }
//
//  ret = atoi(tmp_const);
//  if(ret != 1) {
//    return true;
//  }
//
//  ast_mutex_lock(&g_rb_dialing_mutex);
//
//  json_array_append_new(dialing->j_events, json_incref(j_evt));
//
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//
//  return true;
}


/**
 * Update dialing res
 * @param dialing
 * @param j_res
 * @return
 */
bool rb_dialing_update_dialing_update(rb_dialing* dialing, json_t* j_res)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
  return false;

//  int ret;
//
//  if((dialing == NULL) || (j_res == NULL)) {
//    return false;
//  }
//
//  ast_mutex_lock(&g_rb_dialing_mutex);
//
//  json_object_update(dialing->j_dialing, j_res);
//
//  // send update event AMI
//  ret = rb_dialing_update(dialing);
//
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//  if(ret != true) {
//    return false;
//  }
//
//  return true;
}

/**
 * Update dialing current info.
 * @param dialing
 * @param j_res
 * @return
 */
bool rb_dialing_update_current_update(rb_dialing* dialing, json_t* j_evt)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
  return false;

//  if((dialing == NULL) || (j_evt == NULL)) {
//    return false;
//  }
//
//  ast_mutex_lock(&g_rb_dialing_mutex);
//
//  json_object_update(dialing->j_event, j_evt);
//
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//
//  return true;
}

/**
 * Update dialing current info.
 * @param dialing
 * @param j_res
 * @return
 */
bool rb_dialing_update_event_substitute(rb_dialing* dialing, json_t* j_evt)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
  return false;

//  if((dialing == NULL) || (j_evt == NULL)) {
//    return false;
//  }
//
//  ast_mutex_lock(&g_rb_dialing_mutex);
//
//  if(dialing->j_event != NULL) {
//    json_decref(dialing->j_event);
//  }
//
//  dialing->j_event = json_deep_copy(j_evt);
//
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//
//  return true;
}

bool update_ob_dialing_status(const char* uuid, E_DIALING_STATUS_T status)
{
  json_t* j_tmp;
  char* timestamp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check ob_dialing exists
  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    // not ob_dialing call
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ob_dialing_status. dialing_uuid[%s], status[%d]", uuid, status);

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{s:i, s:s}",
      "status",     status,
      "tm_update",  timestamp
      );
  sfree(timestamp);

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not create update sql.");
    return false;
  }
  asprintf(&sql, "update ob_dialing set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not update ob_dialing status. uuid[%s]", uuid);
    return false;
  }

  return true;
}

bool update_ob_dialing_hangup(const char* uuid, int hangup, const char* hangup_detail)
{
  char* timestamp;
  char* sql;
  char* tmp;
  json_t* j_tmp;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ob_dialing_hangup. uuid[%s], hangup[%d], hangup_detail[%s]",
      uuid, hangup, hangup_detail? : ""
      );

  // check exist
  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    // not ob_dialing call
    return false;
  }

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{s:i, s:i, s:s, s:s}",
      "status",             E_DIALING_HANGUP,
      "res_hangup",         hangup,
      "res_hangup_detail",  hangup_detail? : "",
      "tm_update",          timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create update ob_dialing info.");
    return false;
  }

  // create update sql
  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not create update sql.");
    return false;
  }
  asprintf(&sql, "update ob_dialing set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not update ob_dialing status. uuid[%s]", uuid);
    return false;
  }

  return true;
}

bool update_ob_dialing_res_dial(const char* uuid, bool success, int res_dial, const char* channel)
{
  char* timestamp;
  char* sql;
  char* tmp;
  json_t* j_tmp;
  int ret;
  const char* res_dial_detail;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ob_dialing_res_dial. uuid[%s], success[%d], reason[%d]",
      uuid, success, res_dial);

  // check exist
  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    // not ob_dialing call
    return false;
  }

  // create update info
  timestamp = get_utc_timestamp();
  res_dial_detail = get_res_dial_detail_string(res_dial);
  j_tmp = json_pack("{s:i, s:s, s:s, s:s}",
      "res_dial",         res_dial,
      "res_dial_detail",  res_dial_detail? : "",
      "channel",          channel? : "",
      "tm_update",        timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create update ob_dialing info.");
    return false;
  }

  // create update sql
  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not create update sql.");
    return false;
  }
  asprintf(&sql, "update ob_dialing set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  // update result
  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not update ob_dialing originate response info.");
    return false;
  }

  // update status
  if(success == true) {
    ret = update_ob_dialing_status(uuid, E_DIALING_ORIGINATE_RESPONSED);
  }
  else {
    ret = update_ob_dialing_status(uuid, E_DIALING_ERROR_ORIGINATE_RESPONSE_FAILED);
  }
  if(ret == false) {
    slog(LOG_ERR, "Could not update ob_dialing status info.");
    return false;
  }

  return true;
}

//bool update_ob_dialing_dialend(const char* uuid, const char* dial_status)
//{
//  char* timestamp;
//  char* sql;
//  char* tmp;
//  json_t* j_tmp;
//  int ret;
//
//  if((uuid == NULL) || (dial_status == NULL)) {
//    slog(LOG_WARNING, "Wrong input parameter.");
//    return false;
//  }
//  slog(LOG_DEBUG, "Fired update_ob_dialing_dialend. uuid[%s], dial_status[%s]", uuid, dial_status);
//
//  // check exist
//  ret = is_exist_ob_dialing(uuid);
//  if(ret == false) {
//    // not ob_dialing call
//    return false;
//  }
//
//  if(strcasecmp(dial_status, "ANSWER") == 0) {
//
//  }
//
//  timestamp = get_utc_timestamp();
//  j_tmp = json_pack("{s:i, s:i, s:s, s:s}",
//      "status",             E_DIALING_HANGUP,
//      "res_hangup",         hangup,
//      "res_hangup_detail",  hangup_detail? : "",
//      "tm_update",          timestamp
//      );
//  sfree(timestamp);
//  if(j_tmp == NULL) {
//    slog(LOG_ERR, "Could not create update ob_dialing info.");
//    return false;
//  }
//
//  // create update sql
//  tmp = db_get_update_str(j_tmp);
//  json_decref(j_tmp);
//  if(tmp == NULL) {
//    slog(LOG_ERR, "Could not create update sql.");
//    return false;
//  }
//  asprintf(&sql, "update ob_dialing set %s where uuid=\"%s\";", tmp, uuid);
//  sfree(tmp);
//
//  ret = db_exec(sql);
//  sfree(sql);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not update ob_dialing status. uuid[%s]", uuid);
//    return false;
//  }
//
//  return true;
//}

rb_dialing* rb_dialing_find_chan_name(const char* name)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
  return NULL;

//  rb_dialing* dialing;
//
//  if(name == NULL) {
//    return NULL;
//  }
//
//  slog(LOG_DEBUG, "rb_dialing_find_chan_name. name[%s]", name);
//  ast_mutex_lock(&g_rb_dialing_mutex);
//  dialing = ao2_find(g_rb_dialings, name, OBJ_SEARCH_PARTIAL_KEY);
//  if(dialing == NULL) {
//    ast_mutex_unlock(&g_rb_dialing_mutex);
//    return NULL;
//  }
//  ao2_ref(dialing, -1);
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//
//  return dialing;
}

rb_dialing* rb_dialing_find_chan_uuid(const char* uuid)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
  return NULL;

//  rb_dialing* dialing;
//
//  if(uuid == NULL) {
//    return NULL;
//  }
//
//  slog(LOG_DEBUG, "rb_dialing_find_chan_uuid. uuid[%s]", uuid);
//  ast_mutex_lock(&g_rb_dialing_mutex);
//  dialing = ao2_find(g_rb_dialings, uuid, OBJ_SEARCH_KEY);
//  if(dialing == NULL) {
//    ast_mutex_unlock(&g_rb_dialing_mutex);
//    return NULL;
//  }
//  ao2_ref(dialing, -1);
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//
//  return dialing;
}

/**
 * Return the count of the dialings of the campaign.
 * @param camp_uuid
 * @return
 */
int get_ob_dialing_count_by_camp_uuid(const char* camp_uuid)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  int ret;

  if(camp_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired get_ob_dialing_count_by_camp_uuid.");

  // sql
  asprintf(&sql, "select count(*) from ob_dialing where uuid_camp = \"%s\";", camp_uuid);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct result.");
    return -1;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  ret = json_integer_value(json_object_get(j_res, "count(*)"));
  json_decref(j_res);

  return ret;
}

/**
 * Get dialing info using action_id.
 * @param action_id
 * @return
 */
json_t* get_ob_dialing_by_action_id(const char* action_id)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;
  json_t* j_res;
  const char* uuid;

  if(action_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select uuid from ob_dialing where action_id=\"%s\";", action_id);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dialing record info.");
    return NULL;
  }

  j_tmp = db_get_record(db_res);
  db_free(db_res);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get correct dialing info by action id. action_id[%s]", action_id);
    return NULL;
  }

  uuid = json_string_value(json_object_get(j_tmp, "uuid"));
  if(uuid == NULL) {
    json_decref(j_tmp);
    slog(LOG_ERR, "Could not get correct dialing uuid by action id. action_id[%s]", action_id);
    return NULL;
  }

  j_res = get_ob_dialing(uuid);
  json_decref(j_tmp);

  return j_res;
}

/**
 * Get dialing info using uuid.
 * @param action_id
 * @return
 */
json_t* get_ob_dialing(const char* uuid)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ob_dialing. uuid[%s]", uuid);

  asprintf(&sql, "select * from ob_dialing where uuid=\"%s\";", uuid);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dialing record info.");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get correct dialing info by uuid. uuid[%s]", uuid);
    return NULL;
  }

  return j_res;
}

json_t* get_ob_dialings_uuid_timeout(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_tmp;
  int timeout;
  const char* uuid;

  timeout = json_integer_value(json_object_get(json_object_get(g_app->j_conf, "ob"), "dialing_timeout"));
  if(timeout < DEF_MIN_DIALING_UPDATE_TIMEOUT) {
    timeout = DEF_MIN_DIALING_UPDATE_TIMEOUT;
  }

  asprintf(&sql, "select uuid from ob_dialing where"
      " (strftime('%%s', tm_update) + 0) < (strftime('%%s', 'now') - %d);",
      timeout
      );

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dialing record info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    uuid = json_string_value(json_object_get(j_tmp, "uuid"));
    if(uuid == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res, json_string(uuid));
    json_decref(j_tmp);
  }
  db_free(db_res);

  return j_res;
}

/**
 * Get timeout dialings info.
 * @param action_id
 * @return
 */
json_t* get_ob_dialings_timeout(void)
{
  json_t* j_uuids;
  unsigned int idx;
  json_t* j_val;
  json_t* j_res;
  json_t* j_tmp;
  const char* uuid;

  j_uuids = get_ob_dialings_uuid_timeout();

  j_res = json_array();
  json_array_foreach(j_uuids, idx, j_val) {

    uuid = json_string_value(j_val);
    if(uuid == NULL) {
      continue;
    }

    j_tmp = get_ob_dialing(uuid);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_uuids);

  return j_res;
}

static json_t* get_ob_dalings_uuid_error(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_tmp;
  const char* uuid;

  asprintf(&sql, "select uuid from ob_dialing where status > %d;", E_DIALING_ERROR_UNKNOWN);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dialing record info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    uuid = json_string_value(json_object_get(j_tmp, "uuid"));
    if(uuid == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res, json_string(uuid));
    json_decref(j_tmp);
  }
  db_free(db_res);

  return j_res;
}

/**
 * Get timeout dialings info.
 * @param action_id
 * @return
 */
json_t* get_ob_dialings_error(void)
{
  json_t* j_uuids;
  json_t* j_val;
  json_t* j_res;
  json_t* j_tmp;
  unsigned int idx;
  const char* uuid;

  j_uuids = get_ob_dalings_uuid_error();
  if(j_uuids == NULL) {
    return NULL;
  }

  j_res = json_array();
  json_array_foreach(j_uuids, idx, j_val) {
    uuid = json_string_value(j_val);
    if(uuid == NULL) {
      continue;
    }

    j_tmp = get_ob_dialing(uuid);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_uuids);

  return j_res;
}

static json_t* get_ob_dialings_uuid_by_status(E_DIALING_STATUS_T status)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_tmp;
  const char* uuid;

  asprintf(&sql, "select uuid from ob_dialing where status=%d;", status);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dialing record info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    uuid = json_string_value(json_object_get(j_tmp, "uuid"));
    if(uuid == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res, json_string(uuid));
    json_decref(j_tmp);
  }
  db_free(db_res);

  return j_res;
}

/**
 * Get hungup dialings info.
 * @param action_id
 * @return
 */
json_t* get_ob_dialings_hangup(void)
{
  json_t* j_uuids;
  json_t* j_uuid;
  json_t* j_res;
  json_t* j_tmp;
  const char* uuid;
  unsigned int idx;

  j_uuids = get_ob_dialings_uuid_by_status(E_DIALING_HANGUP);
  if(j_uuids == NULL) {
    return NULL;
  }

  j_res = json_array();
  json_array_foreach(j_uuids, idx, j_uuid) {
    uuid = json_string_value(j_uuid);
    if(uuid == NULL) {
      continue;
    }

    j_tmp = get_ob_dialing(uuid);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_uuids);

  return j_res;
}

/**
 * Get all dialings info.
 * @param action_id
 * @return
 */
json_t* get_ob_dialings_all(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_tmp;

  slog(LOG_DEBUG, "Fired get_ob_dialings_all.");

  asprintf(&sql, "select * from ob_dialing;");

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dialing record info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }
    json_array_append_new(j_res, j_tmp);
  }
  db_free(db_res);

  return j_res;
}


/**
 * Return existence of given ob_dialing uuid.
 * @param uuid
 * @return
 */
bool is_exist_ob_dialing(const char* uuid)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  asprintf(&sql, "select count(*) from ob_dialing where uuid=\"%s\";", uuid);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get ob_dialing info. uuid[%s]", uuid);
    return false;
  }

  j_tmp = db_get_record(db_res);
  db_free(db_res);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get correct ob_plan record. uuid[%s]", uuid);
    return false;
  }

  // result
  ret = json_integer_value(json_object_get(j_tmp, "count(*)"));
  json_decref(j_tmp);
  if(ret <= 0) {
    return false;
  }

  return true;
}

static const char* get_res_dial_detail_string(int res_dial)
{
  int size;
  int i;
  int res;

  // get size
  size = sizeof(g_map_res_dial) / sizeof(map_res_dial);

  for(i = 0; i < size; i++) {
    res = g_map_res_dial[i].res_dial;
    if(res == res_dial) {
      return g_map_res_dial[i].res_dial_detail;
    }
  }

  return NULL;
}

/**
 * Returns list of all dialing uuid.
 * @return
 */
json_t* get_ob_dialings_uuid_all(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select uuid from ob_dialing;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct ob_dialing.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s}",
        "uuid", json_string_value(json_object_get(j_tmp, "uuid"))
        );
    json_decref(j_tmp);
    if(j_res_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_res_tmp);
  }
  db_free(db_res);

  return j_res;
}

/**
 * Send hangup request of ob dialing.
 * @param uuid
 * @return
 */
bool send_ob_dialing_hangup_request(const char* uuid)
{
  json_t* j_tmp;
  json_t* j_dialing;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check dialing existence
  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    slog(LOG_ERR, "The given dialing uuid for hangup is not exist. uuid[%s]", uuid);
    return false;
  }

  // get ob dialing
  j_dialing = get_ob_dialing(uuid);
  if(j_dialing == NULL) {
    slog(LOG_ERR, "Could not get dialing info for hangup. uuid[%s]", uuid);
    return false;
  }

  // create hangup request
  j_tmp = json_pack("{s:s, s:s}",
      "Action",   "Hangup",
      "Channel",  json_string_value(json_object_get(j_dialing, "channel"))? : ""
      );
  json_decref(j_dialing);

  // send hangup request
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action for dialing hangup. action[%s]", "Hangup");
    return false;
  }

  return true;
}
