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

//AST_MUTEX_DEFINE_STATIC(g_rb_dialing_mutex);

//static int rb_dialing_cmp_cb(void* obj, void* arg, int flags);
//static int rb_dialing_sort_cb(const void* o_left, const void* o_right, int flags);
//static void rb_dialing_destructor(void* obj);
//static bool rb_dialing_update(rb_dialing* dialing);

//static struct ao2_container* g_rb_dialings = NULL;  ///< dialing container

/**
 * Initiate rb_diailing.
 * @return
 */
int init_rb_dialing(void)
{
  slog(LOG_ERR, "Need to fix.");
//  g_rb_dialings = ao2_container_alloc_rbtree(AO2_ALLOC_OPT_LOCK_MUTEX, AO2_CONTAINER_ALLOC_OPT_DUPS_REJECT, rb_dialing_sort_cb, rb_dialing_cmp_cb);
//  if(g_rb_dialings == NULL) {
//    slog(LOG_ERROR, "Could not create rbtree.\n");
//    return false;
//  }
//   slog(LOG_NOTICE, "Initiated dialing handler.\n");
//
  return true;
}

///**
// *
// * @param o_left
// * @param o_right
// * @param flags
// * @return
// */
//static int rb_dialing_sort_cb(const void* o_left, const void* o_right, int flags)
//{
//  const rb_dialing* dialing_left;
//  const char* key;
//
//  dialing_left = (rb_dialing*)o_left;
//
//  if(flags & OBJ_SEARCH_KEY) {
//    key = (char*)o_right;
//
//    return strcmp(dialing_left->uuid, key);
//  }
//  else if(flags & OBJ_SEARCH_PARTIAL_KEY) {
//    key = (char*)o_right;
//
//    if((dialing_left->name == NULL) || (key == NULL)) {
//      return 0;
//    }
//    return strcmp(dialing_left->name, key);
//  }
//  else {
//    const rb_dialing* dialing_right;
//
//    dialing_right = (rb_dialing*)o_right;
//    return strcmp(dialing_left->uuid, dialing_right->uuid);
//  }
//}

///**
// *
// * @param obj
// * @param arg
// * @param flags
// * @return
// */
//static int rb_dialing_cmp_cb(void* obj, void* arg, int flags)
//{
//  slog(LOG_ERR, "Need to fix.");
//  return false;
//
////  rb_dialing* dialing;
////  const char *key;
////
////  dialing = (rb_dialing*)obj;
////
////  if(flags & OBJ_SEARCH_KEY) {
////    key = (const char*)arg;
////
////    // channel unique id
////    if(dialing->uuid == NULL) {
////      return 0;
////    }
////    if(strcmp(dialing->uuid, key) == 0) {
////      return CMP_MATCH;
////    }
////    return 0;
////  }
////  else if(flags & OBJ_SEARCH_PARTIAL_KEY) {
////    key = (const char*)arg;
////
////    // channel name
////    if((dialing->name == NULL) || (key == NULL)) {
////      return 0;
////    }
////
////    if(strcmp(dialing->name, key) == 0) {
////      return CMP_MATCH;
////    }
////    return 0;
////  }
////  else {
////    // channel id
////    rb_dialing* dialing_right;
////
////    dialing_right = (rb_dialing*)arg;
////    if(strcmp(dialing->uuid, dialing_right->uuid) == 0) {
////      return CMP_MATCH;
////    }
////    return 0;
////  }
//}

/**
 * Create dialing obj.
 * @param j_camp
 * @param j_plan
 * @param j_dl
 * @return
 */
json_t* rb_dialing_create(
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

  // uuid
  json_object_set_new(j_dialing, "uuid", json_string(dialing_uuid));

  // status
  json_object_set_new(j_dialing, "status", json_integer(E_DIALING_NONE));


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


  ///// dial info
  // dial_channel
  tmp_const = json_string_value(json_object_get(j_dial, "dial_channel"));
  json_object_set_new(j_dialing, "dial_channel", json_string(tmp_const));

  // dial_channel
  tmp_const = json_string_value(json_object_get(j_dial, "dial_addr"));
  json_object_set_new(j_dialing, "dial_addr", json_string(tmp_const));

  // dial_index
  ret = json_integer_value(json_object_get(j_dial, "dial_index"));
  json_object_set_new(j_dialing, "dial_addr", json_integer(ret));

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


  //// timestamp
  // tm_create
  tmp = get_utc_timestamp();
  json_object_set_new(j_dialing, "tm_create", json_string(tmp));
  sfree(tmp);

  // insert data
  slog(LOG_DEBUG, "Check value. dial_channel[%s], dial_addr[%s], dial_index[%lld], dial_trycnt[%lld], dial_timeout[%lld], dial_type[%lld], dial_exten[%s], dial_application[%s]\n",
      json_string_value(json_object_get(j_dial, "dial_channel"))? : "",
      json_string_value(json_object_get(j_dial, "dial_addr"))? : "",
      json_integer_value(json_object_get(j_dial, "dial_index")),
      json_integer_value(json_object_get(j_dial, "dial_trycnt")),
      json_integer_value(json_object_get(j_dial, "dial_timeout")),
      json_integer_value(json_object_get(j_dial, "dial_type")),
      json_string_value(json_object_get(j_dial, "dial_exten"))? : "",
      json_string_value(json_object_get(j_dial, "dial_application"))? : ""
      );
  db_insert("ob_dialing", j_dialing);
  return j_dialing;
}

void rb_dialing_destory(rb_dialing* dialing)
{
  slog(LOG_ERR, "Need to fix.");
//  ast_mutex_lock(&g_rb_dialing_mutex);
//
//  slog(LOG_DEBUG, "Destroying dialing.\n");
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
//  slog(LOG_DEBUG, "Called destroyer.\n");
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
  slog(LOG_ERR, "Need to fix.");
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
  slog(LOG_ERR, "Need to fix.");
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
  slog(LOG_ERR, "Need to fix.");
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
  slog(LOG_ERR, "Need to fix.");
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
  slog(LOG_ERR, "Need to fix.");
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

//bool rb_dialing_update_status(rb_dialing* dialing, E_DIALING_STATUS_T status)
//{
//  slog(LOG_ERR, "Need to fix.");
//  return false;
//
////  if(dialing == NULL) {
////    return false;
////  }
////
////  ast_mutex_lock(&g_rb_dialing_mutex);
////
////  dialing->status = status;
////
////  ast_mutex_unlock(&g_rb_dialing_mutex);
////
////  return true;
//}

rb_dialing* rb_dialing_find_chan_name(const char* name)
{
  slog(LOG_ERR, "Need to fix.");
  return NULL;

//  rb_dialing* dialing;
//
//  if(name == NULL) {
//    return NULL;
//  }
//
//  slog(LOG_DEBUG, "rb_dialing_find_chan_name. name[%s]\n", name);
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
  slog(LOG_ERR, "Need to fix.");
  return NULL;

//  rb_dialing* dialing;
//
//  if(uuid == NULL) {
//    return NULL;
//  }
//
//  slog(LOG_DEBUG, "rb_dialing_find_chan_uuid. uuid[%s]\n", uuid);
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

bool rb_dialing_is_exist_uuid(const char* uuid)
{
  rb_dialing* dialing;

  if(uuid == NULL) {
    return false;
  }

  dialing = rb_dialing_find_chan_uuid(uuid);
  if(dialing == NULL) {
    return false;
  }

  return true;
}

//struct ao2_iterator rb_dialing_iter_init(void)
//{
//  slog(LOG_ERR, "Need to fix.");
//  return NULL;
//
////  struct ao2_iterator iter;
////
////  ast_mutex_lock(&g_rb_dialing_mutex);
////  iter = ao2_iterator_init(g_rb_dialings, 0);
////  ast_mutex_unlock(&g_rb_dialing_mutex);
////
////  return iter;
//}

void rb_dialing_iter_destroy(struct ao2_iterator* iter)
{
  slog(LOG_ERR, "Need to fix.");
  return;
//  ao2_iterator_destroy(iter);
//  return;
}

rb_dialing* rb_dialing_iter_next(struct ao2_iterator *iter)
{
  slog(LOG_ERR, "Need to fix.");
  return NULL;

//  rb_dialing* dialing;
//
//  ast_mutex_lock(&g_rb_dialing_mutex);
//  dialing = ao2_iterator_next(iter);
//  if(dialing == NULL) {
//    ast_mutex_unlock(&g_rb_dialing_mutex);
//    return NULL;
//  }
//  ao2_ref(dialing, -1);
//  ast_mutex_unlock(&g_rb_dialing_mutex);
//
//  return dialing;
}

//json_t* rb_dialing_get_all_for_cli(void)
//{
//  struct ao2_iterator iter;
//  rb_dialing* dialing;
//  json_t* j_res;
//  json_t* j_tmp;
//
//  j_res = json_array();
//  iter = rb_dialing_iter_init();
//  while(1) {
//    dialing = rb_dialing_iter_next(&iter);
//    if(dialing == NULL) {
//      break;
//    }
//
//    j_tmp = json_deep_copy(dialing->j_dialing);
//    json_object_set_new(j_tmp, "status", json_integer(dialing->status));
//    json_array_append_new(j_res, j_tmp);
//  }
//  rb_dialing_iter_destroy(&iter);
//
//  return j_res;
//}

json_t* rb_dialing_get_info_for_cli(const char* uuid)
{
  rb_dialing* dialing;
  json_t* j_res;

  dialing = rb_dialing_find_chan_uuid(uuid);
  if(dialing == NULL) {
    return false;
  }

  j_res = json_pack("{"
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s"
      "}",
      "uuid",        dialing->uuid? : "",
      "status",      dialing->status,
      "name",        dialing->name? : "",

      "tm_create",  dialing->tm_create? : "",
      "tm_update",  dialing->tm_update? : "",
      "tm_delete",  dialing->tm_delete? : ""
      );
  json_object_set_new(j_res, "j_dialing", json_incref(dialing->j_dialing));
  json_object_set_new(j_res, "j_event", json_incref(dialing->j_event));

  return j_res;
}

/**
 * Get count of dialings
 * @return
 */
int rb_dialing_get_count(void)
{
  slog(LOG_ERR, "Need to fix.");
  return 0;

//  int ret;
//
//  if(g_rb_dialings == NULL) {
//    return 0;
//  }
//
//  ret = ao2_container_count(g_rb_dialings);
//
//  return ret;
}

/**
 * Return the count of the dialings of the campaign.
 * @param camp_uuid
 * @return
 */
int rb_dialing_get_count_by_camp_uuid(const char* camp_uuid)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  const char* tmp_const;
  int ret;

  if(camp_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired rb_dialing_get_count_by_camp_uuid.");

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

  tmp_const = json_string_value(json_object_get(j_res, "count(*)"));
  if(tmp_const == NULL) {
    json_decref(j_res);
    slog(LOG_ERR, "Could not get correct result.");
    return -1;
  }
  ret = atoi(tmp_const);
  json_decref(j_res);

  return ret;
}
