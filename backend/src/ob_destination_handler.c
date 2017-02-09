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

#include "ob_destination_handler.h"
#include "db_handler.h"
#include "ob_dl_handler.h"

static json_t* get_destination_deleted(const char* uuid);

static int get_avail_cnt_exten(json_t* j_dest);
static int get_avail_cnt_app(json_t* j_dest);
static int get_avail_cnt_app_park(void);
static int get_avail_cnt_app_queue(const char* name);
static int get_avail_cnt_app_queue_service_perf(const char* name);

static json_t* get_app_queue_param(const char* name);
json_t* get_app_queue_summary(const char* name);

/**
 * Create destination.
 * @param j_camp
 * @return
 */
bool create_destination(const json_t* j_dest)
{
  int ret;
  char* uuid;
  char* tmp;
  json_t* j_tmp;

  if(j_dest == NULL) {
    slog(LOG_ERR, "Wrong input parameter.\n");
    return false;
  }
  j_tmp = json_deep_copy(j_dest);

  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_NOTICE, "Create destination. uuid[%s], name[%s]\n",
      json_string_value(json_object_get(j_tmp, "uuid")),
      json_string_value(json_object_get(j_tmp, "name"))? : ""
      );
  ret = db_insert("destination", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    sfree(uuid);
    return false;
  }

//  // send ami event
//  j_tmp = get_destination(uuid);
//  sfree(uuid);
//  send_manager_evt_out_destination_create(j_tmp);
//  json_decref(j_tmp);

  return true;
}


/**
 * Delete destination.
 * @param uuid
 * @return
 */
bool delete_destination(const char* uuid)
{
  json_t* j_tmp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    slog(LOG_ERR, "Wrong input parameter.\n");
    return false;
  }

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_DL_USE_NO));
  sfree(tmp);

  tmp = db_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update destination set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete destination. uuid[%s]\n", uuid);
    return false;
  }

//  // send notification
//  j_tmp = get_destination_deleted(uuid);
//  send_manager_evt_out_destination_delete(j_tmp);
//  json_decref(j_tmp);

  return true;
}

/**
 * Get specified destination
 * @return
 */
json_t* get_destination(const char* uuid)
{
  json_t* j_res;
  db_res_t* db_res;
  char* sql;

  if(uuid == NULL) {
    slog(LOG_ERR, "Wrong input parameter.\n");
    return NULL;
  }
  slog(LOG_VERBOSE, "Get destination info. uuid[%s]\n", uuid);

  // get specified destination
  asprintf(&sql, "select * from destination where uuid=\"%s\" and in_use=%d;", uuid, E_DL_USE_OK);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get destination info.\n");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  return j_res;
}

/**
 * Get specified destination
 * @return
 */
static json_t* get_destination_deleted(const char* uuid)
{
  json_t* j_res;
  db_res_t* db_res;
  char* sql;

  if(uuid == NULL) {
    slog(LOG_ERR, "Wrong input parameter.\n");
    return NULL;
  }
  slog(LOG_VERBOSE, "Get destination info. uuid[%s]\n", uuid);

  // get specified destination
  asprintf(&sql, "select * from destination where uuid=\"%s\" and in_use=%d;", uuid, E_DL_USE_NO);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get destination info.\n");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);

  return j_res;
}



/**
 * Get all destinations
 * @return
 */
json_t* get_destinations_all(void)
{
  json_t* j_res;
  json_t* j_tmp;
  db_res_t* db_res;
  char* sql;

  // get all campaigns
  asprintf(&sql, "select * from destination where in_use=%d;", E_DL_USE_OK);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get destinations info.\n");
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
 * Update destination
 * @param j_dest
 * @return
 */
bool update_destination(const json_t* j_dest)
{
  char* tmp;
  const char* tmp_const;
  char* sql;
  json_t* j_tmp;
  char* uuid;

  if(j_dest == NULL) {
    slog(LOG_ERR, "Wrong input parameter.\n");
    return false;
  }

  j_tmp = json_deep_copy(j_dest);
  if(j_tmp == NULL) {
    return false;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get uuid info.\n");
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

  asprintf(&sql, "update destination set %s where in_use=%d and uuid=\"%s\";", tmp, E_DL_USE_OK, uuid);
  sfree(tmp);

  db_exec(sql);
  sfree(sql);

//  j_tmp = get_destination(uuid);
//  sfree(uuid);
//  if(j_tmp == NULL) {
//    slog(LOG_WARNING, "Could not get updated destination info.\n");
//    return false;
//  }
//  send_manager_evt_out_destination_update(j_tmp);
//  json_decref(j_tmp);

  return true;
}

/**
 * Return the available resource count of given destination
 * \param j_dest
 * \return
 */
int get_destination_available_count(json_t* j_dest)
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
      slog(LOG_ERR, "No support destination type. type[%d]\n", type);
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
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return 0;
  }

  application = json_string_value(json_object_get(j_dest, "application"));
  if(application == NULL) {
    slog(LOG_WARNING, "Could not get correct application name. uuid[%s]\n",
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
    slog(LOG_WARNING, "Unsupported application. Consider unlimited. application[%s]\n", application);
    ret = DEF_DESTINATION_AVAIL_CNT_UNLIMITED;
  }

  slog(LOG_VERBOSE, "Available application count. cnt[%d]\n", ret);

  return ret;
}

static int get_avail_cnt_app_queue_service_perf(const char* name)
{
  json_t* j_tmp;
  double service_perf;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return 0;
  }

  // get queue param
  j_tmp = get_app_queue_param(name);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue_param info. queue_name[%s]\n", name);
    return 0;
  }

  service_perf = atof(json_string_value(json_object_get(j_tmp, "ServicelevelPerf")));
  if(service_perf == 0) {
    service_perf = 100;
  }
  json_decref(j_tmp);

  return service_perf;
}

static int get_avail_cnt_app_queue(const char* name)
{
  json_t* j_tmp;
  const char* tmp_const;
  int ret;
  int perf;
  int avail;
  int loggedin;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return 0;
  }

  // get queue summary info.
  j_tmp = get_app_queue_summary(name);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue_summary info. queue_name[%s]\n", name);
    return 0;
  }

  // get available, loggedin
  tmp_const = json_string_value(json_object_get(j_tmp, "Available"));
  avail = atoi(tmp_const);

  tmp_const = json_string_value(json_object_get(j_tmp, "LoggedIn"));
  loggedin = atoi(tmp_const);
  json_decref(j_tmp);

  if(loggedin < 10) {
    slog(LOG_VERBOSE, "Not many people logged in. Ignore perf calculate. loggenin[%d], avail[%d]\n", loggedin, avail);
    return avail;
  }

  // get perf
  perf = get_avail_cnt_app_queue_service_perf(name);

  ret = avail * perf / 100;
  slog(LOG_DEBUG, "Application queue available count. name[%s], available[%d], performance[%d]\n", name, avail, perf);
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


/**
 * Get Queue param only.
 * @param name
 * @return
 */
static json_t* get_app_queue_param(const char* name)
{
  json_t* j_ami_res;
  json_t* j_param;
  json_t* j_tmp;
  size_t size;
  int i;
  const char* tmp_const;

  if(name == NULL) {
    return NULL;
  }
  slog(LOG_DEBUG, "Getting queue param info. queue_name[%s]\n", name);

//  todo:
  slog(LOG_ERR, "Need to fix. Need to get queue status.");

//  j_ami_res = ami_cmd_queue_status(name);
//  if(j_ami_res == NULL) {
//    slog(LOG_NOTICE, "Could not get queue status. name[%s]\n", name);
//    return NULL;
//  }

  // get result.
  i = 0;
  j_param = NULL;
  size = json_array_size(j_ami_res);
  for(i = 0; i < size; i++) {
    j_tmp = json_array_get(j_ami_res, i);

    // Event check
    tmp_const = json_string_value(json_object_get(j_tmp, "Event"));
    if(tmp_const == NULL) {
      continue;
    }
    if(strcmp(tmp_const, "QueueParams") != 0) {
      continue;
    }

    // compare the queue name
    tmp_const = json_string_value(json_object_get(j_tmp, "Queue"));
    if(tmp_const == NULL) {
      continue;
    }
    if(strcmp(tmp_const, name) == 0) {
      j_param = json_deep_copy(j_tmp);
      break;
    }
  }
  json_decref(j_ami_res);
  if(j_param == NULL) {
    slog(LOG_NOTICE, "Could not get queue param. name[%s]\n", name);
    return NULL;
  }

  return j_param;
}

json_t* get_app_queue_summary(const char* name)
{
  json_t* j_ami_res;
  json_t* j_queue;
  json_t* j_tmp;
  size_t size;
  int i;
  const char* tmp_const;

  if(name == NULL) {
    return NULL;
  }

  //todo:
  slog(LOG_ERR, "Need to fix. Need to get queue status.");

//  j_ami_res = ami_cmd_queue_summary(name);
//  if(j_ami_res == NULL) {
//    slog(LOG_NOTICE, "Could not get queue summary. name[%s]\n", name);
//    return NULL;
//  }

  // get result.
  i = 0;
  j_queue = NULL;
  size = json_array_size(j_ami_res);
  for(i = 0; i < size; i++) {
    j_tmp = json_array_get(j_ami_res, i);
    tmp_const = json_string_value(json_object_get(j_tmp, "Queue"));
    if(tmp_const == NULL) {
      continue;
    }
    if(strcmp(tmp_const, name) == 0) {
      j_queue = json_deep_copy(j_tmp);
      break;
    }
  }
  json_decref(j_ami_res);
  if(j_queue == NULL) {
    slog(LOG_NOTICE, "Could not get queue summary. name[%s]\n", name);
    return NULL;
  }

  return j_queue;
}

static json_t* create_destination_exten(json_t* j_dest)
{
  json_t* j_res;

  if(j_dest == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
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
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return NULL;
  }

  j_res = json_pack("{s:s, s:s, s:s}",
      "dial_application",    json_string_value(json_object_get(j_dest, "application"))? : "",
      "dial_data",          json_string_value(json_object_get(j_dest, "data"))? : "",
      "dest_variables",      json_string_value(json_object_get(j_dest, "variables"))? : ""
      );

  return j_res;
}

json_t* create_dial_destination_info(json_t* j_dest)
{
  int type;
  json_t* j_res;

  if(j_dest == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
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
      slog(LOG_ERR, "No support destination type. type[%d]\n", type);
      j_res = NULL;
    }
    break;
  }

  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get correct dial destination info.\n");
    return NULL;
  }

  // set dial_type
  json_object_set_new(j_res, "dial_type", json_integer(type));

  return j_res;
}

