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

static json_t* get_deleted_ob_plan(const char* uuid);
static json_t* create_ob_plan_default(void);
static json_t* get_ob_plan_use(const char* uuid, E_USE use);

#define DEF_PLAN_DIMAL_MODE     E_DIAL_MODE_NONE
#define DEF_PLAN_TECH_NAME      "SIP"
#define DEF_PLAN_DIAL_TIMEOUT   30000
#define DEF_PLAN_DL_END         E_PLAN_DL_END_STOP
#define DEF_PLAN_RETRY_DELAY    60
#define DEF_PLAN_SERVICE_LEVEL  0
#define DEF_PLAN_MAX_RETRY_CNT  5

/**
 *
 * @return
 */
bool init_plan(void)
{
  return true;
}

/**
 * Validate plan
 * @param j_plan
 * @return
 */
bool validate_ob_plan(json_t* j_data)
{
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired validate_ob_plan.");

  // variables
  j_tmp = json_object_get(j_data, "variables");
  if(j_tmp != NULL) {
    if(json_is_object(j_tmp) != true) {
      slog(LOG_NOTICE, "Wrong input type for variable. It should be json_object type.");
      return false;
    }
  }

  // tech_name
  tmp_const = json_string_value(json_object_get(j_data, "tech_name"));
  if(tmp_const != NULL) {
    if((strcasecmp(tmp_const, "SIP") != 0)
        && (strcasecmp(tmp_const, "PJSIP") != 0)
        && (strcasecmp(tmp_const, "DAHDI") != 0)
        ) {
      slog(LOG_DEBUG, "Wrong tech_name value. value[%s]", tmp_const);
      return false;
    }
  }

  return true;
}

/**
 * Create plan.
 * @param j_plan
 * @return
 */
json_t* create_ob_plan(json_t* j_plan)
{
  int ret;
  char* uuid;
  json_t* j_tmp;
  char* tmp;

  if(j_plan == NULL) {
    return NULL;
  }
  slog(LOG_DEBUG, "Fired create_ob_plan.");

  j_tmp = create_ob_plan_default();
  json_object_update_existing(j_tmp, j_plan);

  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_NOTICE, "Create plan. uuid[%s], name[%s]",
      json_string_value(json_object_get(j_tmp, "uuid")),
      json_string_value(json_object_get(j_tmp, "name"))? : "<unknown>"
      );
  ret = db_insert("ob_plan", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    sfree(uuid);
    return NULL;
  }
  slog(LOG_DEBUG, "Finished insert.");

  j_tmp = get_ob_plan(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get created plan info. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }
  sfree(uuid);

  return j_tmp;
}

/**
 * Delete plan.
 * @param uuid
 * @return
 */
json_t* delete_ob_plan(const char* uuid)
{
  json_t* j_tmp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_USE_NO));
  sfree(tmp);

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_plan set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete ob_plan. uuid[%s]", uuid);
    return NULL;
  }

  // get deleted info
  j_tmp = get_deleted_ob_plan(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not delete ob_plan info. uuid[%s]", uuid);
    return NULL;
  }
  return j_tmp;
}

static json_t* get_ob_plan_use(const char* uuid, E_USE use)
{
  char* sql;
  json_t* j_res;
  db_res_t* db_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ob_plan_use. uuid[%s], use[%d]", uuid, use);

  asprintf(&sql, "select * from ob_plan where uuid=\"%s\" and in_use=%d;", uuid, use);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get ob_plan info. uuid[%s], use[%d]", uuid, use);
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  return j_res;
}

/**
 * Get plan record info.
 * @param uuid
 * @return
 */
json_t* get_ob_plan(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ob_plan. uuid[%s]", uuid);

  j_res = get_ob_plan_use(uuid, E_USE_OK);

  return j_res;
}

/**
 * Get deleted plan info.
 * @param uuid
 * @return
 */
static json_t* get_deleted_ob_plan(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_deleted_ob_plan. uuid[%s]", uuid);

  j_res = get_ob_plan_use(uuid, E_USE_NO);

  return j_res;
}

/**
 * Get all plan info.
 * @return
 */
json_t* get_ob_plans_all(void)
{
  json_t* j_uuids;
  json_t* j_val;
  json_t* j_res;
  json_t* j_tmp;
  unsigned int idx;
  const char* uuid;

  j_uuids = get_ob_plans_all_uuid();

  j_res = json_array();
  json_array_foreach(j_uuids, idx, j_val) {
    uuid = json_string_value(j_val);
    if(uuid == NULL) {
      continue;
    }

    j_tmp = get_ob_plan(uuid);
    if(j_tmp == NULL) {
      continue;
    }
    json_array_append_new(j_res, j_tmp);

  }
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

  slog(LOG_DEBUG, "Fired get_ob_plans_all_uuid.");

  asprintf(&sql, "select * from ob_plan where in_use=%d;", E_USE_OK);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct ob_plan.");
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
json_t* update_ob_plan(const json_t* j_plan)
{
  char* tmp;
  const char* tmp_const;
  char* sql;
  json_t* j_tmp;
  int ret;
  char* uuid;

  if(j_plan == NULL) {
    slog(LOG_WARNING, "Wrong inpu parameter.");
    return NULL;
  }

  j_tmp = json_deep_copy(j_plan);
  if(j_tmp == NULL) {
    return NULL;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get uuid.");
    json_decref(j_tmp);
    return NULL;
  }
  uuid = strdup(tmp_const);

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(tmp));
  sfree(tmp);

  tmp = db_get_update_str(j_tmp);
  if(tmp == NULL) {
    slog(LOG_WARNING, "Could not get update str.");
    json_decref(j_tmp);
    sfree(uuid);
    return NULL;
  }
  json_decref(j_tmp);

  asprintf(&sql, "update ob_plan set %s where in_use=%d and uuid=\"%s\";", tmp, E_USE_OK, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update ob_plan info. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }

  j_tmp = get_ob_plan(uuid);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get updated plan info. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }
  sfree(uuid);

  return j_tmp;
}

json_t* create_ob_dial_plan_info(json_t* j_plan)
{
  json_t* j_res;

  if(j_plan == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
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
    slog(LOG_WARNING, "Wrong input parameter.");
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
    slog(LOG_WARNING, "Wrong input parameter.");
    return true;
  }

  ret = is_nonstop_ob_dl_handle(j_plan);
  if(ret == true) {
    slog(LOG_DEBUG, "The plan dl_end_handle is nonstop. plan_uuid[%s], is_nonstop[%d]",
        json_string_value(json_object_get(j_plan, "uuid"))? : "",
        ret
        );
    return false;
  }

  return true;
}

/**
 * Return existence of given plan uuid.
 * @param uuid
 * @return
 */
bool is_exist_ob_plan(const char* uuid)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  asprintf(&sql, "select count(*) from ob_plan where uuid=\"%s\" and in_use=%d;",
      uuid,
      E_USE_OK
      );

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get ob_plan info. uuid[%s]", uuid);
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

static json_t* create_ob_plan_default(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired create_ob_plan_default.");

  j_res = json_pack("{"
      "s:o, s:o, "
      "s:i, s:i, s:i, s:i, s:o, s:s, s:i, "
      "s:o, s:o, s:o, "
      "s:o, "
      "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i"
      "}",

      "name",           json_null(),
      "detail",         json_null(),

      "dial_mode",      DEF_PLAN_DIMAL_MODE,
      "dial_timeout",   DEF_PLAN_DIAL_TIMEOUT,
      "dl_end_handle",  DEF_PLAN_DL_END,
      "retry_delay",    DEF_PLAN_RETRY_DELAY,
      "trunk_name",     json_null(),
      "tech_name",      DEF_PLAN_TECH_NAME,
      "service_level",  DEF_PLAN_SERVICE_LEVEL,

      "caller_id",      json_null(),
      "early_media",    json_null(),
      "codecs",         json_null(),

      "variables",      json_object(),

      "max_retry_cnt_1",  DEF_PLAN_MAX_RETRY_CNT,
      "max_retry_cnt_2",  DEF_PLAN_MAX_RETRY_CNT,
      "max_retry_cnt_3",  DEF_PLAN_MAX_RETRY_CNT,
      "max_retry_cnt_4",  DEF_PLAN_MAX_RETRY_CNT,
      "max_retry_cnt_5",  DEF_PLAN_MAX_RETRY_CNT,
      "max_retry_cnt_6",  DEF_PLAN_MAX_RETRY_CNT,
      "max_retry_cnt_7",  DEF_PLAN_MAX_RETRY_CNT,
      "max_retry_cnt_8",  DEF_PLAN_MAX_RETRY_CNT
      );

  return j_res;
}
