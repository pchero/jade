/*
 * ob_dlma_handler.c
 *
 *  Created on: Mar 21, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "db_ctx_handler.h"
#include "utils.h"

#include "ob_dlma_handler.h"
#include "ob_dl_handler.h"
#include "ob_campaign_handler.h"

static bool create_dlma_view(const char* uuid, const char* view_name);
static json_t* create_ob_dlma_default(void);
static json_t* get_ob_dlma_use(const char* uuid, E_USE use);
static char* create_view_name(const char* uuid);

extern db_ctx_t* g_db_ob;

/**
 * Create dl_list_ma.
 * @param j_dlma
 * @return
 */
json_t* create_ob_dlma(json_t* j_dlma)
{
  int ret;
  char* uuid;
  char* view_name;
  char* tmp;
  json_t* j_tmp;

  if(j_dlma == NULL) {
    return NULL;
  }

  // create default dlma and update
  j_tmp = create_ob_dlma_default();
  json_object_update_existing(j_tmp, j_dlma);

  // uuid
  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  // create view_table
  view_name = create_view_name(uuid);
  create_dlma_view(uuid, view_name);
  json_object_set_new(j_tmp, "dl_table", json_string(view_name));
  sfree(view_name);

  // create timestamp
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_NOTICE, "Create dlma. uuid[%s], name[%s]",
      json_string_value(json_object_get(j_tmp, "uuid")),
      json_string_value(json_object_get(j_tmp, "name"))
      );

  ret = db_ctx_insert(g_db_ob, "ob_dl_list_ma", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create dlma info. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }

  j_tmp = get_ob_dlma(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get created dlma info. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }
  sfree(uuid);

  return j_tmp;
}

static bool create_dlma_view(const char* uuid, const char* view_name)
{
  char* sql;
  int ret;

  if((uuid == NULL) || (view_name == NULL)) {
    return false;
  }

  asprintf(&sql, "create view `%s` as select * from ob_dl_list where dlma_uuid=\"%s\";", view_name, uuid);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not create view. uuid[%s], view_name[%s]", uuid, view_name);
    return false;
  }

  return true;
}

static json_t* create_ob_dlma_default(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired create_ob_dlma_default.");

  j_res = json_pack("{"
      "s:o, s:o, "
      "s:s, "
      "s:o"
      "}",

      "name",     json_null(),
      "detail",   json_null(),

      "dl_table",   "",

      "variables",  json_object()
      );

  return j_res;
}

json_t* delete_ob_dlma(const char* uuid)
{
  json_t* j_tmp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired delete_ob_dlma. uuid[%s]", uuid);

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_USE_NO));
  sfree(tmp);

  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_dl_list_ma set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete ob_dlma. uuid[%s]", uuid);
    return NULL;
  }

  // get deleted dlma info
  j_tmp = get_deleted_ob_dlma(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get deleted ob_dlma info. uuid[%s]", uuid);
    return NULL;
  }

  return j_tmp;
}

static json_t* get_ob_dlma_use(const char* uuid, E_USE use)
{
  char* sql;
  json_t* j_res;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }

  asprintf(&sql, "select * from ob_dl_list_ma where uuid=\"%s\" and in_use=%d;", uuid, use);

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_dl_list_ma info. uuid[%s], use[%d]", uuid, use);
    return NULL;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);

  return j_res;
}


/**
 *
 * @param uuid
 * @return
 */
json_t* get_ob_dlma(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }

  j_res = get_ob_dlma_use(uuid, E_USE_OK);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get ob_dl_list_ma info. uuid[%s]", uuid);
    return NULL;
  }
  return j_res;
}

/**
 *
 * @param uuid
 * @return
 */
json_t* get_deleted_ob_dlma(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_deleted_ob_dlma. uuid[%s]", uuid);

  j_res = get_ob_dlma_use(uuid, E_USE_NO);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get ob_dl_list_ma info. uuid[%s]", uuid);
    return NULL;
  }

  return j_res;
}

/**
 * Validate dlma
 * @param j_plan
 * @return
 */
bool validate_ob_dlma(json_t* j_data)
{
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired validate_ob_dlma.");

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

/**
 * Update ob_dl_list_ma.
 * Return the updated ob_dlma info
 * @param j_dlma
 * @return
 */
json_t* update_ob_dlma(const json_t* j_dlma)
{
  char* tmp;
  const char* tmp_const;
  char* sql;
  json_t* j_tmp;
  char* uuid;
  int ret;

  if(j_dlma == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired update_ob_dlma.");

  j_tmp = json_deep_copy(j_dlma);

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get uuid.");
    json_decref(j_tmp);
    return NULL;
  }
  uuid = strdup(tmp_const);

  // update timestamp
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(tmp));
  sfree(tmp);

  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  if(tmp == NULL) {
    slog(LOG_WARNING, "Could not get update str.");
    sfree(uuid);
    return NULL;
  }

  asprintf(&sql, "update ob_dl_list_ma set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get updated ob_dlma. uuid[%s]", uuid);
    sfree(uuid);
    return NULL;
  }

  slog(LOG_DEBUG, "Getting updated dlma info. uuid[%s]", uuid);
  j_tmp = get_ob_dlma(uuid);
  sfree(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get updated dlma info.");
    return NULL;
  }

  return j_tmp;
}

/**
 *
 * @param uuid
 * @return
 */
json_t* get_ob_dlmas_all(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_tmp;

  asprintf(&sql, "select * from ob_dl_list_ma where in_use=%d;", E_USE_OK);
  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct ob_dlma info.");
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
 * Get all dlma's uuid array
 * @return
 */
json_t* get_ob_dlmas_all_uuid(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select uuid from ob_dl_list_ma where in_use=%d;", E_USE_OK);
  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct ob_dlma info.");
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
 * Create view name using uuid.
 * @param uuid
 * @return
 */
static char* create_view_name(const char* uuid)
{
  char* tmp;
  int len;
  int i;
  int j;

  if(uuid == NULL) {
    return NULL;
  }

  len = strlen(uuid);
  tmp = calloc(len + 1, sizeof(char));
  j = 0;
  for(i = 0; i < len; i++) {
    if(uuid[i] == '-') {
      tmp[j] = '_';
    }
    else {
      tmp[j] = uuid[i];
    }
    j++;
  }
  tmp[j] = '\0';
  return tmp;
}

char* get_ob_dlma_table_name(const char* dlma_uuid)
{
  json_t* j_dlma;
  const char* tmp_const;
  char* res;

  if(dlma_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ob_dl_table_name. dlma_uuid[%s]", dlma_uuid);

  j_dlma = get_ob_dlma(dlma_uuid);
  if(j_dlma == NULL) {
    slog(LOG_WARNING, "Could not get ob_dlma info. dlma_uuid[%s]", dlma_uuid);
    return NULL;
  }

  tmp_const = json_string_value(json_object_get(j_dlma, "dl_table"));
  if(tmp_const == NULL) {
    json_decref(j_dlma);
    slog(LOG_INFO, "Could not get dl_table info from ob_dlma. dlma_uuid[%s]", dlma_uuid);
    return NULL;
  }
  res = strdup(tmp_const);
  json_decref(j_dlma);

  return res;
}

/**
 * Check the given ob_dlma is deletable.
 * @param uuid
 * @return
 */
bool is_deletable_ob_dlma(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired is_deletable_ob_dlma. uuid[%s]", uuid);

  // check referenced campaign
  ret = is_referenced_dlma_by_campaign(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "The given dlma info is used in campaign. dlma_uuid[%s]", uuid);
    return false;
  }

  // check dl count
  ret = get_ob_dl_count_by_dlma_uuid(uuid);
  if(ret > 0) {
    slog(LOG_NOTICE, "There are available ob_dl info related with given dlma_uuid. dlma_uuid[%s]", uuid);
    return false;
  }

  return true;
}

