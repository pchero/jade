/*
 * park_handler.c
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
#include "resource_handler.h"
#include "publication_handler.h"

#include "park_handler.h"

#define DEF_PARK_CONFNAME   "res_parking.conf"
#define DEF_JADE_PARK_CONFNAME    "jade.res_parking.conf"


#define DEF_DB_TABLE_PARK_PARKEDCALL   "parked_call"
#define DEF_DB_TABLE_PARK_PARKINGLOT   "parking_lot"


#define DEF_SETTING_CONTEXT_1   "general"
#define DEF_SETTING_CONTEXT_2   "default"


static bool init_databases(void);
static bool init_database_parked_call(void);
static bool init_database_parking_lot(void);

static bool init_configs(void);

static bool clear_park_parkinglot(void);
static bool clear_park_parkedcall(void);

static bool db_create_parkinglot_info(const json_t* j_data);

static bool db_create_parkedcall_info(const json_t* j_data);
static bool db_update_parkedcall_info(const json_t* j_data);
static bool db_delete_parkedcall_info(const char* key);

static bool cfg_create_parkinglot_info(const char* name, const json_t* j_data);
static bool cfg_update_parkinglot_info(const char* name, const json_t* j_data);
static bool cfg_delete_parkinglot_info(const char* key);

static bool is_setting_section(const char* context);


bool park_init_handler(void)
{
  int ret;
  json_t* j_tmp;

  slog(LOG_DEBUG, "Fired init_park_handler.");

  ret = init_databases();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate databases.");
    return false;
  }

  ret = init_configs();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate configs.");
    return false;
  }

  // parking_lot
  j_tmp = json_pack("{s:s}",
      "Action", "ParkingLots"
      );
  ret = ami_send_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "Parkinglosts");
    return false;
  }

  // parked_call
  j_tmp = json_pack("{s:s}",
      "Action", "ParkedCalls"
      );
  ret = ami_send_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "ParkedCalls");
    return false;
  }

  return true;
}

bool park_term_handler(void)
{
  int ret;

  ret = clear_park_parkinglot();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear parkinglot info.");
    return false;
  }

  ret = clear_park_parkedcall();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear parkedcall info.");
    return false;
  }

  return true;
}

bool park_reload_handler(void)
{
  int ret;

  ret = park_term_handler();
  if(ret == false) {
    return false;
  }

  ret = park_init_handler();
  if(ret == false) {
    return false;
  }

  return true;
}

static bool init_databases(void)
{
  int ret;

  ret = init_database_parked_call();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database parked_call.");
    return false;
  }

  ret = init_database_parking_lot();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database parking_lot.");
    return false;
  }

  return true;
}

static bool init_database_parked_call(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_PARK_PARKEDCALL;

  create_table =
    "create table " DEF_DB_TABLE_PARK_PARKEDCALL " ("

    // parked channel id info
    "   parkee_unique_id  varchar(255),"
    "   parkee_linked_id  varchar(255),"

    // parked channel info
    "   parkee_channel              varchar(255),"
    "   parkee_channel_state        varchar(255),"
    "   parkee_channel_state_desc   varchar(255),"

    // parked channel caller info
    "   parkee_caller_id_num    varchar(255),"
    "   parkee_caller_id_name   varchar(255),"

    // parked channel connected line info
    "   parkee_connected_line_num   varchar(255),"
    "   parkee_connected_line_name  varchar(255),"

    // parked channel account info
    "   parkee_account_code   varchar(255),"

    // parked channel dialplan info
    "   parkee_context    varchar(255),"
    "   parkee_exten      varchar(255),"
    "   parkee_priority   varchar(255),"

    // parked channel parker info
    "   parker_dial_string varchar(255),"

    // parking lot info
    "   parking_lot       varchar(255),"
    "   parking_space     varchar(255),"
    "   parking_timeout   int,"
    "   parking_duration  int,"

    // timestamp. UTC."
    "   tm_update         datetime(6),"   // update time."

    "   primary key(parkee_unique_id)"

    ");";

  // execute
  resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_PARK_PARKEDCALL);
    return false;
  }

  return true;
}

static bool init_database_parking_lot(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_PARK_PARKINGLOT;

  create_table =
    "create table " DEF_DB_TABLE_PARK_PARKINGLOT " ("

    "   name varchar(255),"

    "   start_space varchar(255),"
    "   stop_space  varchar(255),"

    "   timeout int,"

    // timestamp. UTC."
    "   tm_update         datetime(6),"   // update time."

    "   primary key(name)"

    ");";

  // execute
  resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_PARK_PARKINGLOT);
    return false;
  }

  return true;
}

static bool init_configs(void)
{
  int ret;

  // park
  ret = conf_add_external_config_file(DEF_PARK_CONFNAME, DEF_JADE_PARK_CONFNAME);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate park config.");
    return false;
  }

  return true;
}


static bool clear_park_parkinglot(void)
{
  int ret;

  ret = resource_clear_mem_table(DEF_DB_TABLE_PARK_PARKINGLOT);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear park_parkinglot");
    return false;
  }

  return true;
}

static bool clear_park_parkedcall(void)
{
  int ret;

  ret = resource_clear_mem_table(DEF_DB_TABLE_PARK_PARKEDCALL);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear park_parkedcall.");
    return false;
  }

  return true;
}



static bool db_create_parkinglot_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_insert_mem_item(DEF_DB_TABLE_PARK_PARKINGLOT, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parking_lot.");
    return false;
  }

  return true;
}

static bool db_create_parkedcall_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_insert_mem_item(DEF_DB_TABLE_PARK_PARKEDCALL, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parked_call.");
    return false;
  }

  return true;
}

static bool db_update_parkedcall_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_update_mem_item(DEF_DB_TABLE_PARK_PARKEDCALL, "parkee_unique_id", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update park parked_call info.");
    return false;
  }

  return true;
}

static bool db_delete_parkedcall_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_delete_mem_items_string(DEF_DB_TABLE_PARK_PARKEDCALL, "parkee_unique_id", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete park parkedcall info. unique_id[%s]", key);
    return false;
  }

  return true;
}



/**
 * Get all parkedcalls info array
 * @return
 */
json_t* park_get_parkedcalls_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_park_parkedcalls_all.");

  j_res = resource_get_mem_items(DEF_DB_TABLE_PARK_PARKEDCALL, "*");

  return j_res;
}

/**
 * Get corresponding parked call detail info.
 * @return
 */
json_t* park_get_parkedcall_info(const char* parkee_unique_id)
{
  json_t* j_res;

  if(parkee_unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_parked_call_info. name[%s]", parkee_unique_id);

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_PARK_PARKEDCALL, "parkee_unique_id", parkee_unique_id);

  return j_res;
}

/**
 * Create parked call detail info.
 * @return
 */
bool park_create_parkedcall_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_create_parkedcall_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parked_call.");
    return false;
  }

  // publish
  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "parkee_unique_id"));
  j_tmp = park_get_parkedcall_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get park parkedcall info. parkee_unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_park_parkedcall(DEF_PUB_TYPE_CREATE, j_tmp);
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
bool park_update_parkedcall_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_park_parkedcall_info.");

  ret = db_update_parkedcall_info(j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update park parked_call info.");
    return false;
  }

  // publish
  // get queue info
  tmp_const = json_string_value(json_object_get(j_data, "parkee_unique_id"));
  j_tmp = park_get_parkedcall_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get park parkedcall info. parkee_unique_id[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_park_parkedcall(DEF_PUB_TYPE_UPDATE, j_tmp);
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
bool park_delete_parkedcall_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = park_get_parkedcall_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "The channel is already deleted. key[%s]", key);
    return false;
  }

  ret = db_delete_parkedcall_info(key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete park parkedcall info. unique_id[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish delete event.
  ret = publication_publish_event_park_parkedcall(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}



/**
 * Get all parking_lot's array
 * @return
 */
json_t* park_get_parkinglots_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_park_parkinglots_all.");

  j_res = resource_get_mem_items(DEF_DB_TABLE_PARK_PARKINGLOT, "*");

  return j_res;
}

/**
 * Get corresponding parking_lot detail info.
 * @return
 */
json_t* park_get_parkinglot_info(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_park_parkinglot_info. name[%s]", name);

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_PARK_PARKINGLOT, "name", name);

  return j_res;
}

/**
 * Create park_parkinglot detail info.
 * @return
 */
bool park_create_parkinglot_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* tmp_const;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_park_parkinglot_info.");

  ret = db_create_parkinglot_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parking_lot.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  j_tmp = park_get_parkinglot_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get park parkinglot info. name[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_park_parkinglot(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

static bool is_setting_section(const char* context)
{
  int ret;

  if(context == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = strcmp(context, DEF_SETTING_CONTEXT_1);
  if(ret == 0) {
    return true;
  }

  ret = strcmp(context, DEF_SETTING_CONTEXT_2);
  if(ret == 0) {
    return true;
  }

  return false;
}

static bool cfg_create_parkinglot_info(const char* name, const json_t* j_data)
{
  int ret;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_create_parkinglot_info. name[%s]", name);

  ret = conf_create_ast_section(DEF_JADE_PARK_CONFNAME, name, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create config for parkinglot.");
    return false;
  }

  return true;
}

static bool cfg_update_parkinglot_info(const char* name, const json_t* j_data)
{
  int ret;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_update_parkinglot_info. name[%s]", name);

  ret = conf_update_ast_section(DEF_JADE_PARK_CONFNAME, name, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update config for parkinglot. name[%s]", name);
    return false;
  }

  return true;
}

static bool cfg_delete_parkinglot_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // delete parkinglot
  ret = conf_delete_ast_section(DEF_JADE_PARK_CONFNAME, key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete parkinglot.");
    return false;
  }

  return true;
}


/**
 * Create parking lot info.
 * @return
 */
bool park_cfg_create_parkinglot_info(const json_t* j_data)
{
  const char* name;
  int ret;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_parkinglot");

  // get mandatory item
  name = json_string_value(json_object_get(j_data, "name"));
  j_tmp = json_object_get(j_data, "data");
  if((name == NULL) || (j_tmp == NULL)) {
    slog(LOG_ERR, "Could not get name for data info.");
    return false;
  }

  // is setting context?
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_NOTICE, "Given context is setting context.");
    return false;
  }

  ret = cfg_create_parkinglot_info(name, j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create new parkinglot.");
    return false;
  }

  return true;
}

/**
 * Update parkinglot info.
 * @param name
 * @param j_data
 * @return
 */
bool park_cfg_update_parkinglot_info(const json_t* j_data)
{
  const char* name;
  json_t* j_tmp;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_parkinglot_info.");

  name = json_string_value(json_object_get(j_data, "name"));
  j_tmp = json_object_get(j_data, "data");
  if((name == NULL) || (j_tmp == NULL)) {
    slog(LOG_NOTICE, "Could not get name or data info.");
    return false;
  }

  // check setting section
  ret = is_setting_section(name);
  if(ret == true) {
    slog(LOG_ERR, "Given parkinglot name is setting section. name[%s]", name);
    return false;
  }

  ret = cfg_update_parkinglot_info(name, j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update park info.");
    return false;
  }

  return true;
}

/**
 * Delete parkinglot.
 * @param parkinglot
 * @return
 */
bool park_cfg_delete_parkinglot_info(const char* parkinglot)
{
  int ret;

  if(parkinglot == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_parkinglot. parkinglot[%s]", parkinglot);

  ret = is_setting_section(parkinglot);
  if(ret == true) {
    slog(LOG_ERR, "Given context is setting context.");
    return false;
  }

  // delete parkinglot
  ret = cfg_delete_parkinglot_info(parkinglot);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete parkinglot.");
    return false;
  }

  return true;
}

