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
#include "ami_action_handler.h"
#include "publication_handler.h"
#include "conf_handler.h"

#include "queue_handler.h"
#include "resource_handler.h"

#define DEF_QUEUE_CONFNAME      "queues.conf"

#define DEF_JADE_QUEUE_CONFNAME_QUEUE   "jade.queues.conf"

#define DEF_SETTING_SECTION "global"

#define DEF_DB_TABLE_QUEUE_PARAM    "queue_param"
#define DEF_DB_TABLE_QUEUE_MEMBER   "queue_member"
#define DEF_DB_TABLE_QUEUE_ENTRY    "queue_entry"

static struct st_callback* g_callback_db_entry;
static struct st_callback* g_callback_db_member;


static bool init_databases(void);
static bool init_database_param(void);
static bool init_database_member(void);
static bool init_database_entry(void);

static bool init_configs(void);

static bool init_callbacks(void);
static bool term_callbacks(void);


static bool db_create_param_info(const json_t* j_data);

static bool db_delete_entry_info(const char* key);
static bool db_create_entry_info(const json_t* j_data);

static bool db_create_member_info(const json_t* j_data);
static bool db_update_member_info(const json_t* j_data);
static bool db_delete_member_info(const char* key);

static void execute_callbacks_db_entry(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);
static void execute_callbacks_db_member(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);


static bool cfg_create_queue_info(const json_t* j_data);
static bool cfg_update_queue_info(const json_t* j_data);
static bool cfg_delete_queue_info(const char* name);

static bool clear_queue_entry(void);
static bool clear_queue_member(void);
static bool clear_queue_param(void);

static bool is_setting_section(const char* section);

static bool send_request_member_penalty_update(const json_t* j_data);
static bool send_request_member_paused_update(const json_t* j_data);
static bool send_request_member_add_to_queue(const json_t* j_data);


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

  // callbacks
  ret = init_callbacks();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate callback.");
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

  ret = term_callbacks();
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

static bool init_callbacks(void)
{
  g_callback_db_entry = utils_create_callback();
  g_callback_db_member = utils_create_callback();

  return true;
}

static bool term_callbacks(void)
{
  utils_terminate_callback(g_callback_db_entry);
  utils_terminate_callback(g_callback_db_member);

  return true;
}

static bool db_create_entry_info(const json_t* j_data)
{
  int ret;
  const char* key;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert item
  ret = resource_insert_mem_item(DEF_DB_TABLE_QUEUE_ENTRY, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_entry.");
    return false;
  }

  // get key
  key = json_string_value(json_object_get(j_data, "unique_id"));
  if(key == NULL) {
    slog(LOG_NOTICE, "Could not get key.");
    return false;
  }

  // get created info
  j_tmp = queue_get_entry_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get created info.");
    return false;
  }

  // execute callback
  execute_callbacks_db_entry(EN_RESOURCE_CREATE, j_tmp);
  json_decref(j_tmp);

  return true;
}

static bool db_delete_entry_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get delete info
  j_tmp = queue_get_entry_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get delete info.");
    return false;
  }

  // delete
  ret = resource_delete_mem_items_string(DEF_DB_TABLE_QUEUE_ENTRY, "unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  execute_callbacks_db_entry(EN_RESOURCE_DELETE, j_tmp);
  json_decref(j_tmp);

  return true;
}

static bool db_create_member_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* key;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // create member info
  ret = resource_insert_mem_item(DEF_DB_TABLE_QUEUE_MEMBER, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return false;
  }

  // get key
  key = json_string_value(json_object_get(j_data, "id"));
  if(key == false) {
    slog(LOG_NOTICE, "Could not get key info.");
    return false;
  }

  // get info
  j_tmp = queue_get_member_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get created info.");
    return false;
  }

  // execute callback
  execute_callbacks_db_member(EN_RESOURCE_CREATE, j_tmp);
  json_decref(j_tmp);

  return true;
}

static bool db_update_member_info(const json_t* j_data)
{
  int ret;
  const char* key;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // update
  ret = resource_update_mem_item(DEF_DB_TABLE_QUEUE_MEMBER, "id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update queue_member info.");
    return false;
  }

  // get key
  key = json_string_value(json_object_get(j_data, "id"));
  if(key == false) {
    slog(LOG_NOTICE, "Could not get key info.");
    return false;
  }

  // get info
  j_tmp = queue_get_member_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get created info.");
    return false;
  }

  // execute callback
  execute_callbacks_db_member(EN_RESOURCE_UPDATE, j_tmp);
  json_decref(j_tmp);

  return true;
}

static bool db_delete_member_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == false) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get delete info
  j_tmp = queue_get_member_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get delete info.");
    return false;
  }

  // delete
  ret = resource_delete_mem_items_string(DEF_DB_TABLE_QUEUE_MEMBER, "id", key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue member info.");
    json_decref(j_tmp);
    return false;
  }

  // execute callback
  execute_callbacks_db_member(EN_RESOURCE_DELETE, j_tmp);
  json_decref(j_tmp);

  return true;
}

static bool cfg_create_queue_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_create_queue_info.");

  ret = conf_create_ast_section(DEF_JADE_QUEUE_CONFNAME_QUEUE, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create config for queue.");
    return false;
  }

  return true;
}

static bool cfg_update_queue_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_update_queue_info.");

  ret = conf_update_ast_section(DEF_JADE_QUEUE_CONFNAME_QUEUE, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update config for queue.");
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
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get info
  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
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
  ret = cfg_create_queue_info(j_data);
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

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_queue_info.");

  // get info
  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_NOTICE, "Could not get name info.");
    return false;
  }

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given queue name is setting section. name[%s]", name);
    return false;
  }

  // update queue info
  ret = cfg_update_queue_info(j_data);
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

  j_res = resource_get_mem_items(DEF_DB_TABLE_QUEUE_PARAM, "*");
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

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_QUEUE_PARAM, "name", key);

  return j_res;
}

static bool clear_queue_entry(void)
{
  int ret;

  ret = resource_clear_mem_table(DEF_DB_TABLE_QUEUE_ENTRY);
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
  ret = resource_insert_mem_item(DEF_DB_TABLE_QUEUE_PARAM, j_data);
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

  return true;
}

/**
 * create queue member info.
 * @return
 */
bool queue_create_member_info(const json_t* j_data)
{
  int ret;

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
  j_res = resource_get_mem_detail_items_key_string(DEF_DB_TABLE_QUEUE_ENTRY, "queue_name", name);

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

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_QUEUE_ENTRY, "unique_id", key);

  return j_res;
}

/**
 * Get all queue members array
 * @return
 */
json_t* queue_get_members_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items(DEF_DB_TABLE_QUEUE_MEMBER, "*");
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
  j_res = resource_get_mem_detail_items_key_string(DEF_DB_TABLE_QUEUE_MEMBER, "queue_name", name);

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
  j_tmp = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_QUEUE_MEMBER, "id", id);
  if(j_tmp == NULL) {
    return NULL;
  }

  return j_tmp;
}

static bool send_request_member_penalty_update(const json_t* j_data)
{
  int ret;
  const char* queue_name;
  const char* interface;
  int penalty_number;
  char penalty_string[64];

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  queue_name = json_string_value(json_object_get(j_data, "queue_name"));
  interface = json_string_value(json_object_get(j_data, "state_interface"));
  penalty_number = json_integer_value(json_object_get(j_data, "penalty"));

  snprintf(penalty_string, sizeof(penalty_string) - 1, "%d", penalty_number);

  // send penalty request
  ret = ami_action_queuepenalty(queue_name, interface, penalty_string);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not send penalty request.");
    return false;
  }

  return true;
}

static bool send_request_member_paused_update(const json_t* j_data)
{
  int ret;
  const char* queue_name;
  const char* interface;
  const char* reason;
  int paused_number;
  char paused_string[64];

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  queue_name = json_string_value(json_object_get(j_data, "queue_name"));
  interface = json_string_value(json_object_get(j_data, "state_interface"));
  reason = json_string_value(json_object_get(j_data, "paused_reason"));
  paused_number = json_integer_value(json_object_get(j_data, "paused"));
  snprintf(paused_string, sizeof(paused_string), "%d", paused_number);

  // send paused request
  ret = ami_action_queuepause(queue_name, interface, paused_string, reason);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not send paused request.");
    return false;
  }

  return true;
}

static bool send_request_member_add_to_queue(const json_t* j_data)
{
  int ret;
  int penalty_number;
  int paused_number;
  const char* queue;
  const char* interface;
  const char* member_name;
  const char* state_interface;
  char penalty[64];
  char paused[64];

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  queue = json_string_value(json_object_get(j_data, "queue_name"));
  interface = json_string_value(json_object_get(j_data, "state_interface"));
  member_name = json_string_value(json_object_get(j_data, "name"));
  state_interface = json_string_value(json_object_get(j_data, "state_interface"));

  penalty_number = json_integer_value(json_object_get(j_data, "penalty"));
  snprintf(penalty, sizeof(penalty) - 1, "%d", penalty_number);

  paused_number = json_integer_value(json_object_get(j_data, "paused"));
  snprintf(paused, sizeof(paused) - 1, "%d", paused_number);

  // send request
  ret = ami_action_queueadd(queue, interface, penalty, paused, member_name, state_interface);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not send ami request.");
    return false;
  }

  return true;
}

/**
 * Update given member's paused and penalty info.
 * @param j_data
 * @return
 */
bool queue_action_update_member_paused_penalty(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check penalty
  j_tmp = json_object_get(j_data, "penalty");
  if(j_tmp != NULL) {
    ret = send_request_member_penalty_update(j_data);
    if(ret == false) {
      slog(LOG_NOTICE, "Could not update member penalty info.");
      return false;
    }
  }

  // check paused
  j_tmp = json_object_get(j_data, "paused");
  if(j_tmp != NULL) {
    ret = send_request_member_paused_update(j_data);
    if(ret == false) {
      slog(LOG_NOTICE, "Could not update member paused info.");
      return false;
    }
  }

  return true;
}

bool queue_action_delete_member_from_queue(const char* id)
{
  int ret;
  const char* interface;
  const char* queue;
  json_t* j_tmp;

  if(id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired queue_action_delete_member_from_queue. id[%s]", id);

  j_tmp = queue_get_member_info(id);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get member info.");
    return false;
  }

  queue = json_string_value(json_object_get(j_tmp, "queue_name"));
  interface = json_string_value(json_object_get(j_tmp, "state_interface"));

  // send request
  ret = ami_action_queueremove(queue, interface);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not remove member from the queue.");
    return false;
  }

  return true;
}

bool queue_action_add_member_to_queue(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = send_request_member_add_to_queue(j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * delete queue entry info.
 * @return
 */
bool queue_delete_entry_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_queue_entry_info. key[%s]", key);

  // delete
  ret = db_delete_entry_info(key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete channel info. unique_id[%s]", key);
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

  return true;
}

/**
 * Get all queue_entry array
 * @return
 */
json_t* queue_get_entries_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items(DEF_DB_TABLE_QUEUE_ENTRY, "*");
  return j_res;
}

static bool clear_queue_member(void)
{
  int ret;

  ret = resource_clear_mem_table(DEF_DB_TABLE_QUEUE_MEMBER);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_member");
    return false;
  }

  return true;
}

bool queue_update_member_info(const json_t* j_data)
{
  int ret;

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

  return true;
}

/**
 * delete queue member info.
 * @return
 */
bool queue_delete_member_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_queue_member_info.");

  // delete
  ret = db_delete_member_info(key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue member info.");
    return false;
  }

  return true;
}

static bool clear_queue_param(void)
{
  int ret;

  ret = resource_clear_mem_table(DEF_DB_TABLE_QUEUE_PARAM);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear clear_queue_param");
    return false;
  }

  return true;
}

json_t* queue_cfg_get_queues_all(void)
{
  json_t* j_res;

  j_res = conf_get_ast_sections_all(DEF_JADE_QUEUE_CONFNAME_QUEUE);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* queue_cfg_get_queue_info(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = conf_get_ast_section(DEF_JADE_QUEUE_CONFNAME_QUEUE, name);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Register callback for db entry
 */
bool queue_register_callback_db_entry(bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*))
{
  int ret;

  if(func == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired queue_register_callback_db_entry.");

  ret = utils_register_callback(g_callback_db_entry, func);
  if(ret == false) {
    slog(LOG_ERR, "Could not register callback for entry.");
    return false;
  }

  return true;
}

/**
 * Execute the registered callbacks for db entry
 * @param j_data
 */
static void execute_callbacks_db_entry(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired execute_callbacks_db_entry.");

  utils_execute_callbacks(g_callback_db_entry, type, j_data);

  return;
}

/**
 * Register callback for db member
 */
bool queue_register_callback_db_member(bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*))
{
  int ret;

  if(func == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired queue_register_callback_db_member.");

  ret = utils_register_callback(g_callback_db_member, func);
  if(ret == false) {
    slog(LOG_ERR, "Could not register callback for member.");
    return false;
  }

  return true;
}

/**
 * Execute the registered callbacks for db member
 * @param j_data
 */
static void execute_callbacks_db_member(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired execute_callbacks_db_member.");

  utils_execute_callbacks(g_callback_db_member, type, j_data);

  return;
}

json_t* queue_get_configurations_all(void)
{
  json_t* j_res;
  json_t* j_tmp;

  j_res = json_array();

  // get current config
  j_tmp = conf_get_ast_current_config_info_text(DEF_QUEUE_CONFNAME);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get current config info.");
    json_decref(j_res);
    return NULL;
  }
  json_array_append_new(j_res, j_tmp);

  // get backup configs
  j_tmp = conf_get_ast_backup_configs_text_all(DEF_QUEUE_CONFNAME);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get backup configs info.");
    json_decref(j_res);
    return NULL;
  }
  json_array_extend(j_res, j_tmp);
  json_decref(j_tmp);

  return j_res;
}

json_t* queue_get_configuration_info(const char* name)
{
  int ret;
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  ret = strcmp(name, DEF_QUEUE_CONFNAME);
  if(ret == 0) {
    j_res = conf_get_ast_current_config_info_text(DEF_QUEUE_CONFNAME);
  }
  else {
    j_res = conf_get_ast_backup_config_info_text(name);
  }

  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

bool queue_update_configuration_info(const json_t* j_data)
{
  int ret;
  const char* name;
  const char* data;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  name = json_string_value(json_object_get(j_data, "name"));
  data = json_string_value(json_object_get(j_data, "data"));
  if((name == NULL) || (data == NULL)) {
    slog(LOG_NOTICE, "Could not get name or data info.");
    return false;
  }

  ret = strcmp(name, DEF_QUEUE_CONFNAME);
  if(ret != 0) {
    slog(LOG_NOTICE, "The only current configuration can update.");
    return false;
  }

  ret = conf_update_ast_current_config_info_text_data(DEF_QUEUE_CONFNAME, data);
  if(ret == false) {
    return false;
  }

  return true;
}

bool queue_delete_configuration_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = strcmp(name, DEF_QUEUE_CONFNAME);
  if(ret == 0) {
    slog(LOG_NOTICE, "The current configuration is not deletable.");
    return false;
  }

  ret = conf_remove_ast_backup_config_info_valid(name, DEF_QUEUE_CONFNAME);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete backup config info.");
    return false;
  }

  return true;
}
