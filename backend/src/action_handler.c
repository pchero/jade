/*
 * action_handler.c
 *
 *  Created on: Feb 15, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "db_handler.h"
#include "action_handler.h"

bool insert_action(const char* id, const char* type)
{
  int ret;
  json_t* j_tmp;

  if((id == NULL) || (type == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_pack("{s:s, s:s}",
      "id",   id,
      "type", type
      );
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create action object.");
    return false;
  }

  ret = db_insert("action", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert action info.");
    return false;
  }

  return true;
}

json_t* get_action(const char* id)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select * from action where id=\"%s\";", id);

  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_ERR, "Could not get correct action info.");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);
  if(j_res == NULL) {
    // no result.
    return NULL;
  }

  return j_res;
}

bool delete_action(const char* id)
{
  char* sql;
  int ret;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  asprintf(&sql, "delete from action where id=\"%s\";", id);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    return false;
  }

  return true;
}

json_t* get_action_and_delete(const char* id)
{
  json_t* j_res;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong parameter.");
    return NULL;
  }

  j_res = get_action(id);
  delete_action(id);

  return j_res;
}
