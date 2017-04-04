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
  db_res_t* db_res;
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
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct databases name info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
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
  db_free(db_res);

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
  db_res_t* db_res;
  json_t* j_res;
  char* sql;

  if((table == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_detail_item_key_string. table[%s], key[%s], val[%s]", table, key, val);

  asprintf(&sql, "select * from %s where %s=\"%s\";", table, key, val);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get detail info.");
    return NULL;
  }

  j_res = db_get_record(db_res);
  db_free(db_res);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;

}


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

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
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
  db_free(db_res);

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

/**
 * Get all registry account array
 * @return
 */
json_t* get_queue_params_all_name(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select name from queue_param;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct queue param info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
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
  db_free(db_res);

  return j_res;
}

/**
 * Get corresponding queue param info.
 * @return
 */
json_t* get_queue_param_info(const char* name)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_param_info. name[%s]", name);

  asprintf(&sql, "select * from queue_param where name=\"%s\";", name);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct queue_param info.");
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
json_t* get_queue_members_all_name_queue(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select name, queue_name from queue_member;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct queue member info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
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
  db_free(db_res);

  return j_res;
}

/**
 * Get corresponding queue member info.
 * @return
 */
json_t* get_queue_member_info(const char* name, const char* queue_name)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;

  if((name == NULL) || (queue_name == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_member_info. name[%s], queue_name[%s]", name, queue_name);

  asprintf(&sql, "select * from queue_member where name=\"%s\" and queue_name=\"%s\";", name, queue_name);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct queue_member info.");
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
 * Get all queue entries array
 * @return
 */
json_t* get_queue_entries_all_unique_id_queue_name(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from queue_entry;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct queue entries info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
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
  db_free(db_res);

  return j_res;
}

/**
 * Get corresponding queue entry info.
 * @return
 */
json_t* get_queue_entry_info(const char* unique_id, const char* queue_name)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;

  if((unique_id == NULL) || (queue_name == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_entry_info. unique_id[%s], queue_name[%s]", unique_id, queue_name);

  asprintf(&sql, "select * from queue_entry where unique_id=\"%s\" and queue_name=\"%s\";", unique_id, queue_name);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct queue_entry info.");
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
 * Get all channel's unique id array
 * @return
 */
json_t* get_channels_all_unique_id(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  asprintf(&sql, "select * from channel;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct channel info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
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
  db_free(db_res);

  return j_res;
}

/**
 * Get corresponding channel info.
 * @return
 */
json_t* get_channel_info(const char* unique_id)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;

  if(unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_channel_info. unique_id[%s]", unique_id);

  asprintf(&sql, "select * from channel where unique_id=\"%s\";", unique_id);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct channel info.");
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
 * Get all agent's id array
 * @return
 */
json_t* get_agents_all_id(void)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_tmp;

  slog(LOG_DEBUG, "Fired get_agents_all_id.");

  asprintf(&sql, "select * from agent;");
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct agent info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_get_record(db_res);
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
  db_free(db_res);

  return j_res;
}

/**
 * Get corresponding agent detail info.
 * @return
 */
json_t* get_agent_info(const char* id)
{
  char* sql;
  db_res_t* db_res;
  json_t* j_tmp;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_agent_info. id[%s]", id);

  asprintf(&sql, "select * from agent where id=\"%s\";", id);
  db_res = db_query(sql);
  sfree(sql);
  if(db_res == NULL) {
    slog(LOG_WARNING, "Could not get correct agent info.");
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
json_t* get_device_state_info(const char* device)
{
  json_t* j_res;

  if(device == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_device_state_info. device[%s]", device);

  j_res = get_detail_item_key_string("device_state", "device", device);

  return j_res;
}


