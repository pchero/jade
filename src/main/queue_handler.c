/*
 * queue_handler.c
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <string.h>

#include "slog.h"
#include "http_handler.h"
#include "utils.h"
#include "conf_handler.h"
#include "ami_handler.h"
#include "publication_handler.h"

#include "queue_handler.h"
#include "resource_handler.h"

#define DEF_QUEUE_CONFNAME      "queues.conf"

#define DEF_JADE_QUEUE_CONFNAME_QUEUE   "jade.queues.conf"




#define DEF_SETTING_SECTION "global"

#define DEF_DB_TABLE_QUEUE_PARAM    "queue_param"
#define DEF_DB_TABLE_QUEUE_MEMBER   "queue_member"
#define DEF_DB_TABLE_QUEUE_ENTRY    "queue_entry"



static bool init_databases(void);
static bool init_database_param(void);
static bool init_database_member(void);
static bool init_database_entry(void);

static bool init_configs(void);

static bool db_create_param_info(const json_t* j_data);

static bool db_delete_entry_info(const char* key);
static bool db_create_entry_info(const json_t* j_data);

static bool db_create_member_info(const json_t* j_data);
static bool db_update_member_info(const json_t* j_data);
static bool db_delete_member_info(const char* key);

static bool cfg_create_queue_info(const char* name, const json_t* j_data);
static bool cfg_update_queue_info(const char* name, const json_t* j_data);
static bool cfg_delete_queue_info(const char* name);

static bool clear_queue_entry(void);
static bool clear_queue_member(void);
static bool clear_queue_param(void);

static bool is_setting_section(const char* section);

bool queue_init_handler(void)
{
  int ret;
  json_t* j_tmp;

  slog(LOG_DEBUG, "Fired init_queue_handler.");

  // databases
  ret = init_databases();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate databases.");
    return false;
  }

  // configs
  ret = init_configs();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate configs.");
    return false;
  }

  // queue status
  j_tmp = json_pack("{s:s}",
      "Action", "QueueStatus"
      );
  ret = ami_send_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "QueueStatus");
    return false;
  }

  return true;
}

bool queue_term_handler(void)
{
  int ret;

  ret = clear_queue_param();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear queue_param info.");
    return false;
  }

  ret = clear_queue_entry();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear queue_entry info.");
    return false;
  }

  ret = clear_queue_member();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear queue_member info.");
    return false;
  }

  return true;
}

bool queue_reload_handler(void)
{
  int ret;

  ret = queue_term_handler();
  if(ret == false) {
    return false;
  }

  ret = queue_init_handler();
  if(ret == false) {
    return false;
  }

  return true;
}

static bool init_databases(void)
{
  int ret;

  ret = init_database_param();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate param database.");
    return false;
  }

  ret = init_database_member();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate member database.");
    return false;
  }

  ret = init_database_entry();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate entry database.");
    return false;
  }

  return true;
}

static bool init_database_param(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_QUEUE_PARAM;

  create_table =
    "create table " DEF_DB_TABLE_QUEUE_PARAM " ("

    // identity
    "   name             varchar(255),"    // queue name.

    // status
    "   max              int,"             // max available calls in the queue.
    "   strategy         varchar(255),"    // queue strategy.
    "   calls            int,"             // waiting call count.
    "   hold_time        int,"             // average waiting time.
    "   talk_time        int,"             // average talk time.
    "   completed        int,"             // distributed call count.
    "   abandoned        int,"             // could not distributed call count.

    // performance
    "   service_level       int,"                 // service level interval time sec.
    "   service_level_perf  real  default 0.0,"   // service level performance(%). completed_call / (abandoned_call + completed_call) * 100

    // etc
    "   weight          int,"    // queue priority.

    // timestamp. UTC."
    "   tm_update       datetime(6),"   // update time."

    "   primary key(name)"

    ");";

  // execute
  resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_QUEUE_PARAM);
    return false;
  }

  return true;
}

static bool init_database_member(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_QUEUE_MEMBER;

  create_table =
    "create table " DEF_DB_TABLE_QUEUE_MEMBER " ("

    // identity
    "   id             varchar(255),"   // member id(name@queue)
    "   queue_name     varchar(255),"   // queue name
    "   name           varchar(255),"   // member name

    "   location          varchar(255),"          // location
    "   state_interface   varchar(255),"          // state interface
    "   membership        varchar(255),"  // membership
    "   penalty           int,"           // penalty
    "   calls_taken       int,"           // call taken count.
    "   last_call         int,"
    "   last_pause        int,"
    "   in_call           int,"
    "   status            int,"
    "   paused            int,"
    "   paused_reason     text,"
    "   ring_inuse        int,"

    // timestamp. UTC."
    "   tm_update         datetime(6),"   // update time."

    "   primary key(queue_name, name)"

    ");";

  // execute
  resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_QUEUE_MEMBER);
    return false;
  }

  return true;
}

static bool init_database_entry(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_QUEUE_ENTRY;

  create_table =
    "create table " DEF_DB_TABLE_QUEUE_ENTRY " ("

    // identity
    "   unique_id         varchar(255),"
    "   queue_name        varchar(255),"
    "   channel           varchar(255),"

    // info
    "   position            int,"
    "   caller_id_num       varchar(255),"
    "   caller_id_name      varchar(255),"
    "   connected_line_num  varchar(255),"
    "   connected_line_name varchar(255),"

    "   wait                int,"

    // timestamp. UTC."
    "   tm_update         datetime(6),"   // update time."

    "   primary key(unique_id)"

    ");";

  // execute
  resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_QUEUE_ENTRY);
    return false;
  }

  return true;
}

static bool init_configs(void)
{
  int ret;

  // queue
  ret = conf_add_external_config_file(DEF_QUEUE_CONFNAME, DEF_JADE_QUEUE_CONFNAME_QUEUE);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate queue config.");
    return false;
  }

  return true;
}

static bool db_create_entry_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert item
  ret = resource_insert_mem_item("queue_entry", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_entry.");
    return false;
  }

  return true;
}

static bool db_delete_entry_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // delete
  ret = resource_delete_mem_items_string("queue_entry", "unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
    return false;
  }

  return true;
}

static bool db_create_member_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // create member info
  ret = resource_insert_mem_item("queue_member", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return false;
  }

  return true;
}

static bool db_update_member_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // update
  ret = resource_update_mem_item("queue_member", "id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue_member info.");
    return false;
  }

  return true;
}

static bool db_delete_member_info(const char* key)
{
  int ret;

  if(key == false) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // delete
  ret = resource_delete_mem_items_string("queue_member", "id", key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue member info.");
    return false;
  }

  return true;
}

static bool cfg_create_queue_info(const char* name, const json_t* j_data)
{
  int ret;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_create_queue_info. name[%s]", name);

  ret = conf_create_ast_section(DEF_JADE_QUEUE_CONFNAME_QUEUE, name, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create config for queue.");
    return false;
  }

  return true;
}

static bool cfg_update_queue_info(const char* name, const json_t* j_data)
{
  int ret;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_update_queue_info. name[%s]", name);

  ret = conf_update_ast_section(DEF_JADE_QUEUE_CONFNAME_QUEUE, name, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update config for queue. name[%s]", name);
    return false;
  }

  return true;
}

static bool cfg_delete_queue_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_update_queue_info. name[%s]", name);

  ret = conf_delete_ast_section(DEF_JADE_QUEUE_CONFNAME_QUEUE, name);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update config for queue. name[%s]", name);
    return false;
  }

  return true;
}


/**
 * Create cfg queue info.
 * @param j_data
 * @return
 */
bool queue_cfg_create_queue_info(const json_t* j_data)
{
  const char* name;
  json_t* j_tmp;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get info
  name = json_string_value(json_object_get(j_data, "name"));
  j_tmp = json_object_get(j_data, "data");
  if((name == NULL) || (j_tmp == NULL)) {
    slog(LOG_NOTICE, "Could not get name or data.");
    return false;
  }

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given queue name is setting section. name[%s]", name);
    return false;
  }

  // create
  ret = cfg_create_queue_info(name, j_tmp);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not get create cfg queue info.");
    return false;
  }

  return true;
}

/**
 * Update cfg queue info.
 * @param j_data
 * @return
 */
bool queue_cfg_update_queue_info(const json_t* j_data)
{
  int ret;
  const char* name;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_queue_info.");

  // get info
  name = json_string_value(json_object_get(j_data, "name"));
  j_tmp = json_object_get(j_data, "data");
  if((name == NULL) || (j_tmp == NULL)) {
    slog(LOG_NOTICE, "Could not get name or data info.");
    return false;
  }

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given queue name is setting section. name[%s]", name);
    return false;
  }

  // update queue info
  ret = cfg_update_queue_info(name, j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue info.");
    return false;
  }

  return true;
}

/**
 * Delete cfg queue info.
 * @param j_data
 * @return
 */
bool queue_cfg_delete_queue_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_queue_info.");

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given queue name is setting section. name[%s]", name);
    return false;
  }

  // delete queue info
  ret = cfg_delete_queue_info(name);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue info.");
    return false;
  }

  return true;
}

/**
 * Return true if given section is global setting section
 * @param context
 * @return
 */
static bool is_setting_section(const char* section)
{
  int ret;

  if(section == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = strcmp(section, DEF_SETTING_SECTION);
  if(ret == 0) {
    return true;
  }

  return false;
}


/**
 * Get all queue_param array
 * @return
 */
json_t* queue_get_queue_params_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items("queue_param", "*");
  return j_res;
}

/**
 * Get corresponding queue param info.
 * @return
 */
json_t* queue_get_queue_param_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_param_info.");

  j_res = resource_get_mem_detail_item_key_string("queue_param", "name", key);

  return j_res;
}

static bool clear_queue_entry(void)
{
  int ret;

  ret = resource_clear_mem_table("queue_entry");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_entry");
    return false;
  }

  return true;
}

static bool db_create_param_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert queue info
  ret = resource_insert_mem_item("queue_param", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_param.");
    return false;
  }

  return true;

}

/**
 * create queue info.
 * @return
 */
bool queue_create_param_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert queue info
  ret = db_create_param_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_param.");
    return false;
  }

  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  j_tmp = queue_get_queue_param_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue info. name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_queue_queue(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * create queue member info.
 * @return
 */
bool queue_create_member_info(const json_t* j_data)
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
  ret = db_create_member_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return false;
  }

  // get data
  id = json_string_value(json_object_get(j_data, "id"));
  j_tmp = queue_get_member_info(id);

  // publish event
  ret = publication_publish_event_queue_member(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish update event.");
    return false;
  }

  return true;
}

/**
 * Get queue_entry array of given queue name
 * @return
 */
json_t* queue_get_entries_all_by_queuename(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_entries_all_by_queuename. name[%s]", name);

  // get items
  j_res = resource_get_mem_detail_items_key_string("queue_entry", "queue_name", name);

  return j_res;
}

/**
 * Get detail info of given queue_entry key.
 * @param name
 * @return
 */
json_t* queue_get_entry_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_entry_info.");

  j_res = resource_get_mem_detail_item_key_string("queue_entry", "unique_id", key);

  return j_res;
}

/**
 * Get all queue members array
 * @return
 */
json_t* queue_get_members_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items("queue_member", "*");
  return j_res;
}

/**
 * Get queue_member array of given queue name
 * @return
 */
json_t* queue_get_members_all_by_queuename(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_members_all_by_queuename. name[%s]", name);

  // get items
  j_res = resource_get_mem_detail_items_key_string("queue_member", "queue_name", name);

  return j_res;
}

/**
 * Get corresponding queue member info.
 * @return
 */
json_t* queue_get_member_info(const char* id)
{
  json_t* j_tmp;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_queue_member_info. id[%s]", id);

  // get queue member
  j_tmp = resource_get_mem_detail_item_key_string("queue_member", "id", id);
  if(j_tmp == NULL) {
    return NULL;
  }

  return j_tmp;
}

/**
 * delete queue entry info.
 * @return
 */
bool queue_delete_entry_info(const char* key)
{
  int ret;
  json_t* j_data;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_queue_entry_info. key[%s]", key);

  // get data
  j_data = resource_get_mem_detail_item_key_string("queue_entry", "unique_id", key);
  if(j_data == NULL) {
    slog(LOG_NOTICE, "The queue_entry info is already deleted. unique_id[%s]", key);
    return true;
  }

  // delete
  ret = db_delete_entry_info(key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
    json_decref(j_data);
    return false;
  }

  // publish event
  ret = publication_publish_event_queue_entry(DEF_PUB_TYPE_DELETE, j_data);
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
bool queue_create_entry_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired queue_create_entry_info.");

  ret = db_create_entry_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create entry.");
    return false;
  }

  // publish
  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = queue_get_entry_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue entry info. unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_queue_entry(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Get all queue_entry array
 * @return
 */
json_t* queue_get_entries_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items("queue_entry", "*");
  return j_res;
}

static bool clear_queue_member(void)
{
  int ret;

  ret = resource_clear_mem_table("queue_member");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_member");
    return false;
  }

  return true;
}

bool queue_update_member_info(const json_t* j_data)
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
  ret = db_update_member_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue_member info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "id"));
  j_tmp = queue_get_member_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get queue_member info. id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_queue_member(DEF_PUB_TYPE_UPDATE, j_tmp);
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
bool queue_delete_member_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_queue_member_info.");

  // get member info
  j_tmp = resource_get_mem_detail_item_key_string("queue_member", "id", key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "The key is already deleted.");
    return true;
  }

  // delete
  ret = db_delete_member_info(key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue member info.");
    json_decref(j_tmp);
    return false;
  }

  // publish
  ret = publication_publish_event_queue_member(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish delete event.");
    return false;
  }

  return true;
}

static bool clear_queue_param(void)
{
  int ret;

  ret = resource_clear_mem_table("queue_param");
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_param");
    return false;
  }

  return true;
}





