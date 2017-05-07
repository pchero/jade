/*
 * destination_handler.c
 *
 *  Created on: Oct 19, 2016
 *      Author: pchero
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "event_handler.h"
#include "ami_handler.h"
#include "ast_header.h"

#include "ob_destination_handler.h"
#include "db_ctx_handler.h"
#include "ob_dl_handler.h"
#include "ob_campaign_handler.h"

static json_t* get_deleted_ob_destination(const char* uuid);
static json_t* create_ob_destination_default(void);
static json_t* get_ob_destination_use(const char* uuid, E_USE use);

static int get_avail_cnt_exten(json_t* j_dest);
static int get_avail_cnt_app(json_t* j_dest);
static int get_avail_cnt_app_park(void);
static int get_avail_cnt_app_queue(const char* name);
static int get_avail_cnt_app_queue_service_perf(const char* name);
static int get_avail_cnt_app_queue_available_member(const char* name);

extern db_ctx_t* g_db_ob;

/**
 * Create destination.
 * @param j_dest
 * @return
 */
json_t* create_ob_destination(json_t* j_dest)
{
  int ret;
  char* uuid;
  char* tmp;
  json_t* j_tmp;

  if(j_dest == NULL) {
    slog(LOG_ERR, "Wrong input parameter.");
    return false;
  }

  j_tmp = create_ob_destination_default();
  json_object_update_existing(j_tmp, j_dest);

  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_NOTICE, "Create ob_destination. uuid[%s], name[%s]",
      json_string_value(json_object_get(j_tmp, "uuid")),
      json_string_value(json_object_get(j_tmp, "name"))? : ""
      );
  ret = db_ctx_insert(g_db_ob, "ob_destination", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    sfree(uuid);
    return NULL;
  }

  j_tmp = get_ob_destination(uuid);
  sfree(uuid);

  return j_tmp;
}

static json_t* create_ob_destination_default(void)
{
  json_t* j_res;
  slog(LOG_DEBUG, "Fired create_ob_destination_default.");

  j_res = json_pack("{"
      "s:o, s:o, "
      "s:o, s:o, s:o, "
      "s:o, s:o, "
      "s:o, "
      "}",

      "name",     json_null(),
      "detail",   json_null(),

      "context",  json_null(),
      "exten",    json_null(),
      "priority", json_null(),

      "application",  json_null(),
      "data",         json_null(),

      "variables",    json_object()
      );

  return j_res;
}

/**
 * Delete destination.
 * @param uuid
 * @return
 */
json_t* delete_ob_destination(const char* uuid)
{
  json_t* j_tmp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    slog(LOG_ERR, "Wrong input parameter.");
    return NULL;
  }

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_USE_NO));
  sfree(tmp);

  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_destination set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete ob_destination. uuid[%s]", uuid);
    return NULL;
  }

  j_tmp = get_deleted_ob_destination(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get deleted destination info. uuid[%s]", uuid);
    return NULL;
  }

  return j_tmp;
}

static json_t* get_ob_destination_use(const char* uuid, E_USE use)
{
  char* sql;
  json_t* j_res;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ob_destination_use. uuid[%s], use[%d]", uuid, use);

  asprintf(&sql, "select * from ob_destination where uuid=\"%s\" and in_use=%d;", uuid, use);

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_destination info. uuid[%s], use[%d]", uuid, use);
    return NULL;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);

  return j_res;
}

/**
 * Get specified destination
 * @return
 */
json_t* get_ob_destination(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_ERR, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_ob_destination_use(uuid, E_USE_OK);
  if(j_res == NULL) {
    slog(LOG_WARNING, "Could not get ob_destination info.");
    return NULL;
  }

  return j_res;
}

/**
 * Get specified destination
 * @return
 */
static json_t* get_deleted_ob_destination(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_ob_destination_use(uuid, E_USE_NO);
  return j_res;
}

/**
 * Returns list of all destination uuid.
 * @return
 */
json_t* get_ob_destinations_all_uuid(void)
{

  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select uuid from ob_destination where in_use=%d;", E_USE_OK);
  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct ob_destination info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ob);
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
  db_ctx_free(g_db_ob);

  return j_res;
}

/**
 * Get all destinations
 * @return
 */
json_t* get_ob_destinations_all(void)
{

  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_tmp;

  asprintf(&sql, "select * from ob_destination where in_use=%d;", E_USE_OK);
  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct ob_destination info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ob);
    if(j_tmp == NULL) {
      break;
    }

    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(g_db_ob);

  return j_res;
}

/**
 * Update ob_destination
 * @param j_dest
 * @return updated ob_destination info
 */
json_t* update_ob_destination(const json_t* j_dest)
{
  char* tmp;
  const char* tmp_const;
  char* sql;
  json_t* j_tmp;
  char* uuid;
  int ret;

  if(j_dest == NULL) {
    slog(LOG_ERR, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_deep_copy(j_dest);
  if(j_tmp == NULL) {
    return false;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get uuid info.");
    json_decref(j_tmp);
    return false;
  }
  uuid = strdup(tmp_const);

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(tmp));
  sfree(tmp);

  tmp = db_ctx_get_update_str(j_tmp);
  if(tmp == NULL) {
    slog(LOG_WARNING, "Could not get update str.");
    json_decref(j_tmp);
    sfree(uuid);
    return NULL;
  }
  json_decref(j_tmp);

  asprintf(&sql, "update ob_destination set %s where in_use=%d and uuid=\"%s\";", tmp, E_USE_OK, uuid);
  sfree(tmp);

  // update
  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not update ob_destination info. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }

  // get updated ob_destination info
  j_tmp = get_ob_destination(uuid);
  sfree(uuid);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get updated ob_destination info.");
    return NULL;
  }

  return j_tmp;
}

/**
 * Return the available resource count of given destination
 * \param j_dest
 * \return
 */
int get_ob_destination_available_count(json_t* j_dest)
{
  int type;
  int ret;

  type = json_integer_value(json_object_get(j_dest, "type"));
  switch(type) {
    case DESTINATION_EXTEN:
    {
      ret = get_avail_cnt_exten(j_dest);
    }
    break;

    case DESTINATION_APPLICATION:
    {
      ret = get_avail_cnt_app(j_dest);
    }
    break;

    default:
    {
      slog(LOG_ERR, "No support destination type. type[%d]", type);
      ret = 0;
    }
    break;
  }

  return ret;
}

/**
 * \param j_dest
 * \return
 */
static int get_avail_cnt_exten(json_t* j_dest)
{
  return DEF_DESTINATION_AVAIL_CNT_UNLIMITED;
}

/**
 * TODO:
 * \param j_dest
 * \return
 */
static int get_avail_cnt_app(json_t* j_dest)
{
  const char* application;
  const char* data;
  int ret;

  if(j_dest == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 0;
  }

  application = json_string_value(json_object_get(j_dest, "application"));
  if(application == NULL) {
    slog(LOG_WARNING, "Could not get correct application name. uuid[%s]",
        json_string_value(json_object_get(j_dest, "uuid"))
        );
    return 0;
  }

  data = json_string_value(json_object_get(j_dest, "data"));

  if(strcasecmp(application, "queue") == 0) {
    ret = get_avail_cnt_app_queue(data);
  }
  else if(strcasecmp(application, "park") == 0) {
    ret = get_avail_cnt_app_park();
  }
  else {
    slog(LOG_WARNING, "Unsupported application. Consider unlimited. application[%s]", application);
    ret = DEF_DESTINATION_AVAIL_CNT_UNLIMITED;
  }
  slog(LOG_DEBUG, "Available application count. cnt[%d]", ret);

  return ret;
}

static int get_avail_cnt_app_queue_available_member(const char* name)
{
  int ret;
  char* sql;
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 0;
  }
  slog(LOG_DEBUG, "Fired get_avail_cnt_app_queue.");

  asprintf(&sql, "select count(*) from queue_member where queue_name=\"%s\""
      " and status = %d"
      " and paused = %d"
      ";",
      name,
      AST_DEVICE_NOT_INUSE,
      0
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get correct available queue member info. queue_name[%s]", name);
    return 0;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get correct available queue member count. queue_name[%s]", name);
    return 0;
  }

  ret = json_integer_value(json_object_get(j_res, "count(*)"));
  json_decref(j_res);
  return ret;
}

static int get_avail_cnt_app_queue_service_perf(const char* name)
{
  int ret;
  char* sql;
  json_t* j_res;
  double service_perf;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 0;
  }
  slog(LOG_DEBUG, "fired get_avail_cnt_app_queue_service_perf.");

  asprintf(&sql, "select service_level_perf from queue_param where name=\"%s\";",
      name
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get correct queue performance info. queue_name[%s]", name);
    return 0;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get correct queue performance info. queue_name[%s]", name);
    return 0;
  }

  // get service level performance
  service_perf = json_real_value(json_object_get(j_res, "service_level_perf"));
  json_decref(j_res);
  if(service_perf == 0) {
    service_perf = 1;
  }

  // convert type. we need only the integer here.
  ret = (int)service_perf;

  return ret;
}

/**
 * Get available count for application.
 * application: Queue
 * @param name
 * @return
 */
static int get_avail_cnt_app_queue(const char* name)
{
  int perf;
  int avail;
  int ret;

  // get performance
  perf = get_avail_cnt_app_queue_service_perf(name);

  // get available
  avail = get_avail_cnt_app_queue_available_member(name);

  ret = avail + (avail * perf / 100);
  slog(LOG_DEBUG, "Application queue available count. name[%s], available[%d], performance[%d], ret[%d]",
      name, avail, perf, ret
      );

  return ret;
}

/**
 *
 * \return -1:unlimited
 */
static int get_avail_cnt_app_park(void)
{
  return DEF_DESTINATION_AVAIL_CNT_UNLIMITED;
}


///**
// * Get Queue param only.
// * @param name
// * @return
// */
//static json_t* get_app_queue_param(const char* name)
//{
//  json_t* j_ami_res;
//  json_t* j_param;
//  json_t* j_tmp;
//  size_t size;
//  int i;
//  const char* tmp_const;
//
//  if(name == NULL) {
//    return NULL;
//  }
//  slog(LOG_DEBUG, "Getting queue param info. queue_name[%s]", name);
//
////  todo:
//  slog(LOG_ERR, "Need to fix. Need to get queue status.");
//
////  j_ami_res = ami_cmd_queue_status(name);
////  if(j_ami_res == NULL) {
////    slog(LOG_NOTICE, "Could not get queue status. name[%s]", name);
////    return NULL;
////  }
//
//  // get result.
//  i = 0;
//  j_param = NULL;
//  size = json_array_size(j_ami_res);
//  for(i = 0; i < size; i++) {
//    j_tmp = json_array_get(j_ami_res, i);
//
//    // Event check
//    tmp_const = json_string_value(json_object_get(j_tmp, "Event"));
//    if(tmp_const == NULL) {
//      continue;
//    }
//    if(strcmp(tmp_const, "QueueParams") != 0) {
//      continue;
//    }
//
//    // compare the queue name
//    tmp_const = json_string_value(json_object_get(j_tmp, "Queue"));
//    if(tmp_const == NULL) {
//      continue;
//    }
//    if(strcmp(tmp_const, name) == 0) {
//      j_param = json_deep_copy(j_tmp);
//      break;
//    }
//  }
//  json_decref(j_ami_res);
//  if(j_param == NULL) {
//    slog(LOG_NOTICE, "Could not get queue param. name[%s]", name);
//    return NULL;
//  }
//
//  return j_param;
//}

//json_t* get_app_queue_summary(const char* name)
//{
//  json_t* j_ami_res;
//  json_t* j_queue;
//  json_t* j_tmp;
//  size_t size;
//  int i;
//  const char* tmp_const;
//
//  if(name == NULL) {
//    return NULL;
//  }
//
//  //todo:
//  slog(LOG_ERR, "Need to fix. Need to get queue status.");
//
////  j_ami_res = ami_cmd_queue_summary(name);
////  if(j_ami_res == NULL) {
////    slog(LOG_NOTICE, "Could not get queue summary. name[%s]", name);
////    return NULL;
////  }
//
//  // get result.
//  i = 0;
//  j_queue = NULL;
//  size = json_array_size(j_ami_res);
//  for(i = 0; i < size; i++) {
//    j_tmp = json_array_get(j_ami_res, i);
//    tmp_const = json_string_value(json_object_get(j_tmp, "Queue"));
//    if(tmp_const == NULL) {
//      continue;
//    }
//    if(strcmp(tmp_const, name) == 0) {
//      j_queue = json_deep_copy(j_tmp);
//      break;
//    }
//  }
//  json_decref(j_ami_res);
//  if(j_queue == NULL) {
//    slog(LOG_NOTICE, "Could not get queue summary. name[%s]", name);
//    return NULL;
//  }
//
//  return j_queue;
//}

static json_t* create_destination_exten(json_t* j_dest)
{
  json_t* j_res;

  if(j_dest == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = json_pack("{s:s, s:s, s:s, s:s}",
      "dial_exten",      json_string_value(json_object_get(j_dest, "exten"))? : "",
      "dial_context",    json_string_value(json_object_get(j_dest, "context"))? : "",
      "dial_priority",  json_string_value(json_object_get(j_dest, "priority"))? : "",
      "dest_variables",  json_string_value(json_object_get(j_dest, "variables"))? : ""
      );

  return j_res;
}

static json_t* create_dial_destination_application(json_t* j_dest)
{
  json_t* j_res;

  if(j_dest == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = json_pack("{s:s, s:s, s:s}",
      "dial_application",    json_string_value(json_object_get(j_dest, "application"))? : "",
      "dial_data",          json_string_value(json_object_get(j_dest, "data"))? : "",
      "dest_variables",      json_string_value(json_object_get(j_dest, "variables"))? : ""
      );

  return j_res;
}

json_t* create_ob_dial_destination_info(json_t* j_dest)
{
  int type;
  json_t* j_res;

  if(j_dest == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  type = json_integer_value(json_object_get(j_dest, "type"));
  switch(type) {
    case DESTINATION_EXTEN:
    {
      j_res = create_destination_exten(j_dest);
    }
    break;

    case DESTINATION_APPLICATION:
    {
      j_res = create_dial_destination_application(j_dest);
    }
    break;

    default:
    {
      slog(LOG_ERR, "No support destination type. type[%d]", type);
      j_res = NULL;
    }
    break;
  }

  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get correct dial destination info.");
    return NULL;
  }

  // set dial_type
  json_object_set_new(j_res, "dial_type", json_integer(type));

  return j_res;
}

/**
 * Return existence of given destination uuid.
 * @param uuid
 * @return
 */
bool is_exist_ob_destination(const char* uuid)
{
  int ret;
  char* sql;
  json_t* j_tmp;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  asprintf(&sql, "select count(*) from ob_destination where uuid=\"%s\" and in_use=%d;", uuid, E_USE_OK);

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_destination info. uuid[%s]", uuid);
    return false;
  }

  j_tmp = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get correct ob_destination info. uuid[%s]", uuid);
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

/**
 * Return false if given destination is not deletable.
 * @param uuid
 * @return
 */
bool is_deletable_destination(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired is_deletable_destination. uuid[%s]", uuid);

  // check referenced campaign
  ret = is_referenced_destination_by_campaign(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "The given destination info is used in campaign. dlma_uuid[%s]", uuid);
    return false;
  }

  return true;
}

/**
 * Validate destination
 * @param
 * @return
 */
bool validate_ob_destination(json_t* j_data)
{
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired validate_ob_destination.");

  // variables
  j_tmp = json_object_get(j_data, "variables");
  if(j_tmp != NULL) {
    if(json_is_object(j_tmp) != true) {
      slog(LOG_NOTICE, "Wrong input type for variable. It should be json_object type.");
      return false;
    }
  }

  return true;
}
