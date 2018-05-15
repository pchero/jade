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
#include "db_sql_create.h"
#include <publication_handler.h>

#include "resource_handler.h"


extern app* g_app;

db_ctx_t* g_db_memory;
db_ctx_t* g_db_file;

// db
static bool clear_table(db_ctx_t* ctx, const char* table);
static bool insert_item(db_ctx_t* ctx, const char* table, const json_t* j_data);
static bool insrep_item(db_ctx_t* ctx, const char* table, const json_t* j_data);
static bool update_item(db_ctx_t* ctx, const char* table, const char* key_column, const json_t* j_data);
static json_t* get_items(db_ctx_t* ctx, const char* table, const char* item);
static json_t* get_detail_item_key_string(db_ctx_t* ctx, const char* table, const char* key, const char* val);
static json_t* get_detail_items_key_string(db_ctx_t* ctx, const char* table, const char* key, const char* val);
static bool delete_items_string(db_ctx_t* ctx, const char* table, const char* key, const char* val);
static bool delete_items_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj);
static json_t* get_detail_item_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj);
static json_t* get_detail_items_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj);
static json_t* get_detail_items_by_obj_order(db_ctx_t* ctx, const char* table, json_t* j_obj, const char* order);
static json_t* get_detail_items_by_condition(db_ctx_t* ctx, const char* table, const char* condition);

static bool init_db(void);
static bool init_ast_database(void);

/**
 * Initiate resource.
 * @return
 */
bool resource_init_handler(void)
{
  int ret;

  ret = init_db();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate db.");
    return false;
  }

  ret = init_ast_database();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ast database.");
    return false;
  }

  return true;
}

void resource_term_handler(void)
{
  slog(LOG_DEBUG, "Fired term_resource_handler.");

  db_ctx_term(g_db_memory);
  db_ctx_term(g_db_file);
}

static bool init_db(void)
{
  // init database
  const char* tmp_const;

  // init database_ast
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "database_name_ast"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get database_name_ast info.");
    return false;
  }

  g_db_memory = db_ctx_init(tmp_const);
  if(g_db_memory == NULL) {
    slog(LOG_WARNING, "Could not initiate db_ast.");
    return false;
  }

  // init database_jade
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "database_name_jade"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get database_name_jade info.");
    return false;
  }

  g_db_file = db_ctx_init(tmp_const);
  if(g_db_file == NULL) {
    slog(LOG_WARNING, "Could not initiate db_jade.");
    return false;
  }

  slog(LOG_NOTICE, "Finished db_init.");

  return true;
}

/**
 * Initialize ast database.
 * @return Success: true\n
 * Failure: false
 */
static bool init_ast_database(void)
{
  int ret;

  // action
  db_ctx_exec(g_db_memory, g_sql_drop_action);
  ret = db_ctx_exec(g_db_memory, g_sql_create_action);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "action");
    return false;
  }

  // system
  db_ctx_exec(g_db_memory, g_sql_drop_system);
  ret = db_ctx_exec(g_db_memory, g_sql_create_system);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "system");
    return false;
  }

  // core_agi
  db_ctx_exec(g_db_memory, g_sql_drop_core_agi);
  ret = db_ctx_exec(g_db_memory, g_sql_create_core_agi);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "core_agi");
    return false;
  }

  // core_module
  db_ctx_exec(g_db_memory, g_sql_drop_core_module);
  ret = db_ctx_exec(g_db_memory, g_sql_create_core_module);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "core_module");
    return false;
  }

  // database
  db_ctx_exec(g_db_memory, g_sql_drop_database);
  ret = db_ctx_exec(g_db_memory, g_sql_create_database);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "database");
    return false;
  }

  // agent
  db_ctx_exec(g_db_memory, g_sql_drop_agent);
  ret = db_ctx_exec(g_db_memory, g_sql_create_agent);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "agent");
    return false;
  }

  // device_state
  db_ctx_exec(g_db_memory, g_sql_drop_device_state);
  ret = db_ctx_exec(g_db_memory, g_sql_create_device_state);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "device_state");
    return false;
  }

  // voicemail_user
  db_ctx_exec(g_db_memory, g_sql_drop_voicemail_user);
  ret = db_ctx_exec(g_db_memory, g_sql_create_voicemail_user);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "voicemail_user");
    return false;
  }

  return true;
}

/**
 * Insert
 * @param table
 * @param item
 * @return
 */
static bool insert_item(db_ctx_t* ctx, const char* table, const json_t* j_data)
{
  int ret;

  if((ctx == NULL) || (table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_item. table[%s]", table);

  ret = db_ctx_insert(ctx, table, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert item.");
    return false;
  }

  return true;
}

/**
 * Insert or replace
 * @param ctx
 * @param table
 * @param j_data
 * @return
 */
static bool insrep_item(db_ctx_t* ctx, const char* table, const json_t* j_data)
{
  int ret;

  if((ctx == NULL) || (table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_item. table[%s]", table);

  ret = db_ctx_insert_or_replace(ctx, table, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert item.");
    return false;
  }

  return true;
}



/**
 * Update item info.
 * @param table
 * @param key_name
 * @param j_data
 * @return
 */
static bool update_item(db_ctx_t* ctx, const char* table, const char* key_column, const json_t* j_data)
{
  char* tmp;
  char* sql;
  char* sql_tmp;
  int ret;
  json_t* j_key;

  if((ctx == NULL) || (table == NULL) || (key_column == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get key
  j_key = json_object_get(j_data, key_column);
  if(j_key == NULL) {
    slog(LOG_ERR, "Could not get key column info. key_colum[%s]", key_column);
    return false;
  }

  // create update sql_tmp
  tmp = db_ctx_get_update_str(j_data);
  asprintf(&sql_tmp, "update %s set %s where %s",
      table,
      tmp,
      key_column
      );
  sfree(tmp);

  // create update sql
  if(json_is_string(j_key)) {
    asprintf(&sql, "%s=\"%s\";", sql_tmp, json_string_value(j_key));
  }
  else if(json_is_integer(j_key)) {
    asprintf(&sql, "%s=%"JSON_INTEGER_FORMAT";",sql_tmp, json_integer_value(j_key));
  }
  else if(json_is_real(j_key)) {
    asprintf(&sql, "%s=%f;", sql_tmp, json_real_value(j_key));
  }
  else {
    // wrong format
    slog(LOG_ERR, "Could not get key info.");
    sfree(sql_tmp);
    return false;
  }
  sfree(sql_tmp);

  // execute
  ret = db_ctx_exec(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update info.");
    return false;
  }

  return true;
}

static bool clear_table(db_ctx_t* ctx, const char* table)
{
  int ret;
  char* sql;

  if((ctx == NULL) || (table == NULL)) {
    slog(LOG_WARNING, "Wrong input paramter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired clear_table. table[%s]", table);

  asprintf(&sql, "delete from %s;", table);

  ret = db_ctx_exec(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not clear table.");
    return false;
  }

  return true;
}

/**
 *
 * @param table
 * @param item
 * @return
 */
static json_t* get_items(db_ctx_t* ctx, const char* table, const char* item)
{
  int ret;
  json_t* j_res;
  json_t* j_tmp;
  char* sql;

  if((ctx == NULL) || (table == NULL) || (item == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_items. table[%s], item[%s]", table, item);

  asprintf(&sql, "select %s from %s;", item, table);
  ret = db_ctx_query(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct databases item info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(ctx);
    if(j_tmp == NULL) {
      break;
    }

    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(ctx);

  return j_res;
}

/**
 * delete all selected items with string value.
 * @return
 */
static bool delete_items_string(db_ctx_t* ctx, const char* table, const char* key, const char* val)
{
  int ret;
  char* sql;

  if((ctx == NULL) || (table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_items_string. table[%s], key[%s], val[%s]", table, key, val);

  asprintf(&sql, "delete from %s where %s=\"%s\";", table, key, val);
  ret = db_ctx_exec(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete items.");
    return false;
  }

  return true;
}

/**
 * delete all selected items with given conditions.
 * @param ctx
 * @param table
 * @param j_obj
 * @return
 */
static bool delete_items_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj)
{
  int ret;
  char* sql;
  char* sql_cond;

  if((ctx == NULL) || (table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_items_by_obj. table[%s]", table);

  sql_cond = db_ctx_get_condition_str(j_obj);
  if(sql_cond == NULL) {
    return false;
  }

  asprintf(&sql, "delete from %s where %s;", table, sql_cond);
  sfree(sql_cond);
  ret = db_ctx_exec(ctx, sql);
  slog(LOG_DEBUG, "Check value. sql[%s]", sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete items.");
    return false;
  }

  return true;
}

/**
 * Get detail info of key="val" from table. get only 1 item.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
static json_t* get_detail_item_key_string(db_ctx_t* ctx, const char* table, const char* key, const char* val)
{
  int ret;
  json_t* j_res;
  char* sql;

  if((ctx == NULL) || (table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_item_key_string. table[%s], key[%s], val[%s]", table, key, val);

  asprintf(&sql, "select * from %s where %s=\"%s\";", table, key, val);
  ret = db_ctx_query(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = db_ctx_get_record(ctx);
  db_ctx_free(ctx);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;

}

/**
 * Get detail info of key="val" from table. Get all items.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
static json_t* get_detail_items_key_string(db_ctx_t* ctx, const char* table, const char* key, const char* val)
{
  int ret;
  json_t* j_res;
  json_t* j_tmp;
  char* sql;

  if((ctx == NULL) || (table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_items_key_string. table[%s], key[%s], val[%s]", table, key, val);

  asprintf(&sql, "select * from %s where %s=\"%s\";", table, key, val);
  ret = db_ctx_query(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = json_array();
  while(true) {
    j_tmp = db_ctx_get_record(ctx);
    if(j_tmp == NULL) {
      break;
    }
    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(ctx);

  return j_res;
}

/**
 * Get detail info of key="val" from table. Get all items.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
static json_t* get_detail_item_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj)
{
  int ret;
  json_t* j_res;
  char* sql;
  char* sql_cond;

  if((ctx == NULL) || (table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_items_by_obj. table[%s]", table);

  sql_cond = db_ctx_get_condition_str(j_obj);
  if(sql_cond == NULL) {
    return NULL;
  }

  asprintf(&sql, "select * from %s where %s;", table, sql_cond);
  sfree(sql_cond);
  ret = db_ctx_query(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = db_ctx_get_record(ctx);
  db_ctx_free(ctx);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Get details info of key="val" from table. Get all items.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
static json_t* get_detail_items_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj)
{
  int ret;
  json_t* j_res;
  json_t* j_tmp;
  char* sql;
  char* sql_cond;

  if((ctx == NULL) || (table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_items_by_obj. table[%s]", table);

  sql_cond = db_ctx_get_condition_str(j_obj);
  if(sql_cond == NULL) {
    slog(LOG_WARNING, "Could not get condition string.");
    return NULL;
  }

  asprintf(&sql, "select * from %s where %s;", table, sql_cond);
  sfree(sql_cond);
  ret = db_ctx_query(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = json_array();
  while(true) {
    j_tmp = db_ctx_get_record(ctx);
    if(j_tmp == NULL) {
      break;
    }
    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(ctx);

  return j_res;
}

/**
 * Get detail info of key="val" from table. Get all items.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
static json_t* get_detail_items_by_obj_order(db_ctx_t* ctx, const char* table, json_t* j_obj, const char* order)
{
  int ret;
  json_t* j_res;
  json_t* j_tmp;
  char* sql;
  char* sql_cond;

  if((ctx == NULL) || (table == NULL) || (j_obj == NULL) || (order == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_items_by_obj. table[%s]", table);

  sql_cond = db_ctx_get_condition_str(j_obj);
  if(sql_cond == NULL) {
    return NULL;
  }

  asprintf(&sql, "select * from %s where %s order by %s;", table, sql_cond, order);
  sfree(sql_cond);
  ret = db_ctx_query(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = json_array();
  while(true) {
    j_tmp = db_ctx_get_record(ctx);
    if(j_tmp == NULL) {
      break;
    }
    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(ctx);

  return j_res;
}

/**
 * @param table
 * @param item
 * @return
 */
static json_t* get_detail_items_by_condition(db_ctx_t* ctx, const char* table, const char* condition)
{
  int ret;
  json_t* j_res;
  json_t* j_tmp;
  char* sql;

  if((ctx == NULL) || (table == NULL) || (condition == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_items_by_condition. table[%s], condition[%s]", table, condition);

  asprintf(&sql, "select * from %s %s;", table, condition);
  ret = db_ctx_query(ctx, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = json_array();
  while(true) {
    j_tmp = db_ctx_get_record(ctx);
    if(j_tmp == NULL) {
      break;
    }
    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(ctx);

  return j_res;
}

bool resource_exec_mem_sql(const char* sql)
{
  int ret;

  if(sql == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_ctx_exec(g_db_memory, sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not execute sql for ast database.");
    return false;
  }

  return true;
}

bool resource_clear_mem_table(const char* table)
{
  int ret;

  if(table == NULL) {
    slog(LOG_WARNING, "Wrong input paramter.");
    return false;
  }

  ret = clear_table(g_db_memory, table);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear ast_table. table[%s]", table);
    return false;
  }

  return true;
}

/**
 *
 * @param table
 * @param item
 * @return
 */
bool resource_insert_mem_item(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired insert_ast_item. table[%s]", table);

  ret = insert_item(g_db_memory, table, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert ast info.");
    return false;
  }
  return true;
}

/**
 *
 * @param table
 * @param item
 * @return
 */
bool resource_insrep_mem_item(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired resource_insrep_ast_item. table[%s]", table);

  ret = insrep_item(g_db_memory, table, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert ast info.");
    return false;
  }
  return true;
}

/**
 * Update item info.
 * @param table
 * @param key_name
 * @param j_data
 * @return
 */
bool resource_update_mem_item(const char* table, const char* key_column, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (key_column == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ast_item. table[%s]", table);

  ret = update_item(g_db_memory, table, key_column, j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update ast info.");
    return false;
  }

  return true;
}

/**
 *
 * @param table
 * @param item
 * @return
 */
json_t* resource_get_mem_items(const char* table, const char* item)
{
  json_t* j_res;

  if((table == NULL) || (item == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_items. table[%s], item[%s]", table, item);

  j_res = get_items(g_db_memory, table, item);

  return j_res;
}

json_t* resource_get_mem_detail_items_by_condtion(const char* table, const char* condition)
{
  json_t* j_res;

  if((table == NULL) || (condition == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_items_by_condition(g_db_memory, table, condition);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * delete all selected items with string value.
 * @return
 */
bool resource_delete_mem_items_string(const char* table, const char* key, const char* val)
{
  int ret;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_ast_items_string. table[%s], key[%s], val[%s]", table, key, val);

  ret = delete_items_string(g_db_memory, table, key, val);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete items.");
    return false;
  }

  return true;
}

/**
 * Get detail info of key="val" from table. get only 1 item.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
json_t* resource_get_mem_detail_item_key_string(const char* table, const char* key, const char* val)
{
  json_t* j_res;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_detail_item_key_string. table[%s], key[%s], val[%s]", table, key, val);

  j_res = get_detail_item_key_string(g_db_memory, table, key, val);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Get detail info of key="val" from table. Get all items.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
json_t* resource_get_mem_detail_items_key_string(const char* table, const char* key, const char* val)
{
  json_t* j_res;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_items_key_string. table[%s], key[%s], val[%s]", table, key, val);

  j_res = get_detail_items_key_string(g_db_memory, table, key, val);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

bool resource_exec_file_sql(const char* sql)
{
  int ret;

  if(sql == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_ctx_exec(g_db_file, sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not execute sql for jade database.");
    return false;
  }

  return true;
}

/**
 *
 * @param table
 * @param item
 * @return
 */
bool resource_insert_file_item(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired insert_jade_item. table[%s]", table);

  ret = insert_item(g_db_file, table, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert jade info.");
    return false;
  }
  return true;
}

/**
 * insert or replace
 * @param table
 * @param item
 * @return
 */
bool resource_insrep_file_item(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired resource_insrep_jade_item. table[%s]", table);

  ret = insrep_item(g_db_file, table, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert jade info.");
    return false;
  }
  return true;
}

/**
 * Update item info.
 * @param table
 * @param key_name
 * @param j_data
 * @return
 */
bool resource_update_file_item(const char* table, const char* key_column, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (key_column == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_jade_item. table[%s]", table);

  ret = update_item(g_db_file, table, key_column, j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update jade info.");
    return false;
  }

  return true;
}

/**
 * delete all selected items with string value.
 * @return
 */
bool resource_delete_file_items_string(const char* table, const char* key, const char* val)
{
  int ret;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_jade_items_string. table[%s], key[%s], val[%s]", table, key, val);

  ret = delete_items_string(g_db_file, table, key, val);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete jade info.");
    return false;
  }

  return true;
}

/**
 * delete all selected items with string value.
 * @return
 */
bool resource_delete_file_items_by_obj(const char* table, json_t* j_obj)
{
  int ret;

  if((table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  ret = delete_items_by_obj(g_db_file, table, j_obj);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 *
 * @param table
 * @param item
 * @return
 */
json_t* resource_get_file_items(const char* table, const char* item)
{
  json_t* j_res;

  if((table == NULL) || (item == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_jade_items. table[%s], item[%s]", table, item);

  j_res = get_items(g_db_file, table, item);

  return j_res;
}

/**
 * Get detail info of key="val" from table. get only 1 item.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
json_t* resource_get_file_detail_item_key_string(const char* table, const char* key, const char* val)
{
  json_t* j_res;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_jade_detail_item_key_string. table[%s], key[%s], val[%s]", table, key, val);

  j_res = get_detail_item_key_string(g_db_file, table, key, val);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* resource_get_file_detail_item_by_obj(const char* table, json_t* j_obj)
{
  json_t* j_res;

  if((table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_item_by_obj(g_db_file, table, j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Get list detail info of key="val" from table. returns array.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
json_t* resource_get_file_detail_items_key_string(const char* table, const char* key, const char* val)
{
  json_t* j_res;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_jade_detail_items_key_string. table[%s], key[%s], val[%s]", table, key, val);

  j_res = get_detail_items_key_string(g_db_file, table, key, val);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* resource_get_file_detail_items_by_obj(const char* table, json_t* j_obj)
{
  json_t* j_res;

  if((table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_items_by_obj(g_db_file, table, j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* resource_get_file_detail_items_by_obj_order(const char* table, json_t* j_obj, const char* order)
{
  json_t* j_res;

  if((table == NULL) || (j_obj == NULL) || (order == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_items_by_obj_order(g_db_file, table, j_obj, order);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;

}

json_t* resource_get_file_detail_items_by_condtion(const char* table, const char* condition)
{
  json_t* j_res;

  if((table == NULL) || (condition == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_items_by_condition(g_db_file, table, condition);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Return the sorted json array of given data.
 * @param j_data
 * @param sort
 * @return
 */
json_t* resource_sort_json_array_string(const json_t* j_data, enum EN_SORT_TYPES type)
{
  int ret;
  int idx;
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_tmp_res;
  json_t* j_tmp_insert;
  char* sql;
  char* table_name;
  char* uuid;
  char* tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  ret = json_is_array(j_data);
  if(ret == false) {
    return NULL;
  }

  uuid = utils_gen_uuid();
  tmp = utils_string_replace_char(uuid, '-', '_');
  sfree(uuid);

  // create tmp table anem
  asprintf(&table_name, "tmp_sort_%s", tmp);
  sfree(tmp);

  // create sql
  asprintf(&sql, "create table %s (item varchar(255));", table_name);

  // create table
  ret = resource_exec_mem_sql(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not create tmp sort table.");
    sfree(table_name);
    return NULL;
  }

  // insert items
  json_array_foreach(j_data, idx, j_tmp) {
    ret = json_is_string(j_tmp);
    if(ret == false) {
      continue;
    }

    j_tmp_insert = json_pack("{s:O}",
        "item", j_tmp
        );

    resource_insert_mem_item(table_name, j_tmp_insert);
    json_decref(j_tmp_insert);
  }

  // sort
  if(type == EN_SORT_ASC) {
    asprintf(&sql, "order by item asc");
  }
  else {
    asprintf(&sql, "order by item desc");
  }

  // get sorted items
  j_tmp_res = resource_get_mem_detail_items_by_condtion(table_name, sql);
  sfree(sql);

  // delete table
  asprintf(&sql, "drop table %s", table_name);
  resource_exec_mem_sql(sql);
  sfree(sql);
  sfree(table_name);

  // create result
  j_res = json_array();
  json_array_foreach(j_tmp_res, idx, j_tmp) {

    j_tmp_insert = json_object_get(j_tmp, "item");
    json_array_append(j_res, j_tmp_insert);
  }
  json_decref(j_tmp_res);

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
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from database;");
  ret = db_ctx_query(g_db_memory, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_res_tmp = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_memory);
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
  db_ctx_free(g_db_memory);

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
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select * from database where key=\"%s\";", key);
  ret = db_ctx_query(g_db_memory, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_tmp = db_ctx_get_record(g_db_memory);
  db_ctx_free(g_db_memory);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}

/**
 * Get all core_agi's info array
 * @return
 */
json_t* get_core_agis_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items("core_agi", "*");
  return j_res;
}

/**
 * Get corresponding core_agi info.
 * @return
 */
json_t* get_core_agi_info(const char* unique_id)
{
  json_t* j_res;

  if(unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_core_agi_info. unique_id[%s]", unique_id);

  j_res = resource_get_mem_detail_item_key_string("core_agi", "unique_id", unique_id);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * create core_agi info.
 * @return
 */
bool create_core_agi_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_core_agi_info.");

  // insert item
  ret = resource_insert_mem_item("core_agi", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert core_channel.");
    return false;
  }

  // publish
  // get data info
  tmp_const = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = get_core_agi_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get core_agi info. unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_core_agi(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * update core_agi info.
 * @return
 */
bool update_core_agi_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_core_agi_info.");

  // update
  ret = resource_update_mem_item("core_agi", "unique_id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core_agi info.");
    return false;
  }

  // publish
  // get data info
  tmp_const = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = get_core_agi_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get core_agi info. unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_core_agi(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update core_agi cmd result info
 * @return
 */
bool update_core_agi_info_cmd_result_done(const char* agi_uuid, const char* cmd_id, const char* result_org)
{
  int ret;
  json_t* j_agi;
  json_t* j_cmd;
  char* timestamp;
  char* result;
  int result_code;
  char* tmp;
  char* parse;
  char* org;

  if((agi_uuid == NULL) || (cmd_id == NULL) || (result_org == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get agi info.
  j_agi = get_core_agi_info(agi_uuid);
  if(j_agi == NULL) {
    slog(LOG_ERR, "Could not get core_agi info.");
    return false;
  }

  // parse result code
  result_code = 0;
  tmp = strdup(result_org);
  org = tmp;
  parse = strsep(&tmp, " ");
  if(tmp != NULL) {
    result_code = atoi(parse);
  }
  sfree(org);

  // parse result message
  result = NULL;
  tmp = strdup(result_org);
  org = tmp;
  if(result_code == 200) {
    parse = strsep(&tmp, "=");
    if(tmp != NULL) {
      tmp[strcspn(tmp, "\n")] = '\0';
      result = strdup(tmp);
    }
  }
  else {
    // error code
    parse = strsep(&tmp, " ");
    if(tmp != NULL) {
      tmp[strcspn(tmp, "\n")] = '\0';
      result = strdup(tmp);
    }
  }
  sfree(org);

  // update cmd result
  // if there's no corresponding cmd_id info,
  // create new one.
  timestamp = utils_get_utc_timestamp();
  j_cmd = json_object_get(json_object_get(j_agi, "cmd"), cmd_id);
  if(j_cmd == NULL) {
    j_cmd = json_object();
    json_object_set_new(json_object_get(j_agi, "cmd"), cmd_id, j_cmd);
  }

  // update cmd info
  json_object_set_new(j_cmd, "status", json_string("done"));
  json_object_set_new(j_cmd, "result", json_string(result));
  json_object_set_new(j_cmd, "result_code", json_integer(result_code));
  json_object_set_new(j_cmd, "result_org", json_string(result_org));
  json_object_set_new(j_cmd, "tm_update", json_string(timestamp));
  sfree(result);

  // update agi info
  json_object_set_new(j_agi, "tm_update", json_string(timestamp));
  sfree(timestamp);

  // update info
  ret = update_core_agi_info(j_agi);
  json_decref(j_agi);
  if(ret == false) {
    return false;
  }

  return true;

}

/**
 * Add core_agi cmd info
 * @param key
 * @param cmd_id
 * @param result
 * @return
 */
bool add_core_agi_info_cmd(const char* agi_uuid, const char* cmd_uuid, const char* command, const char* dp_uuid)
{
  int ret;
  json_t* j_agi;
  json_t* j_cmd;
  char* timestamp;

  if((agi_uuid == NULL) || (cmd_uuid == NULL) || (command == NULL) || (dp_uuid == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get agi info.
  j_agi = get_core_agi_info(agi_uuid);
  if(j_agi == NULL) {
    slog(LOG_ERR, "Could not get core_agi info.");
    return false;
  }

  // update cmd result
  // if there's no corresponding cmd_id info,
  // create new one.
  timestamp = utils_get_utc_timestamp();
  j_cmd = json_pack("{"
      "s:s, s:s, s:s, s:s, s:i"
      "s:s"
      "}",

      "status",         "queued",
      "command",        command,
      "dp_uuid",        dp_uuid,
      "result",         "",
      "result_code",    -1,

      "tm_create",  timestamp
      );
  sfree(timestamp);

  json_object_set_new(json_object_get(j_agi, "cmd"), cmd_uuid, j_cmd);

  // update info
  ret = update_core_agi_info(j_agi);
  json_decref(j_agi);
  if(ret == false) {
    return false;
  }

  return true;

}


/**
 * delete core_agi info.
 * @return
 */
bool delete_core_agi_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_core_agi_info.");

  // get data info
  j_tmp = get_core_agi_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get core_channel info. unique_id[%s]", key);
    return false;
  }

  ret = resource_delete_mem_items_string("core_agi", "unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete core_agi info. unique_id[%s]", key);
    return false;
  }

  // publish
  // publish event
  ret = publication_publish_event_core_agi(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Get all agent's id array
 * @return
 */
json_t* get_agent_agents_all_id(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  slog(LOG_DEBUG, "Fired get_agents_all_id.");

  asprintf(&sql, "select * from agent;");
  ret = db_ctx_query(g_db_memory, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct agent info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_memory);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s}",
        "id",  json_string_value(json_object_get(j_tmp, "id"))
        );
    json_decref(j_tmp);
    if(j_res_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_res_tmp);
  }
  db_ctx_free(g_db_memory);

  return j_res;
}

/**
 * Get all agents array
 * @return
 */
json_t* get_agent_agents_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_agents_all.");

  j_res = resource_get_mem_items("agent", "*");

  return j_res;
}


/**
 * Get corresponding agent detail info.
 * @return
 */
json_t* get_agent_agent_info(const char* id)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_agent_info. id[%s]", id);

  asprintf(&sql, "select * from agent where id=\"%s\";", id);
  ret = db_ctx_query(g_db_memory, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct agent info.");
    return NULL;
  }

  j_tmp = db_ctx_get_record(g_db_memory);
  db_ctx_free(g_db_memory);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}

/**
 * Get all system's all id array
 * @return
 */
json_t* get_core_systems_all_id(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_systems_all_id.");

  j_res = resource_get_mem_items("system", "id");

  return j_res;
}

/**
 * Get all system's array
 * @return
 */
json_t* get_core_systems_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_systems_all.");

  j_res = resource_get_mem_items("system", "*");

  return j_res;
}


/**
 * Get corresponding system detail info.
 * @return
 */
json_t* get_core_system_info(const char* id)
{
  json_t* j_res;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_system_info. id[%s]", id);

  j_res = resource_get_mem_detail_item_key_string("system", "id", id);

  return j_res;
}


/**
 * Get all device_state's all device array
 * @return
 */
json_t* get_device_states_all_device(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_device_states_all_device.");

  j_res = resource_get_mem_items("device_state", "device");

  return j_res;
}

/**
 * Get corresponding device_state detail info.
 * @return
 */
json_t* get_device_state_info(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_device_state_info. device[%s]", name);

  j_res = resource_get_mem_detail_item_key_string("device_state", "device", name);

  return j_res;
}









/**
 * create voicemail user
 * @param j_tmp
 * @return
 */
bool create_voicemail_user_info(json_t* j_tmp)
{
  int ret;

  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_voicemail_user_info.");

  ret = resource_insert_mem_item("voicemail_user", j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create voicemail_user.");
    return false;
  }

  return true;
}

/**
 * Get corresponding voicemail_user info.
 * @param name
 * @return
 */
json_t* get_voicemail_user_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_voicemail_user_info. id[%s]", key);

  j_res = resource_get_mem_detail_item_key_string("voicemail_user", "id", key);
  if(j_res == NULL) {
    return NULL;
  }
  return j_res;
}

/**
 * Get all voicemail_user info.
 * @return
 */
json_t* get_voicemail_users_all()
{
  json_t* j_res;
  slog(LOG_DEBUG, "Fired get_voicemail_users_all.");

  j_res = resource_get_mem_items("voicemail_user", "*");

  return j_res;
}

/**
 * Insert data into core_module
 * @param j_tmp
 * @return
 */
int create_core_module(json_t* j_tmp)
{
  int ret;

  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_insert_mem_item("core_module", j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert core_module.");
    return false;
  }

  return true;
}

/**
 * Get all core_modules info.
 * @return
 */
json_t* get_core_modules_all(void)
{
  json_t* j_res;
  slog(LOG_DEBUG, "Fired get_core_modules_all.");

  j_res = resource_get_mem_items("core_module", "*");

  return j_res;
}

/**
 * Get given core_module info.
 * @return
 */
json_t* get_core_module_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_core_module_info.");

  j_res = resource_get_mem_detail_item_key_string("core_module", "name", key);

  return j_res;
}

/**
 * Update core module info.
 * @param j_data
 * @return
 */
bool update_core_module_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* module_name;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_core_module_info.");

  ret = resource_update_mem_item("core_module", "name", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update core module info.");
    return false;
  }

  module_name = json_string_value(json_object_get(j_data, "name"));
  if(module_name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    return false;
  }

  j_tmp = resource_get_mem_detail_item_key_string("core_module", "name", module_name);

  ret = publication_publish_event_core_module(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Create agent agent info.
 * @param j_data
 * @return
 */
bool create_agent_agent_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_agent_agent_info.");

  // insert queue info
  ret = resource_insert_mem_item("agent", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert agent agent.");
    return false;
  }

  return true;
}

/**
 * Update agent agent info.
 * @param j_data
 * @return
 */
bool update_agent_agent_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_agent_agent_info.");

  ret = resource_update_mem_item("agent", "id", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update agent agent info.");
    return false;
  }

  return true;
}

/**
 * delete agent agent info.
 * @return
 */
bool delete_agent_agent_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_agent_agent_info. id[%s]", key);

  ret = resource_delete_mem_items_string("agent", "id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete agent agent info. id[%s]", key);
    return false;
  }

  return true;
}

