/*
 * campaign_handler.c
 *
 *  Created on: Nov 26, 2015
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
#include "ob_campaign_handler.h"
#include "ob_dl_handler.h"
#include "ob_plan_handler.h"

static json_t* get_deleted_campaign(const char* uuid);

static bool is_startable_campaign_schedule(json_t* j_camp);
static bool is_startable_campaign_schedule_day(json_t* j_camp, int day);
static bool is_startable_campaign_schedule_check(json_t* j_camp, const char* cur_date, const char* cur_time, int cur_day);
static bool is_stopable_campaign_schedule_day_date_list(json_t* j_camp);
static bool is_stoppable_campaign_schedule_check(json_t* j_camp, const char* cur_date, const char* cur_time, int cur_day);


/**
 * Create ob campaign.
 * @param j_camp
 * @return
 */
json_t* create_ob_campaign(const json_t* j_camp)
{
  int ret;
  char* uuid;
  char* tmp;
  json_t* j_tmp;

  if(j_camp == NULL) {
    return NULL;
  }
  j_tmp = json_deep_copy(j_camp);

  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_NOTICE, "Create ob_campaign. uuid[%s], name[%s]",
      json_string_value(json_object_get(j_tmp, "uuid")),
      json_string_value(json_object_get(j_tmp, "name"))
      );
  ret = db_insert("ob_campaign", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    sfree(uuid);
    return NULL;
  }

  j_tmp = get_ob_campaign(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create campaign info correctly. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }
  sfree(uuid);

  return j_tmp;
}

/**
 * Delete outbound campaign.
 * @param uuid
 * @return
 */
json_t* delete_ob_campaign(const char* uuid)
{
  json_t* j_tmp;
  json_t* j_res;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired delete_ob_campaign. uuid[%s]", uuid);

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "status", json_integer(E_CAMP_STOP));
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_DL_USE_NO));
  sfree(tmp);

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_campaign set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete campaign. uuid[%s]", uuid);
    return NULL;
  }

  j_res = get_deleted_campaign(uuid);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get deleted campaign info. uuid[%s]", uuid);
    return NULL;
  }

  return j_res;
}

/**
 * Get specified campaign
 * @return
 */
json_t* get_ob_campaign(const char* uuid)
{
  json_t* j_res;
  db_res_t* db_res;
  char* sql;

  if(uuid == NULL) {
    return NULL;
  }
  slog(LOG_DEBUG, "Get ob_campaign info. uuid[%s]", uuid);

  // get specified campaign
  asprintf(&sql, "select * from ob_campaign where uuid=\"%s\" and in_use=%d;", uuid, E_DL_USE_OK);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get ob_campaign info.");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  return j_res;
}

/**
 * Get deleted campaign.
 * @return
 */
static json_t* get_deleted_campaign(const char* uuid)
{
  json_t* j_res;
  db_res_t* db_res;
  char* sql;

  if(uuid == NULL) {
    return NULL;
  }
  slog(LOG_DEBUG, "Get deleted campaign info. uuid[%s]", uuid);

  // get specified campaign
  asprintf(&sql, "select * from ob_campaign where uuid=\"%s\" and in_use=%d;", uuid, E_DL_USE_NO);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get deleted ob_campaign info.");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  return j_res;
}

/**
 * Get all campaigns
 * @return
 */
json_t* get_ob_campaigns_all(void)
{
  json_t* j_res;
  json_t* j_tmp;
  db_res_t* db_res;
  char* sql;

  // get all campaigns
  asprintf(&sql, "select * from ob_campaign where in_use=%d", E_DL_USE_OK);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get ob_campaign info.");
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
 * Return the json array of all campaign uuid.
 * @return
 */
json_t* get_ob_campaigns_all_uuid(void)
{
  char* sql;
  const char* tmp_const;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from ob_campaign where in_use=%d;", E_DL_USE_OK);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct ob_campaign.");
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
 * Get all start-able campaigns by schedule.
 * \return
 */
json_t* get_campaigns_schedule_start(void)
{
  char* sql;
  json_t* j_res;
  json_t* j_tmp;
  db_res_t* db_res;
  int ret;

  asprintf(&sql, "select * from ob_campaign where status == %d and sc_mode == %d;",
      E_CAMP_STOP,
      E_CAMP_SCHEDULE_ON
      );

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    ret = is_startable_campaign_schedule(j_tmp);
    if(ret == false) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  db_free(db_res);

  return j_res;
}

/**
 * Get all stop-able campaigns by schedule.
 * \return
 */
json_t* get_campaigns_schedule_end(void)
{
  char* sql;
  json_t* j_res;
  json_t* j_tmp;
  db_res_t* db_res;
  int ret;

  asprintf(&sql, "select * from ob_campaign where status == %d and sc_mode == %d;",
      E_CAMP_START,
      E_CAMP_SCHEDULE_ON
      );

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    ret = is_stopable_campaign_schedule_day_date_list(j_tmp);
    if(ret == false) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  db_free(db_res);

  return j_res;
}



/**
 * Update ob_campaign
 * @param j_camp
 * @return
 */
json_t* update_ob_campaign(const json_t* j_camp)
{
  char* tmp;
  const char* tmp_const;
  char* sql;
  json_t* j_tmp;
  char* uuid;

  if(j_camp == NULL) {
    return NULL;
  }

  j_tmp = json_deep_copy(j_camp);
  if(j_tmp == NULL) {
    return NULL;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get uuid info.");
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

  asprintf(&sql, "update ob_campaign set %s where uuid=\"%s\" and in_use=%d;", tmp, uuid, E_DL_USE_OK);
  sfree(tmp);

  // update
  db_exec(sql);
  sfree(sql);

  slog(LOG_DEBUG, "Getting updated campaign info. uuid[%s]", uuid);
  j_tmp = get_ob_campaign(uuid);
  sfree(uuid);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get updated campaign info.");
    return NULL;
  }

  return j_tmp;
}

/**
 * Update campaign status info.
 * @param uuid
 * @param status
 * @return
 */
bool update_ob_campaign_status(const char* uuid, E_CAMP_STATUS_T status)
{
  char* tmp_status;
  json_t* j_camp;
  json_t* j_tmp;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return false;
  }

  if(status == E_CAMP_START)      tmp_status = "run";
  else if(status == E_CAMP_STOP)    tmp_status = "stop";
  else if(status == E_CAMP_PAUSE)   tmp_status = "pause";
  else if(status == E_CAMP_STARTING)  tmp_status = "running";
  else if(status == E_CAMP_STOPPING)  tmp_status = "stopping";
  else if(status == E_CAMP_PAUSING)   tmp_status = "pausing";
//  else if(status == E_CAMP_SCHEDULE_STOP)       tmp_status = "schedule_stop";
//  else if(status == E_CAMP_SCHEDULE_STOPPING)   tmp_status = "schedule_stopping";
  else {
    slog(LOG_WARNING, "Invalid input parameters.");
    return false;
  }
  slog(LOG_NOTICE, "update ob_campaign status. uuid[%s], status[%d], status_string[%s]", uuid, status, tmp_status);

  // get campaign
  j_camp = get_ob_campaign(uuid);
  if(j_camp == NULL) {
    slog(LOG_WARNING, "Could not find campaign. camp_uuid[%s]", uuid);
    return false;
  }

  // set status
  json_object_set_new(j_camp, "status", json_integer(status));

  // update
  j_tmp = update_ob_campaign(j_camp);
  json_decref(j_camp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not update campaign status. camp_uuid[%s], status[%d]", uuid, status);
    return false;
  }
  json_decref(j_tmp);

  return true;
}

/**
 * Get campaign for dialing.
 * @return
 */
json_t* get_ob_campaigns_by_status(E_CAMP_STATUS_T status)
{
  json_t* j_res;
  json_t* j_tmp;
  db_res_t* db_res;
  char* sql;

  // get "start" status campaign only.
  asprintf(&sql, "select * from ob_campaign where status = %d and in_use=%d;",
      status, E_DL_USE_OK
      );

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get ob_campaign info.");
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
 * Get campaign for dialing.
 * @return
 */
json_t* get_ob_campaign_for_dialing(void)
{
  json_t* j_res;
  db_res_t* db_res;
  char* sql;

  // get "start" status campaign only.
  asprintf(&sql, "select * from ob_campaign where status = %d and in_use = %d order by %s limit 1;",
      E_CAMP_START,
      E_DL_USE_OK,
      "random()"
      );

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get ob_campaign info.");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  return j_res;
}

/**
 *
 * \param uuid
 * \return
 */
json_t* get_ob_campaign_stat(const char* uuid)
{
  json_t* j_camp;
  json_t* j_plan;
  json_t* j_dlma;
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get campaign
  j_camp = get_ob_campaign(uuid);
  if(j_camp == NULL) {
    slog(LOG_DEBUG, "Could not get ob_campaign info. camp_uuid[%s]", uuid);
    return NULL;
  }

  // get plan
  j_plan = get_ob_plan(json_string_value(json_object_get(j_camp, "plan")));
  j_dlma = get_ob_dlma(json_string_value(json_object_get(j_camp, "dlma")));
  if((j_plan == NULL) || (j_dlma == NULL)) {
    slog(LOG_DEBUG, "Could not basic info. camp_uuid[%s], plan_uuid[%s], dlma_uuid[%s]",
        uuid,
        json_string_value(json_object_get(j_camp, "plan")),
        json_string_value(json_object_get(j_camp, "dlma"))
        );
    json_decref(j_camp);
    return NULL;
  }

  // create
  j_res = json_pack("{"
      "s:s, "
      "s:i, s:i, s:i, s:i, s:i"
      "}",

      "uuid",                  json_string_value(json_object_get(j_camp, "uuid"))? : "",

      "dial_total_count",      get_ob_dl_list_cnt_total(j_dlma),
      "dial_finished_count",  get_ob_dl_list_cnt_finshed(j_dlma, j_plan),
      "dial_available_count",  get_ob_dl_list_cnt_available(j_dlma, j_plan),
      "dial_dialing_count",    get_ob_dl_list_cnt_dialing(j_dlma),
      "dial_called_count",    get_ob_dl_list_cnt_tried(j_dlma)
      );

  json_decref(j_camp);
  json_decref(j_plan);
  json_decref(j_dlma);

  return j_res;
}

json_t* get_campaigns_stat_all(void)
{
  json_t* j_stats;
  json_t* j_stat;
  json_t* j_camps;
  json_t* j_camp;
  int i;
  int size;

  j_camps = get_ob_campaigns_all();
  size = json_array_size(j_camps);
  j_stats = json_array();
  for(i = 0; i < size; i++) {
    j_camp = json_array_get(j_camps, i);
    if(j_camp == NULL) {
      continue;
    }

    j_stat = get_ob_campaign_stat(json_string_value(json_object_get(j_camp, "uuid")));
    if(j_stat == NULL) {
      continue;
    }

    json_array_append_new(j_stats, j_stat);
  }

  json_decref(j_camps);

  return j_stats;
}


/**
 * return the possibility of status change to start
 */
bool is_startable_campgain(json_t* j_camp)
{
  if(j_camp == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  return true;
}

/**
 * return the possibility of status change to stop
 */
bool is_stoppable_campgain(json_t* j_camp)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;
  int ret;

  if(j_camp == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired is_stoppable_campgain. uuid[%s], name[%s]",
      json_string_value(json_object_get(j_camp, "uuid"))? : "",
      json_string_value(json_object_get(j_camp, "name"))? : ""
      );

  asprintf(&sql, "select count(*) from ob_dialing where uuid_camp=\"%s\";",
      json_string_value(json_object_get(j_camp, "uuid"))? : ""
      );

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_INFO, "Could not get correct stoppable campaign info.");
    return false;
  }

  j_tmp = db_get_record(db_res);
  db_free(db_res);
  if(j_tmp == NULL) {
    slog(LOG_INFO, "Could not get correct stoppable campaign info.");
    return false;
  }

  ret = json_integer_value(json_object_get(j_tmp, "count(*)"));
  json_decref(j_tmp);
  if(ret > 0) {
    return false;
  }

  return true;
}

static bool is_startable_campaign_schedule(json_t* j_camp)
{
  char* cur_date;
  char* cur_time;
  int cur_day;
  int ret;

  if(j_camp == NULL) {
    return false;
  }

  cur_date = get_utc_timestamp_date();
  cur_time = get_utc_timestamp_time();
  cur_day = get_utc_timestamp_day();

  // check startable date
  ret = is_startable_campaign_schedule_check(j_camp, cur_date, cur_time, cur_day);
  sfree(cur_date);
  sfree(cur_time);
  if(ret == true) {
    return true;
  }

  return false;
}

static bool is_startable_campaign_schedule_check(
    json_t* j_camp,
    const char* cur_date,
    const char* cur_time,
    int cur_day
    )
{
  const char* date_list;
  const char* date_except;
  char* tmp;
  const char* tmp_const;
  int ret;

  if((j_camp == NULL) || (cur_date == NULL) || (cur_time == NULL)) {
    return false;
  }

  // check except date. If there's except date, return false
  date_except = json_string_value(json_object_get(j_camp, "sc_date_list_except"));
  if(date_except != NULL) {
    tmp = strstr(date_except, cur_date);
    if(tmp != NULL) {
      return false;
    }
  }

  // check date_list. If there's current date, return true
  date_list = json_string_value(json_object_get(j_camp, "sc_date_list"));
  if(date_list != NULL) {
    tmp = strstr(date_list, cur_date);
    if(tmp != NULL) {
      return true;
    }
  }

  // check start date.
  // if start date is in the future, return false
  tmp_const = json_string_value(json_object_get(j_camp, "sc_date_start"));
  if(tmp_const != NULL) {
    ret = strcmp(tmp_const, cur_date);
    if(ret > 0) {
      return false;
    }
  }

  // check end date.
  // if end date is in the fast, return false
  tmp_const = json_string_value(json_object_get(j_camp, "sc_date_end"));
  if(tmp_const != NULL) {
    ret = strcmp(tmp_const, cur_date);
    if(ret < 0) {
      return false;
    }
  }

  // check startable day
  ret = is_startable_campaign_schedule_day(j_camp, cur_day);
  if(ret == false) {
    return false;
  }

  // check end time
  // if end time is in the fast, return false
  tmp_const = json_string_value(json_object_get(j_camp, "sc_time_end"));
  if(tmp_const != NULL) {
    ret = strcmp(tmp_const, cur_time);
    if(ret < 0) {
      return false;
    }
  }

  // check start time
  // if start time is in the future, return false
  tmp_const = json_string_value(json_object_get(j_camp, "sc_time_start"));
  if(tmp_const != NULL) {
    ret = strcmp(tmp_const, cur_time);
    if(ret > 0) {
      return false;
    }
  }

  return true;
}

static bool is_startable_campaign_schedule_day(json_t* j_camp, int day)
{
  const char* day_list;
  char* tmp;
  char   day_str[2];

  // 0=Sunday, 1=Monday, ..., 6=Saturday
  if((j_camp == NULL) || (day < 0) || (day > 6)) {
    return false;
  }

  // if it doesn't set, just return true.
  day_list = json_string_value(json_object_get(j_camp, "sc_day_list"));
  if((day_list == NULL) || (strlen(day_list) == 0)) {
    return true;
  }

  // check date_list. If there's no current date, return false
  snprintf(day_str, sizeof(day_str), "%d", day);
  tmp = strstr(day_list, day_str);
  if(tmp == NULL) {
    return false;
  }

  return true;
}

static bool is_stopable_campaign_schedule_day_date_list(json_t* j_camp)
{
  char* cur_date;
  char* cur_time;
  int cur_day;
  int ret;

  if(j_camp == NULL) {
    return false;
  }

  cur_date = get_utc_timestamp_date();
  cur_time = get_utc_timestamp_time();
  cur_day = get_utc_timestamp_day();

  // check startable date
  ret = is_stoppable_campaign_schedule_check(j_camp, cur_date, cur_time, cur_day);
  sfree(cur_date);
  sfree(cur_time);
  if(ret == true) {
    return true;
  }

  return false;
}

static bool is_stoppable_campaign_schedule_check(json_t* j_camp, const char* cur_date, const char* cur_time, int cur_day)
{
  int ret;

  ret = is_startable_campaign_schedule_check(j_camp, cur_date, cur_time, cur_day);
  if(ret == true) {
    return false;
  }

  return true;
}

/**
 * Return existence of given ob_campaign uuid.
 * @param uuid
 * @return
 */
bool is_exist_ob_campaign(const char* uuid)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  asprintf(&sql, "select count(*) from ob_campaign where uuid=\"%s\" and in_use=%d;", uuid, E_DL_USE_OK);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get ob_campaign info. uuid[%s]", uuid);
    return false;
  }

  j_tmp = db_get_record(db_res);
  db_free(db_res);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get correct ob_campaign info. uuid[%s]", uuid);
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
