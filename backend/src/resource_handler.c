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
#include "resource_handler.h"

static json_t* get_items(const char* table, const char* item);
static json_t* get_detail_item_key_string(const char* table, const char* key, const char* val);


/**
 *
 * @param table
 * @param item
 * @return
 */
static json_t* get_items(const char* table, const char* item)
{
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;
  json_t* j_val;
  char* sql;

  if((table == NULL) || (item == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_items. table[%s], item[%s]", table, item);

  asprintf(&sql, "select %s from %s;", item, table);
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
    if(j_tmp == NULL) {
      break;
    }

    j_val = json_object_get(j_tmp, item);
    if(j_val == NULL) {
      json_decref(j_tmp);
      continue;
    }

    j_res_tmp = json_pack("{s:O}", item, j_val);
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
 * Get detail info of key="val" from table.
 * @param table
 * @param item
 * @return
 */
static json_t* get_detail_item_key_string(const char* table, const char* key, const char* val)
{
  int ret;
  json_t* j_res;
  char* sql;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_item_key_string. table[%s], key[%s], val[%s]", table, key, val);

  asprintf(&sql, "select * from %s where %s=\"%s\";", table, key, val);
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = db_ctx_get_record(g_db_ast);
  db_ctx_free(g_db_ast);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;

}


/**
 * Get all peers array
 * @return
 */
json_t* get_peers_all_peer(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_peers_all_peer.");

  j_res = get_items("peer", "peer");

  return j_res;
}

/**
 * Get given peer's detail info.
 * @return
 */
json_t* get_peer_detail(const char* peer)
{
  json_t* j_res;

  if(peer == NULL) {
    slog(LOG_WARNING, "Wrong parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_peer_detail.");

  j_res = get_detail_item_key_string("peer", "peer", peer);
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

/**
 * Get all registry account array
 * @return
 */
json_t* get_registries_all_account(void)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from registry;");
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct registry account info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ast);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s}",
        "account", json_string_value(json_object_get(j_tmp, "account"))
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
 * Get corresponding registry info.
 * @return
 */
json_t* get_registry_info(const char* account)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if(account == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select * from registry where account=\"%s\";", account);
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct registry account info.");
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
 * Get all registry account array
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
 * Get corresponding queue param info.
 * @return
 */
json_t* get_queue_param_info(const char* name)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_param_info. name[%s]", name);

  asprintf(&sql, "select * from queue_param where name=\"%s\";", name);
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct queue_param info.");
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
 * Get corresponding queue member info.
 * @return
 */
json_t* get_queue_member_info(const char* name, const char* queue_name)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if((name == NULL) || (queue_name == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_member_info. name[%s], queue_name[%s]", name, queue_name);

  asprintf(&sql, "select * from queue_member where name=\"%s\" and queue_name=\"%s\";", name, queue_name);
  ret = db_ctx_query(g_db_ast, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not get correct queue_member info.");
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
 * Get corresponding queue entry info.
 * @return
 */
json_t* get_queue_entry_info(const char* unique_id, const char* queue_name)
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
 * Get all channel's unique id array
 * @return
 */
json_t* get_channels_all_unique_id(void)
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
 * Get corresponding channel info.
 * @return
 */
json_t* get_channel_info(const char* unique_id)
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
 * Get all agent's id array
 * @return
 */
json_t* get_agents_all_id(void)
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
 * Get corresponding agent detail info.
 * @return
 */
json_t* get_agent_info(const char* id)
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
json_t* get_systems_all_id(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_systems_all_id.");

  j_res = get_items("system", "id");

  return j_res;
}

/**
 * Get corresponding system detail info.
 * @return
 */
json_t* get_system_info(const char* id)
{
  json_t* j_res;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_system_info. id[%s]", id);

  j_res = get_detail_item_key_string("system", "id", id);

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

  j_res = get_items("device_state", "device");

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

  j_res = get_detail_item_key_string("device_state", "device", name);

  return j_res;
}

/**
 * Get all parking_lot's all name array
 * @return
 */
json_t* get_parking_lots_all_name(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_parking_lots_all_name.");

  j_res = get_items("parking_lot", "name");

  return j_res;
}

/**
 * Get corresponding parking_lot detail info.
 * @return
 */
json_t* get_parking_lot_info(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_parking_lot_info. name[%s]", name);

  j_res = get_detail_item_key_string("parking_lot", "name", name);

  return j_res;
}

/**
 * Get all parking_lot's all parkee_unique_id array
 * @return
 */
json_t* get_parked_calls_all_parkee_unique_id(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_parked_calls_all_unique_id.");

  j_res = get_items("parked_call", "parkee_unique_id");

  return j_res;
}

/**
 * Get corresponding parking_lot detail info.
 * @return
 */
json_t* get_parked_call_info(const char* parkee_unique_id)
{
  json_t* j_res;

  if(parkee_unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_parked_call_info. name[%s]", parkee_unique_id);

  j_res = get_detail_item_key_string("parked_call", "parkee_unique_id", parkee_unique_id);

  return j_res;
}


