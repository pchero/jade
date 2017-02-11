/*
 * plan_handler.c
 *
 *  Created on: Nov 30, 2015
 *    Author: pchero
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <jansson.h>

#include "utils.h"
#include "ami_handler.h"
#include "event_handler.h"
#include "slog.h"

#include "ob_plan_handler.h"
#include "db_handler.h"
#include "ob_campaign_handler.h"
#include "ob_dl_handler.h"

//static json_t* get_plan_deleted(const char* uuid);

/**
 *
 * @return
 */
bool init_plan(void)
{
  return true;
}

/**
 * Create plan.
 * @param j_plan
 * @return
 */
bool create_ob_plan(const json_t* j_plan)
{
  int ret;
  char* uuid;
  json_t* j_tmp;
  char* tmp;

  if(j_plan == NULL) {
    return false;
  }

  j_tmp = json_deep_copy(j_plan);
  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_NOTICE, "Create plan. uuid[%s], name[%s]\n",
      json_string_value(json_object_get(j_tmp, "uuid")),
      json_string_value(json_object_get(j_tmp, "name"))? : "<unknown>"
      );
  ret = db_insert("ob_plan", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    sfree(uuid);
    return false;
  }
  slog(LOG_VERBOSE, "Finished insert.\n");

//  // send ami event
//  j_tmp = get_plan(uuid);
//  slog(LOG_VERBOSE, "Check plan info. uuid[%s]\n",
//      json_string_value(json_object_get(j_tmp, "uuid"))
//      );
//  sfree(uuid);
//  if(j_tmp == NULL) {
//    slog(LOG_ERR, "Could not get created plan info.");
//    return false;
//  }
//  send_manager_evt_out_plan_create(j_tmp);
  json_decref(j_tmp);

  return true;
}

/**
 * Delete plan.
 * @param uuid
 * @return
 */
bool delete_ob_plan(const char* uuid)
{
  json_t* j_tmp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    return false;
  }

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_DL_USE_NO));
  sfree(tmp);

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_plan set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete ob_plan. uuid[%s]\n", uuid);
    return false;
  }

//  // send notification
//  j_tmp = get_plan_deleted(uuid);
//  send_manager_evt_out_plan_delete(j_tmp);
//  json_decref(j_tmp);

  return true;
}


/**
 * Get plan record info.
 * @param uuid
 * @return
 */
json_t* get_ob_plan(const char* uuid)
{
  char* sql;
  json_t* j_res;
  db_res_t* db_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.\n");
    return NULL;
  }
  asprintf(&sql, "select * from plan where in_use=%d and uuid=\"%s\";", E_DL_USE_OK, uuid);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get plan info. uuid[%s]\n", uuid);
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  return j_res;
}

///**
// * Get deleted plan info.
// * @param uuid
// * @return
// */
//static json_t* get_plan_deleted(const char* uuid)
//{
//  char* sql;
//  json_t* j_res;
//  db_res_t* db_res;
//
//  if(uuid == NULL) {
//    slog(LOG_WARNING, "Invalid input parameters.\n");
//    return NULL;
//  }
//  asprintf(&sql, "select * from plan where in_use=%d and uuid=\"%s\";", E_DL_USE_NO, uuid);
//
//  db_res = db_query(sql);
//  sfree(sql);
//  if(db_res == NULL) {
//    slog(LOG_ERR, "Could not get plan info. uuid[%s]\n", uuid);
//    return NULL;
//  }
//
//  j_res = db_get_record(db_res);
//  db_free(db_res);
//
//  return j_res;
//}

/**
 * Get all plan info.
 * @return
 */
json_t* get_ob_plans_all(void)
{
  char* sql;
  json_t* j_res;
  json_t* j_tmp;
  db_res_t* db_res;

  asprintf(&sql, "select * from plan where in_use=%d;", E_DL_USE_OK);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get plan all info.\n");
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
 * Get all plan's uuid array
 * @return
 */
json_t* get_ob_plans_all_uuid(void)
{
  char* sql;
  const char* tmp_const;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from plan;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct plan.");
    return NULL;
  }

  j_res_tmp = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
    if(tmp_const == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res_tmp, json_string(tmp_const));
    json_decref(j_tmp);
  }
  db_free(db_res);

  j_res = json_object();
  json_object_set_new(j_res, "list", j_res_tmp);

  return j_res;
}


/**
 * Update plan
 * @param j_plan
 * @return
 */
bool update_ob_plan(const json_t* j_plan)
{
  char* tmp;
  const char* tmp_const;
  char* sql;
  json_t* j_tmp;
  int ret;
  char* uuid;

  if(j_plan == NULL) {
    return false;
  }

  j_tmp = json_deep_copy(j_plan);
  if(j_tmp == NULL) {
    return false;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get uuid.\n");
    json_decref(j_tmp);
    return false;
  }
  uuid = strdup(tmp_const);

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(tmp));
  sfree(tmp);

  tmp = db_get_update_str(j_tmp);
  if(tmp == NULL) {
    slog(LOG_WARNING, "Could not get update str.\n");
    json_decref(j_tmp);
    sfree(uuid);
    return false;
  }
  json_decref(j_tmp);

  asprintf(&sql, "update ob_plan set %s where in_use=%d and uuid=\"%s\";", tmp, E_DL_USE_OK, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update ob_plan info. uuid[%s]\n", uuid);
    sfree(uuid);
    return false;
  }

//  j_tmp = get_plan(uuid);
//  sfree(uuid);
//  if(j_tmp == NULL) {
//    slog(LOG_WARNING, "Could not get updated plan info.\n");
//    return false;
//  }
//  send_manager_evt_out_plan_update(j_tmp);
//  json_decref(j_tmp);

  return true;
}

json_t* create_ob_dial_plan_info(json_t* j_plan)
{
  json_t* j_res;

  if(j_plan == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return NULL;
  }

  j_res = json_pack("{s:I, s:s}",
      "dial_timeout",   json_integer_value(json_object_get(j_plan, "dial_timeout")),
      "plan_variables",  json_string_value(json_object_get(j_plan, "variables"))? : ""
      );

  if(json_string_value(json_object_get(j_plan, "caller_id")) != NULL) {
    json_object_set_new(j_res, "callerid", json_incref(json_object_get(j_plan, "caller_id")));
  }

  return j_res;

}

/**
 * Return the is nonstop dl handle
 * \param j_plan
 * \return
 */
bool is_nonstop_ob_dl_handle(json_t* j_plan)
{
  int ret;

  if(j_plan == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return true;
  }

  ret = json_integer_value(json_object_get(j_plan, "dl_end_handle"));
  if(ret == E_PLAN_DL_END_NONSTOP) {
    return true;
  }
  return false;
}

/**
 * Return the is stoppable plan or not.
 * \param j_plan
 * \return
 */
bool is_endable_ob_plan(json_t* j_plan)
{
  int ret;

  if(j_plan == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return true;
  }

  ret = is_nonstop_ob_dl_handle(j_plan);
  if(ret == true) {
    slog(LOG_VERBOSE, "The plan dl_end_handle is nonstop. plan_uuid[%s], is_nonstop[%d]\n",
        json_string_value(json_object_get(j_plan, "uuid"))? : "",
        ret
        );
    return false;
  }

  return true;
}

