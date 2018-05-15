
#define _GNU_SOURCE

#include <stdio.h>
#include <jansson.h>

#include "slog.h"
#include "resource_handler.h"
#include "http_handler.h"
#include "ami_action_handler.h"
#include "utils.h"
#include "call_handler.h"
#include "publication_handler.h"

#include "core_handler.h"

#define DEF_DB_TABLE_CALL_CHANNEL "channel"
#define DEF_DB_TABLE_MODULE "core_module"
#define DEF_DB_TABLE_SYSTEM "core_system"

static bool init_databases(void);
static bool init_database_channel(void);
static bool init_database_module(void);
static bool init_database_system(void);

static bool init_resources(void);

static bool db_create_channel_info(const json_t* j_data);
static bool db_update_channel_info(const json_t* j_data);
static bool db_delete_channel_info(const char* key);

static bool db_create_module_info(const json_t* j_data);
static bool db_update_module_info(const json_t* j_data);

static bool db_create_system_info(const json_t* j_data);


bool core_init_handler(void)
{
  int ret;

  ret = init_databases();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database.");
    return false;
  }

  ret = init_resources();
  if(ret == false) {
    slog(LOG_ERR, "Coudl not initiate resources.");
    return false;
  }

  return true;
}

bool core_term_handler(void)
{
  return true;
}

bool core_reload_handler(void)
{
  int ret;

  ret = core_term_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not terminate call_handler.");
    return false;
  }

  ret = core_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate call_handler.");
    return false;
  }

  return true;
}


static bool init_databases(void)
{
  int ret;

  slog(LOG_DEBUG, "Fired init_databases.");

  ret = init_database_channel();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate channel database.");
    return false;
  }

  ret = init_database_module();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate module database.");
    return false;
  }

  ret = init_database_system();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate system database.");
    return false;
  }

  return true;
}

static bool init_database_channel(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_CALL_CHANNEL ";";
  create_table =
      "create table " DEF_DB_TABLE_CALL_CHANNEL "("

      // identity
      "   unique_id   varchar(255) not null,"
      "   linked_id   varchar(255),"

      // channel info
      "   channel             varchar(255),"    ///< channel name
      "   channel_state       int,"
      "   channel_state_desc  varchar(255),"

      // dial info
      "   caller_id_num       varchar(255),"
      "   caller_id_name      varchar(255),"

      "   connected_line_num  varchar(255),"
      "   connected_line_name varchar(255),"

      "   language            varchar(255),"
      "   account_code        varchar(255),"

      // dialplan
      "   context     varchar(255),"
      "   exten       varchar(255),"
      "   priority    varchar(255),"

      "   application       varchar(255),"
      "   application_data  varchar(1023),"
      "   bridge_id         varchar(255),"

      // hangup
      "   hangup_cause        int,"
      "   hangup_cause_desc   varchar(255),"

      // variables
      "   variables   text,"

      // other
      "   duration  int,"

      // timestamp. UTC."
      "   tm_update     datetime(6),"

      "   primary key(unique_id)"
      ");";


  // execute
  ret = resource_exec_mem_sql(drop_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_CALL_CHANNEL);
    return false;
  }

  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_CALL_CHANNEL);
    return false;
  }

  return true;
}

static bool init_database_module(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_MODULE ";";
  create_table =
      "create table " DEF_DB_TABLE_MODULE "("

      // identity
      "   name      varchar(255) not null,"

      // info
      "   size      int,"
      "   load      varchar(255),"
      "   version   varchar(255),"

      // timestamp. UTC."
      "   tm_update     datetime(6),"

      "   primary key(name)"
      ");";


  // execute
  ret = resource_exec_mem_sql(drop_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_MODULE);
    return false;
  }

  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_MODULE);
    return false;
  }

  return true;
}

static bool init_database_system(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_SYSTEM ";";
  create_table =
      "create table " DEF_DB_TABLE_SYSTEM "("

      // identity
      "   id varchar(255),"

      // version info
      "   ami_version   varchar(255),"
      "   ast_version   varchar(255),"
      "   system_name   varchar(255),"

      //  time info
      "   startup_date  varchar(255),"
      "   startup_time  varchar(255),"
      "   reload_date   varchar(255),"
      "   reload_time   varchar(255),"
      "   current_calls int,"

      // max info
      "   max_calls         int,"
      "   max_load_avg      real,"
      "   max_file_handles  int,"

      "   run_user          varchar(255),"
      "   run_group         varchar(255),"

      "   real_time_enabled varchar(255),"
      "   cdr_enabled       varchar(255),"
      "   http_enabled      varchar(255),"

      // timestamp. UTC."
      "   tm_update         datetime(6),"   // update time."

      "   primary key(id)"
      ");";


  // execute
  ret = resource_exec_mem_sql(drop_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_SYSTEM);
    return false;
  }

  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_SYSTEM);
    return false;
  }

  return true;
}

static bool init_resources(void)
{
  int ret;
  json_t* j_tmp;

  // create initial system info
  j_tmp = json_pack("{s:s}", "id", "1");
  ret = db_create_system_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}


static bool db_create_channel_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // insert item
  ret = resource_insert_mem_item(DEF_DB_TABLE_CALL_CHANNEL, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert core_channel.");
    return false;
  }

  return true;
}

static bool db_update_channel_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_update_mem_item(DEF_DB_TABLE_CALL_CHANNEL, "unique_id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core_channel info.");
    return false;
  }

  return true;
}

static bool db_delete_channel_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_delete_mem_items_string(DEF_DB_TABLE_CALL_CHANNEL, "unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
    return false;
  }

  return true;
}

static bool db_create_module_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // insert item
  ret = resource_insert_mem_item(DEF_DB_TABLE_MODULE, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create info module.");
    return false;
  }

  return true;
}

static bool db_update_module_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_update_mem_item(DEF_DB_TABLE_MODULE, "name", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update core module info.");
    return false;
  }

  return true;
}

static bool db_create_system_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // insert item
  ret = resource_insert_mem_item(DEF_DB_TABLE_SYSTEM, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create info module.");
    return false;
  }

  return true;
}

static bool db_update_system_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // insert item
  ret = resource_update_mem_item(DEF_DB_TABLE_SYSTEM, "id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update info module.");
    return false;
  }

  return true;
}


/**
 * Get all channel's array
 * @return
 */
json_t* core_get_channels_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items(DEF_DB_TABLE_CALL_CHANNEL, "*");
  return j_res;
}

/**
 * Returns all of channels info belongs to the given device_name.
 * Beware, the device_name should be like an uuid.
 * @param devicename
 * @return
 */
json_t* core_get_channels_by_devicename(const char* device_name)
{
  json_t* j_res;
  char* condition;

  if(device_name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired call_get_channels_by_devicename. devicename[%s]", device_name);

  asprintf(&condition, "where channel like '%%%s%%'", device_name);

  j_res = resource_get_mem_detail_items_by_condtion(DEF_DB_TABLE_CALL_CHANNEL, condition);
  sfree(condition);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get chat rooms info. user_uuid[%s]", device_name);
    return NULL;
  }

  return j_res;
}

/**
 * Get corresponding channel info.
 * @return
 */
json_t* core_get_channel_info(const char* unique_id)
{
  json_t* j_res;

  if(unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_channel_info. unique_id[%s]", unique_id);

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_CALL_CHANNEL, "uniquie_id", unique_id);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * create core_channel info.
 * @return
 */
bool core_create_channel_info(const json_t* j_data)
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
  ret = db_create_channel_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert core_channel.");
    return false;
  }

  // publish
  // get data info
  tmp_const = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = core_get_channel_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get core_channel info. unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_core_channel(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * update channel info.
 * @return
 */
int core_update_channel_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* key;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // update
  ret = db_update_channel_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core_channel info.");
    return false;
  }

  // get updated info
  key = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = core_get_channel_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get channel info. unique_id[%s]", key);
    return false;
  }

  // publish event
  ret = publication_publish_event_core_channel(DEF_PUB_TYPE_UPDATE, j_tmp);
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
int core_delete_channel_info(const char* key)
{
  json_t* j_tmp;
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = core_get_channel_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "The channel is already deleted. key[%s]", key);
    return false;
  }

  ret = db_delete_channel_info(key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish delete event.
  ret = publication_publish_event_core_channel(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Get all core_modules info.
 * @return
 */
json_t* core_get_modules_all(void)
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
json_t* core_get_module_info(const char* key)
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
 * Insert data into core_module
 * @param j_tmp
 * @return
 */
bool core_create_module(json_t* j_tmp)
{
  int ret;

  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_create_module_info(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert core_module.");
    return false;
  }

  return true;
}

/**
 * Update core module info.
 * @param j_data
 * @return
 */
bool core_update_module_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* module_name;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_core_module_info.");

  ret = db_update_module_info(j_data);
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
 * Get all system's array
 * @return
 */
json_t* core_get_systems_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_systems_all.");

  j_res = resource_get_mem_items(DEF_DB_TABLE_SYSTEM, "*");

  return j_res;
}

/**
 * Get corresponding system detail info.
 * @return
 */
json_t* core_get_system_info(const char* id)
{
  json_t* j_res;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_system_info. id[%s]", id);

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_SYSTEM, "id", id);

  return j_res;
}

bool core_create_system_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_create_system_info(j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

bool core_update_system_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_update_system_info(j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

bool core_module_unload(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = ami_action_moduleload(name, "unload");
  if(ret == false) {
    return false;
  }

  return true;
}

bool core_module_load(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = ami_action_moduleload(name, "load");
  if(ret == false) {
    return false;
  }

  return true;
}

bool core_module_reload(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = ami_action_moduleload(name, "reload");
  if(ret == false) {
    return false;
  }

  return true;
}

