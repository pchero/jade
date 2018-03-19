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
#include "publish_handler.h"

#include "resource_handler.h"


extern app* g_app;

db_ctx_t* g_db_ast;
db_ctx_t* g_db_jade;

// db
static bool clear_table(db_ctx_t* ctx, const char* table);
static bool insert_item(db_ctx_t* ctx, const char* table, const json_t* j_data);
static bool update_item(db_ctx_t* ctx, const char* table, const char* key_column, const json_t* j_data);
static json_t* get_items(db_ctx_t* ctx, const char* table, const char* item);
static json_t* get_detail_item_key_string(db_ctx_t* ctx, const char* table, const char* key, const char* val);
static json_t* get_detail_items_key_string(db_ctx_t* ctx, const char* table, const char* key, const char* val);
static bool delete_items_string(db_ctx_t* ctx, const char* table, const char* key, const char* val);
static json_t* get_detail_item_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj);
static json_t* get_detail_items_by_obj(db_ctx_t* ctx, const char* table, json_t* j_obj);
static json_t* get_detail_items_by_obj_order(db_ctx_t* ctx, const char* table, json_t* j_obj, const char* order);


// db_ast

// db_jade


static bool init_db(void);
static bool init_ast_database(void);
static bool init_jade_database(void);


/**
 * Initiate resource.
 * @return
 */
bool init_resource_handler(void)
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

  ret = init_jade_database();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate jade database.");
    return false;
  }

  return true;
}

void term_resource_handler(void)
{
  slog(LOG_DEBUG, "Fired term_resource_handler.");

  db_ctx_term(g_db_ast);
  db_ctx_term(g_db_jade);
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

  g_db_ast = db_ctx_init(tmp_const);
  if(g_db_ast == NULL) {
    slog(LOG_WARNING, "Could not initiate db_ast.");
    return false;
  }

  // init database_jade
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "database_name_jade"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get database_name_jade info.");
    return false;
  }

  g_db_jade = db_ctx_init(tmp_const);
  if(g_db_jade == NULL) {
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
  db_ctx_exec(g_db_ast, g_sql_drop_action);
  ret = db_ctx_exec(g_db_ast, g_sql_create_action);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "action");
    return false;
  }

  // system
  db_ctx_exec(g_db_ast, g_sql_drop_system);
  ret = db_ctx_exec(g_db_ast, g_sql_create_system);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "system");
    return false;
  }

  // core_agi
  db_ctx_exec(g_db_ast, g_sql_drop_core_agi);
  ret = db_ctx_exec(g_db_ast, g_sql_create_core_agi);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "core_agi");
    return false;
  }

  // core_module
  db_ctx_exec(g_db_ast, g_sql_drop_core_module);
  ret = db_ctx_exec(g_db_ast, g_sql_create_core_module);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "core_module");
    return false;
  }

  // core_channel
  db_ctx_exec(g_db_ast, g_sql_drop_channel);
  ret = db_ctx_exec(g_db_ast, g_sql_create_channel);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "channel");
    return false;
  }

  // queue_param
  db_ctx_exec(g_db_ast, g_sql_drop_queue_param);
  ret = db_ctx_exec(g_db_ast, g_sql_create_queue_param);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_param");
    return false;
  }

  // queue_member
  db_ctx_exec(g_db_ast, g_sql_drop_queue_member);
  ret = db_ctx_exec(g_db_ast, g_sql_create_queue_member);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_member");
    return false;
  }

  // queue_member
  db_ctx_exec(g_db_ast, g_sql_drop_queue_entry);
  ret = db_ctx_exec(g_db_ast, g_sql_create_queue_entry);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_entry");
    return false;
  }

  // database
  db_ctx_exec(g_db_ast, g_sql_drop_database);
  ret = db_ctx_exec(g_db_ast, g_sql_create_database);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "database");
    return false;
  }

  // agent
  db_ctx_exec(g_db_ast, g_sql_drop_agent);
  ret = db_ctx_exec(g_db_ast, g_sql_create_agent);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "agent");
    return false;
  }

  // device_state
  db_ctx_exec(g_db_ast, g_sql_drop_device_state);
  ret = db_ctx_exec(g_db_ast, g_sql_create_device_state);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "device_state");
    return false;
  }

  // parking_lot
  db_ctx_exec(g_db_ast, g_sql_drop_parking_lot);
  ret = db_ctx_exec(g_db_ast, g_sql_create_parking_lot);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "parking_lot");
    return false;
  }

  // parked_call
  db_ctx_exec(g_db_ast, g_sql_drop_parked_call);
  ret = db_ctx_exec(g_db_ast, g_sql_create_parked_call);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "parked_call");
    return false;
  }

  // voicemail_user
  db_ctx_exec(g_db_ast, g_sql_drop_voicemail_user);
  ret = db_ctx_exec(g_db_ast, g_sql_create_voicemail_user);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "voicemail_user");
    return false;
  }

  return true;
}

/**
 * Initialize jade database.
 * @return Success: true\n
 * Failure: false
 */
static bool init_jade_database(void)
{
  int ret;

  // dp_dpma
//  db_ctx_exec(g_db_jade, g_sql_drop_dp_dpma);
  ret = db_ctx_exec(g_db_jade, g_sql_create_dp_dpma);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "dp_dpma");
    return false;
  }

  // dp_dialplan
//  db_ctx_exec(g_db_jade, g_sql_drop_dp_dialplan);
  ret = db_ctx_exec(g_db_jade, g_sql_create_dp_dialplan);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "dp_dialplan");
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
static bool insert_item(db_ctx_t* ctx, const char* table, const json_t* j_data)
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
  db_ctx_free(g_db_ast);

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

  sql_cond = db_ctx_get_select_str(j_obj);
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

  sql_cond = db_ctx_get_select_str(j_obj);
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

  sql_cond = db_ctx_get_select_str(j_obj);
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

bool exec_ast_sql(const char* sql)
{
  int ret;

  if(sql == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_ctx_exec(g_db_ast, sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not execute sql for ast database.");
    return false;
  }

  return true;
}

bool clear_ast_table(const char* table)
{
  int ret;

  if(table == NULL) {
    slog(LOG_WARNING, "Wrong input paramter.");
    return false;
  }

  ret = clear_table(g_db_ast, table);
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
bool insert_ast_item(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired insert_ast_item. table[%s]", table);

  ret = insert_item(g_db_ast, table, j_data);
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
bool update_ast_item(const char* table, const char* key_column, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (key_column == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ast_item. table[%s]", table);

  ret = update_item(g_db_ast, table, key_column, j_data);
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
json_t* get_ast_items(const char* table, const char* item)
{
  json_t* j_res;

  if((table == NULL) || (item == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_items. table[%s], item[%s]", table, item);

  j_res = get_items(g_db_ast, table, item);

  return j_res;
}

/**
 * delete all selected items with string value.
 * @return
 */
bool delete_ast_items_string(const char* table, const char* key, const char* val)
{
  int ret;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_ast_items_string. table[%s], key[%s], val[%s]", table, key, val);

  ret = delete_items_string(g_db_ast, table, key, val);
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
json_t* get_ast_detail_item_key_string(const char* table, const char* key, const char* val)
{
  json_t* j_res;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_detail_item_key_string. table[%s], key[%s], val[%s]", table, key, val);

  j_res = get_detail_item_key_string(g_db_ast, table, key, val);
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
json_t* get_ast_detail_items_key_string(const char* table, const char* key, const char* val)
{
  json_t* j_res;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_items_key_string. table[%s], key[%s], val[%s]", table, key, val);

  j_res = get_detail_items_key_string(g_db_ast, table, key, val);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

bool exec_jade_sql(const char* sql)
{
  int ret;

  if(sql == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_ctx_exec(g_db_jade, sql);
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
bool insert_jade_item(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired insert_jade_item. table[%s]", table);

  ret = insert_item(g_db_jade, table, j_data);
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
bool update_jade_item(const char* table, const char* key_column, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (key_column == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_jade_item. table[%s]", table);

  ret = update_item(g_db_jade, table, key_column, j_data);
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
bool delete_jade_items_string(const char* table, const char* key, const char* val)
{
  int ret;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_jade_items_string. table[%s], key[%s], val[%s]", table, key, val);

  ret = delete_items_string(g_db_jade, table, key, val);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete jade info.");
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
json_t* get_jade_items(const char* table, const char* item)
{
  json_t* j_res;

  if((table == NULL) || (item == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_jade_items. table[%s], item[%s]", table, item);

  j_res = get_items(g_db_jade, table, item);

  return j_res;
}

/**
 * Get detail info of key="val" from table. get only 1 item.
 * "select * from <table> where <key>=<val>;"
 * @param table
 * @param item
 * @return
 */
json_t* get_jade_detail_item_key_string(const char* table, const char* key, const char* val)
{
  json_t* j_res;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_jade_detail_item_key_string. table[%s], key[%s], val[%s]", table, key, val);

  j_res = get_detail_item_key_string(g_db_jade, table, key, val);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* get_jade_detail_item_by_obj(const char* table, json_t* j_obj)
{
  json_t* j_res;

  if((table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_item_by_obj(g_db_jade, table, j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* get_jade_detail_items_by_obj(const char* table, json_t* j_obj)
{
  json_t* j_res;

  if((table == NULL) || (j_obj == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_items_by_obj(g_db_jade, table, j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* get_jade_detail_items_by_obj_order(const char* table, json_t* j_obj, const char* order)
{
  json_t* j_res;

  if((table == NULL) || (j_obj == NULL) || (order == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_detail_items_by_obj_order(g_db_jade, table, j_obj, order);
  if(j_res == NULL) {
    return NULL;
  }

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
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_res_tmp = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
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
  db_ctx_free(g_db_ast);

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
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_tmp = db_ctx_get_record(g_db_ast);
  db_ctx_free(g_db_ast);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}

bool clear_queue_param(void)
{
  int ret;

  ret = clear_ast_table("queue_param");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_param");
    return false;
  }

  return true;
}

bool clear_queue_member(void)
{
  int ret;

  ret = clear_ast_table("queue_member");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_member");
    return false;
  }

  return true;
}

bool clear_queue_entry(void)
{
  int ret;

  ret = clear_ast_table("queue_entry");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_entry");
    return false;
  }

  return true;
}

/**
 * Get all queue_param array
 * @return
 */
json_t* get_queue_params_all_name(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select name from queue_param;");
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct queue param info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s}",
        "name", json_string_value(json_object_get(j_tmp, "name"))
        );
    json_decref(j_tmp);
    if(j_res_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_res_tmp);
  }
  db_ctx_free(g_db_ast);

  return j_res;
}

/**
 * create queue info.
 * @return
 */
int create_queue_param_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert queue info
  ret = insert_ast_item("queue_param", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_param.");
    return false;
  }

  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  j_tmp = get_queue_param_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue info. name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_queue_queue(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete queue info.
 * @return
 */
int delete_queue_param_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = delete_ast_items_string("queue_param", "name", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete queue info. name[%s]", key);
    return false;
  }

  return true;
}

/**
 * Get all queue_param array
 * @return
 */
json_t* get_queue_params_all(void)
{
  json_t* j_res;

  j_res = get_ast_items("queue_param", "*");
  return j_res;
}

/**
 * Get corresponding queue param info.
 * @return
 */
json_t* get_queue_param_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_param_info.");

  j_res = get_ast_detail_item_key_string("queue_param", "name", key);

  return j_res;
}

/**
 * Get all registry account array
 * @return
 */
json_t* get_queue_members_all_name_queue(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select name, queue_name from queue_member;");
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct queue member info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s, s:s}",
        "name",       json_string_value(json_object_get(j_tmp, "name"))? : "",
        "queue_name", json_string_value(json_object_get(j_tmp, "queue_name"))? : ""
        );
    json_decref(j_tmp);
    if(j_res_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_res_tmp);
  }
  db_ctx_free(g_db_ast);

  return j_res;
}

/**
 * create queue member info.
 * @return
 */
bool create_queue_member_info(const json_t* j_data)
{
  int ret;
  const char* id;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_queue_member_info.");

  // create member info
  ret = insert_ast_item("queue_member", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return false;
  }

  // get data
  id = json_string_value(json_object_get(j_data, "id"));
  j_tmp = get_queue_member_info(id);

  // publish event
  ret = publish_event_queue_member(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish update event.");
    return false;
  }

  return true;
}

bool update_queue_member_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_queue_member_info.");

  // update
  ret = update_ast_item("queue_member", "id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue_member info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "id"));
  j_tmp = get_queue_member_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue_member info. id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_queue_member(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete queue member info.
 * @return
 */
bool delete_queue_member_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_queue_member_info.");

  // get member info
  j_tmp = get_ast_detail_item_key_string("queue_member", "id", key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "The key is already deleted.");
    return true;
  }

  // delete
  ret = delete_ast_items_string("queue_member", "id", key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue member info.");
    json_decref(j_tmp);
    return false;
  }

  // publish
  ret = publish_event_queue_member(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish delete event.");
    return false;
  }

  return true;
}

/**
 * Get all queue members array
 * @return
 */
json_t* get_queue_members_all(void)
{
  json_t* j_res;

  j_res = get_ast_items("queue_member", "*");
  return j_res;
}

/**
 * Get queue_member array of given queue name
 * @return
 */
json_t* get_queue_members_all_by_queuename(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_members_all_by_queuename. name[%s]", name);

  // get items
  j_res = get_ast_detail_items_key_string("queue_member", "queue_name", name);

  return j_res;
}

/**
 * Get corresponding queue member info.
 * @return
 */
json_t* get_queue_member_info(const char* id)
{
  json_t* j_tmp;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_member_info. id[%s]", id);

  // get queue member
  j_tmp = get_ast_detail_item_key_string("queue_member", "id", id);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}

/**
 * Get all queue entries array
 * @return
 */
json_t* get_queue_entries_all_unique_id_queue_name(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from queue_entry;");
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct queue entries info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s, s:s}",
        "unique_id",  json_string_value(json_object_get(j_tmp, "unique_id")),
        "queue_name", json_string_value(json_object_get(j_tmp, "queue_name"))
        );
    json_decref(j_tmp);
    if(j_res_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_res_tmp);
  }
  db_ctx_free(g_db_ast);

  return j_res;
}

/**
 * Get all queue_entry array
 * @return
 */
json_t* get_queue_entries_all(void)
{
  json_t* j_res;

  j_res = get_ast_items("queue_entry", "*");
  return j_res;
}

/**
 * Get queue_entry array of given queue name
 * @return
 */
json_t* get_queue_entries_all_by_queuename(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_entries_all_by_queuename. name[%s]", name);

  // get items
  j_res = get_ast_detail_items_key_string("queue_entry", "queue_name", name);

  return j_res;
}


/**
 * Get detail info of given queue_entry key.
 * @param name
 * @return
 */
json_t* get_queue_entry_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_entry_info.");

  j_res = get_ast_detail_item_key_string("queue_entry", "unique_id", key);

  return j_res;
}


/**
 * Get corresponding queue entry info.
 * @return
 */
json_t* get_queue_entry_info_by_id_name(const char* unique_id, const char* queue_name)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if((unique_id == NULL) || (queue_name == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_entry_info. unique_id[%s], queue_name[%s]", unique_id, queue_name);

  asprintf(&sql, "select * from queue_entry where unique_id=\"%s\" and queue_name=\"%s\";", unique_id, queue_name);
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct queue_entry info.");
    return NULL;
  }

  j_tmp = db_ctx_get_record(g_db_ast);
  db_ctx_free(g_db_ast);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}

/**
 * delete queue info.
 * @return
 */
int delete_queue_entry_info(const char* key)
{
  int ret;
  json_t* j_data;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_queue_entry_info. key[%s]", key);

  // get data
  j_data = get_ast_detail_item_key_string("queue_entry", "unique_id", key);
  if(j_data == NULL) {
    slog(LOG_NOTICE, "The queue_entry info is already deleted. unique_id[%s]", key);
    return true;
  }

  // delete
  ret = delete_ast_items_string("queue_entry", "unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
    json_decref(j_data);
    return false;
  }

  // publish event
  ret = publish_event_queue_entry(DEF_PUB_TYPE_DELETE, j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }
  return true;
}

/**
 * create queue info.
 * @return
 */
int create_queue_entry_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_queue_entry_info.");

  // insert item
  ret = insert_ast_item("queue_entry", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_entry.");
    return false;
  }

  // publish
  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = get_queue_entry_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue entry info. unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_queue_entry(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Get all queues status info.
 * @return
 */
json_t* get_queue_statuses_all(void)
{
  json_t* j_res;
  json_t* j_queues;
  json_t* j_queue;
  json_t* j_entries;
  json_t* j_members;
  json_t* j_tmp;
  int idx;
  const char* name;

  j_res = json_array();

  // get all queue info
  j_queues = get_queue_params_all();
  json_array_foreach(j_queues, idx, j_queue) {
    j_tmp = json_object();

    // get queue name
    name = json_string_value(json_object_get(j_queue, "name"));
    if(name == NULL) {
      slog(LOG_ERR, "Could not get queue name info.");
      continue;
    }

    // set queue info
    json_object_set(j_tmp, "queue", j_queue);

    // get and set entries info
    j_entries = get_queue_entries_all_by_queuename(name);
    json_object_set_new(j_tmp, "entries", j_entries);

    // get and set members info
    j_members = get_queue_members_all_by_queuename(name);
    json_object_set_new(j_tmp, "members", j_members);

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_queues);

  return j_res;
}

/**
 * Get detail queues status info.
 * @return
 */
json_t* get_queue_status_info(const char* name)
{
  json_t* j_res;
  json_t* j_tmp;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = json_object();

  // get and set queue info
  j_tmp = get_queue_param_info(name);
  json_object_set_new(j_res, "queue", j_tmp);

  // get and set entries info
  j_tmp = get_queue_entries_all_by_queuename(name);
  json_object_set_new(j_res, "entries", j_tmp);

  // get and set members info
  j_tmp = get_queue_members_all_by_queuename(name);
  json_object_set_new(j_res, "members", j_tmp);

  return j_res;
}

/**
 * create core_channel info.
 * @return
 */
bool create_core_channel_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_core_channel_info.");

  // insert item
  ret = insert_ast_item("channel", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert core_channel.");
    return false;
  }

  // publish
  // get data info
  tmp_const = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = get_core_channel_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get core_channel info. unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_core_channel(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Get all channel's unique id array
 * @return
 */
json_t* get_core_channels_all_unique_id(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from channel;");
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);

  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct channel info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s}",
        "unique_id",  json_string_value(json_object_get(j_tmp, "unique_id"))
        );
    json_decref(j_tmp);
    if(j_res_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_res_tmp);
  }
  db_ctx_free(g_db_ast);

  return j_res;
}

/**
 * Get all channel's array
 * @return
 */
json_t* get_core_channels_all(void)
{
  json_t* j_res;

  j_res = get_ast_items("channel", "*");
  return j_res;
}

/**
 * Get corresponding channel info.
 * @return
 */
json_t* get_core_channel_info(const char* unique_id)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if(unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_channel_info. unique_id[%s]", unique_id);

  asprintf(&sql, "select * from channel where unique_id=\"%s\";", unique_id);
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct channel info.");
    return NULL;
  }

  j_tmp = db_ctx_get_record(g_db_ast);
  db_ctx_free(g_db_ast);
  if(j_tmp == NULL) {
    return NULL;
  }
  return j_tmp;
}

/**
 * update channel info.
 * @return
 */
int update_core_channel_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* key;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // update
  ret = update_ast_item("channel", "unique_id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core_channel info.");
    return false;
  }

  // get updated info
  key = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = get_core_channel_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get channel info. unique_id[%s]", key);
    return false;
  }

  // publish event
  ret = publish_event_core_channel(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete channel info.
 * @return
 */
int delete_core_channel_info(const char* key)
{
  json_t* j_tmp;
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = get_core_channel_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "The channel is already deleted. key[%s]", key);
    return false;
  }

  ret = delete_ast_items_string("channel", "unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish delete event.
  ret = publish_event_core_channel(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Get all core_agi's info array
 * @return
 */
json_t* get_core_agis_all(void)
{
  json_t* j_res;

  j_res = get_ast_items("core_agi", "*");
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

  j_res = get_ast_detail_item_key_string("core_agi", "unique_id", unique_id);
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
  ret = insert_ast_item("core_agi", j_data);
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
  ret = publish_event_core_agi(DEF_PUB_TYPE_CREATE, j_tmp);
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
  ret = update_ast_item("core_agi", "unique_id", j_data);
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
  ret = publish_event_core_agi(DEF_PUB_TYPE_UPDATE, j_tmp);
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
  timestamp = get_utc_timestamp();
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
bool add_core_agi_info_cmd(const char* agi_uuid, const char* cmd_uuid, const char* command, const char* dp_uuid
    )
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
  timestamp = get_utc_timestamp();
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

  ret = delete_ast_items_string("core_agi", "unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete core_agi info. unique_id[%s]", key);
    return false;
  }

  // publish
  // publish event
  ret = publish_event_core_agi(DEF_PUB_TYPE_DELETE, j_tmp);
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
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct agent info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
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
  db_ctx_free(g_db_ast);

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

  j_res = get_ast_items("agent", "*");

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
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct agent info.");
    return NULL;
  }

  j_tmp = db_ctx_get_record(g_db_ast);
  db_ctx_free(g_db_ast);
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

  j_res = get_ast_items("system", "id");

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

  j_res = get_ast_items("system", "*");

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

  j_res = get_ast_detail_item_key_string("system", "id", id);

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

  j_res = get_ast_items("device_state", "device");

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

  j_res = get_ast_detail_item_key_string("device_state", "device", name);

  return j_res;
}

bool clear_park_parkinglot(void)
{
  int ret;

  ret = clear_ast_table("parking_lot");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear park_parkinglot");
    return false;
  }

  return true;
}

/**
 * Get all parking_lot's array
 * @return
 */
json_t* get_park_parkinglots_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_park_parkinglots_all.");

  j_res = get_ast_items("parking_lot", "*");

  return j_res;
}

/**
 * Get all parking_lot's all name array
 * @return
 */
json_t* get_park_parkinglots_all_name(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_park_parkinglots_all_name.");

  j_res = get_ast_items("parking_lot", "name");

  return j_res;
}

/**
 * Get corresponding parking_lot detail info.
 * @return
 */
json_t* get_park_parkinglot_info(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_park_parkinglot_info. name[%s]", name);

  j_res = get_ast_detail_item_key_string("parking_lot", "name", name);

  return j_res;
}

/**
 * Create park_parkinglot detail info.
 * @return
 */
int create_park_parkinglot_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* tmp_const;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_park_parkinglot_info.");

  ret = insert_ast_item("parking_lot", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parking_lot.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  j_tmp = get_park_parkinglot_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get park parkinglot info. name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_park_parkinglot(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

bool clear_park_parkedcall(void)
{
  int ret;

  ret = clear_ast_table("parked_call");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear park_parkedcall.");
    return false;
  }

  return true;
}

/**
 * Get all parking_lot's all parkee_unique_id array
 * @return
 */
json_t* get_park_parkedcalls_all_parkee_unique_id(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_park_parkedcalls_all_parkee_unique_id.");

  j_res = get_ast_items("parked_call", "parkee_unique_id");

  return j_res;
}

/**
 * Get all parkedcalls info array
 * @return
 */
json_t* get_park_parkedcalls_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_park_parkedcalls_all.");

  j_res = get_ast_items("parked_call", "*");

  return j_res;
}


/**
 * Get corresponding parked call detail info.
 * @return
 */
json_t* get_park_parkedcall_info(const char* parkee_unique_id)
{
  json_t* j_res;

  if(parkee_unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_parked_call_info. name[%s]", parkee_unique_id);

  j_res = get_ast_detail_item_key_string("parked_call", "parkee_unique_id", parkee_unique_id);

  return j_res;
}

/**
 * Create parked call detail info.
 * @return
 */
bool create_park_parkedcall_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = insert_ast_item("parked_call", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parked_call.");
    return false;
  }

  // publish
  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "parkee_unique_id"));
  j_tmp = get_park_parkedcall_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get park parkedcall info. parkee_unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_park_parkedcall(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }


  return true;
}

/**
 * Update parked call detail info.
 * @return
 */
bool update_park_parkedcall_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_park_parkedcall_info.");

  ret = update_ast_item("parked_call", "parkee_unique_id", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update park parked_call info.");
    return false;
  }

  // publish
  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "parkee_unique_id"));
  j_tmp = get_park_parkedcall_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get park parkedcall info. parkee_unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_park_parkedcall(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}


/**
 * Delete parked call detail info.
 * @return
 */
bool delete_park_parkedcall_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = get_park_parkedcall_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "The channel is already deleted. key[%s]", key);
    return false;
  }

  ret = delete_ast_items_string("parked_call", "parkee_unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete park parkedcall info. unique_id[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish delete event.
  ret = publish_event_park_parkedcall(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
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

  ret = insert_ast_item("voicemail_user", j_tmp);
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

  j_res = get_ast_detail_item_key_string("voicemail_user", "id", key);
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

  j_res = get_ast_items("voicemail_user", "*");

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

  ret = insert_ast_item("core_module", j_tmp);
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

  j_res = get_ast_items("core_module", "*");

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

  j_res = get_ast_detail_item_key_string("core_module", "name", key);

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

  ret = update_ast_item("core_module", "name", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update core module info.");
    return false;
  }

  module_name = json_string_value(json_object_get(j_data, "name"));
  if(module_name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    return false;
  }

  j_tmp = get_ast_detail_item_key_string("core_module", "name", module_name);

  ret = publish_event_core_module(DEF_PUB_TYPE_UPDATE, j_tmp);
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
  ret = insert_ast_item("agent", j_data);
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

  ret = update_ast_item("agent", "id", j_data);
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

  ret = delete_ast_items_string("agent", "id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete agent agent info. id[%s]", key);
    return false;
  }

  return true;
}

/**
 * Get all dp_dpma array
 * @return
 */
json_t* get_dp_dpmas_all(void)
{
  json_t* j_res;

  j_res = get_jade_items("dp_dpma", "*");
  return j_res;
}

/**
 * Get corresponding dp_dpma detail info.
 * @return
 */
json_t* get_dp_dpma_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_dp_dpma_info. key[%s]", key);

  j_res = get_jade_detail_item_key_string("dp_dpma", "uuid", key);

  return j_res;
}


/**
 * Create dp dpma info.
 * @param j_data
 * @return
 */
bool create_dp_dpma_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_dp_dpma_info.");

  // insert info
  ret = insert_jade_item("dp_dpma", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dpma info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = get_dp_dpma_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dpma info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_dp_dpma(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update dp_dpma info.
 * @param j_data
 * @return
 */
bool update_dp_dpma_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_dp_dpma_info.");

  // update
  ret = update_jade_item("dp_dpma", "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dp_dpma info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = get_dp_dpma_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dpma info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_dp_dpma(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Delete dp_dpma info.
 * @param j_data
 * @return
 */
bool delete_dp_dpma_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_dp_dpma_info. key[%s]", key);

  // get info
  j_tmp = get_dp_dpma_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dpma info. uuid[%s]", key);
    return false;
  }

  ret = delete_jade_items_string("dp_dpma", "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete dp_dpma info. key[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publish_event_dp_dpma(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Get all dp_dialplan array
 * @return
 */
json_t* get_dp_dialplans_all(void)
{
  json_t* j_res;

  j_res = get_jade_items("dp_dialplan", "*");
  return j_res;
}

/**
 * Get all dp_dialplans by dpma_uuid order by sequence
 * @return
 */
json_t* get_dp_dialplans_by_dpma_uuid_order_sequence(const char* dpma_uuid)
{
  json_t* j_res;
  json_t* j_obj;

  if(dpma_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_obj = json_pack("{s:s}", "dpma_uuid", dpma_uuid);

  j_res = get_jade_detail_items_by_obj_order("dp_dialplan", j_obj, "sequence");
  json_decref(j_obj);

  return j_res;
}

/**
 * Get corresponding dp_dialplan detail info.
 * @return
 */
json_t* get_dp_dialplan_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_dp_dialplan_info. key[%s]", key);

  j_res = get_jade_detail_item_key_string("dp_dialplan", "uuid", key);

  return j_res;
}

/**
 * Get corresponding dp_dialplan detail info.
 * @return
 */
json_t* get_dp_dialplan_info_by_dpma_seq(const char* dpma_uuid, int seq)
{
  json_t* j_res;
  json_t* j_obj;

  if(dpma_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_dp_dialplan_info_by_dpma_seq. dpma_uuid[%s], sequence[%d]", dpma_uuid, seq);

  j_obj = json_pack("{s:s, s:i}",
      "dpma_uuid",  dpma_uuid,
      "sequence",   seq
      );

  j_res = get_jade_detail_item_by_obj("dp_dialplan", j_obj);
  json_decref(j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}


/**
 * Create dp_dialplan info.
 * @param j_data
 * @return
 */
bool create_dp_dialplan_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* tmp_const;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_dp_dialplan_info.");

  // insert info
  ret = insert_jade_item("dp_dialplan", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dialplan contact.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = get_dp_dialplan_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dialplan info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_dp_dialplan(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update dp_dialplan info.
 * @param j_data
 * @return
 */
bool update_dp_dialplan_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_dp_dialplan_info.");

  // update
  ret = update_jade_item("dp_dialplan", "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dp_dialplan info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = get_dp_dialplan_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dialplan info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publish_event_dp_dialplan(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Delete dp_dialplan info.
 * @param j_data
 * @return
 */
bool delete_dp_dialplan_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_dp_dialplan_info. key[%s]", key);

  // get info
  j_tmp = get_dp_dialplan_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dialplan info. uuid[%s]", key);
    return false;
  }

  ret = delete_jade_items_string("dp_dialplan", "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete dp_dialplan info. key[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publish_event_dp_dialplan(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

