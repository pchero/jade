/*
 * resource_handler.c
 *
 *  Created on: Feb 10, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "db_handler.h"
#include "resource_handler.h"


/**
 * Get all peer names array
 * @return
 */
json_t* get_peers_all_name(void)
{
  char* sql;
  const char* tmp_const;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from peer;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct peers name info.");
    return NULL;
  }

  j_res_tmp = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    tmp_const = json_string_value(json_object_get(j_tmp, "name"));
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
 * Get all database keys array
 * @return
 */
json_t* get_databases_all_key(void)
{
  char* sql;
  const char* tmp_const;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from database;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_res_tmp = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    tmp_const = json_string_value(json_object_get(j_tmp, "key"));
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
 * Get corresponding database info.
 * @return
 */
json_t* get_database_info(const char* key)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select * from database where key=\"%s\";", key);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_tmp = db_get_record(db_res);
  db_free(db_res);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}

/**
 * Get all registry account array
 * @return
 */
json_t* get_registries_all_account(void)
{
  char* sql;
  const char* tmp_const;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from registry;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct registry account info.");
    return NULL;
  }

  j_res_tmp = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
    if(j_tmp == NULL) {
      break;
    }

    tmp_const = json_string_value(json_object_get(j_tmp, "account"));
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
 * Get corresponding registry info.
 * @return
 */
json_t* get_registry_info(const char* account)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;

  if(account == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select * from registry where account=\"%s\";", account);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct registry account info.");
    return NULL;
  }

  j_tmp = db_get_record(db_res);
  db_free(db_res);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}
