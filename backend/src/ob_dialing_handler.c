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

#include "ob_dialing_handler.h"
#include "ob_event_handler.h"
#include "ob_dl_handler.h"

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

  // dial_exten
  tmp_const = json_string_value(json_object_get(j_dial, "dial_exten"));
  json_object_set_new(j_dialing, "dial_exten", json_string(tmp_const));

  // dial_application
  tmp_const = json_string_value(json_object_get(j_dial, "dial_application"));
  json_object_set_new(j_dialing, "dial_application", json_string(tmp_const));

  // dial_data
  tmp_const = json_string_value(json_object_get(j_dial, "dial_data"));
  json_object_set_new(j_dialing, "dial_data", json_string(tmp_const));


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

  // insert data
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
  ret = db_insert("ob_dialing", j_dialing);
  if(ret == false) {
    slog(LOG_ERR, "Could not create ob_dialing info.");
    json_decref(j_dialing);
    return NULL;
  }

  return j_dialing;
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

bool update_ob_dialing_dialend(const char* uuid, const char* dial_status)
{
  char* timestamp;
  char* sql;
  char* tmp;
  json_t* j_tmp;
  int ret;

  if((uuid == NULL) || (dial_status == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ob_dialing_dialend. uuid[%s], dial_status[%s]", uuid, dial_status);

  // check exist
  ret = is_exist_ob_dialing(uuid);
  if(ret == false) {
    // not ob_dialing call
    return false;
  }

  if(strcasecmp(dial_status, "ANSWER") == 0) {

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
  json_t* j_res;

  if(action_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select * from ob_dialing where action_id=\"%s\";", action_id);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dialing record info.");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get correct dialing info by action id. action[%s]", action_id);
    return NULL;
  }

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

