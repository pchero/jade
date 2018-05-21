/*
 * dialplan_handler.c
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <jansson.h>
#include <string.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "conf_handler.h"
#include "http_handler.h"
#include "resource_handler.h"
#include "publication_handler.h"
#include "ami_action_handler.h"

#include "dialplan_handler.h"


#define DEF_AST_DIALPLAN_CONFNAME   "extensions.conf"

#define DEF_JADE_DIALPLAN_CONFNAME  "jade.extensions.conf"

#define DEF_JADE_DEMO_CONTEXT       "jade_demo"


#define DEF_DB_TABLE_DP_DIALPLANMASTER    "dp_dpma"
#define DEF_DB_TABLE_DP_DIALPLAN          "dp_dialplan"

static bool init_databases(void);
static bool init_database_dp_dialplan(void);
static bool init_database_dp_dialplanmaster(void);

static bool init_configs(void);

static bool db_create_dialplan_info(const json_t* j_data);
static bool db_update_dialplan_info(const json_t* j_data);
static bool db_delete_dialplan_info(const char* key);

static bool db_create_dpma_info(const json_t* j_data);
static bool db_update_dpma_info(const json_t* j_data);
static bool db_delete_dpma_info(const char* key);

static bool init_default_originate_to_device(void);

static bool create_dpma_info_with_uuid(const char* uuid, const json_t* j_data);
static bool is_exist_dpma_info(const char* uuid);
static bool is_default_dpma_originate_to_device(const char* agi_uuid);

static bool is_exist_dialplan_info(const char* dpma_uuid, int seq);
static bool is_exist_dialplan_info_uuid(const char* uuid);

// static dialplan
static bool cfg_create_sdialplan_info(const char* name, const json_t* j_data);
static bool cfg_update_sdialplan_info(const json_t* j_data);
static bool cfg_delete_sdialplan_info(const char* name);
static bool is_exist_sdialplan(const char* name);

static bool send_setvar_for_agi_call(const char* agi_uuid);

static bool add_agi_cmd_for_default_originate_to_device(const json_t* j_agi);


/**
 * init dialplan.
 * @return
 */
bool dialplan_init_handler(void)
{
  int ret;

  ret = init_databases();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database.");
    return false;
  }

  ret = init_configs();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate config.");
    return false;
  }

  ret = init_default_originate_to_device();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate default dialplans.");
    return false;
  }

  return true;
}

bool dialplan_term_handler(void)
{
  return true;
}

bool dialplan_reload_handler(void)
{
  int ret;

  ret = dialplan_term_handler();
  if(ret == false) {
    return false;
  }

  ret = dialplan_init_handler();
  if(ret == false) {
    return false;
  }

  return true;
}

static bool init_configs(void)
{
  int ret;
  json_t* j_tmp;

  // check config file
  ret = conf_is_exist_config_file(DEF_JADE_DIALPLAN_CONFNAME);
  if(ret == true) {
    return true;
  }

  ret = conf_add_external_config_file(DEF_AST_DIALPLAN_CONFNAME, DEF_JADE_DIALPLAN_CONFNAME);
  if(ret == false) {
    slog(LOG_ERR, "Could not add the external config file. filename[%s], external[%s]", DEF_AST_DIALPLAN_CONFNAME, DEF_JADE_DIALPLAN_CONFNAME);
    return false;
  }

  // add default context
  j_tmp = json_pack("["
        "{s:s}, "
        "{s:s}"
      "]",

      "exten",  "> _X.,1,BackGround(demo-congrats)",
      "same",   "> n,Hangup()"
      );
  ret = cfg_create_sdialplan_info(DEF_JADE_DEMO_CONTEXT, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create default jade demo context.");
    return false;
  }

  ret = dialplan_reload_asterisk();
  if(ret == false) {
    slog(LOG_ERR, "Could not reload dialplan asterisk.");
    return false;
  }

  return true;
}

static bool init_databases(void)
{
  int ret;

  slog(LOG_DEBUG, "Fired init_databases.");

  ret = init_database_dp_dialplanmaster();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate dialplanmaster database.");
    return false;
  }

  ret = init_database_dp_dialplan();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate dialplan database.");
    return false;
  }

  return true;
}

static bool init_database_dp_dialplanmaster(void)
{
  int ret;
  const char* create_table;

  create_table =
      "create table if not exists " DEF_DB_TABLE_DP_DIALPLANMASTER " ("

      // identify
      "   uuid    varchar(255),"

      // info
      "   name        varchar(255),"
      "   detail      varchar(1023),"

      // timestamp. UTC."
      "   tm_create         datetime(6),"   // create time
      "   tm_update         datetime(6),"   // update time.

      "   primary key(uuid)"
      ");";

  ret = resource_exec_file_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_DP_DIALPLANMASTER);
    return false;
  }

  return true;
}

static bool init_database_dp_dialplan(void)
{
  int ret;
  const char* create_table;

  create_table =
      "create table if not exists " DEF_DB_TABLE_DP_DIALPLAN " ("

      // identify
      "   uuid        varchar(255),"  // identity.
      "   dpma_uuid   varchar(255),"
      "   sequence    int,"

      // info
      "   name        varchar(255),"
      "   detail      varchar(1023),"
      "   command     varchar(1023),"

      // timestamp. UTC."
      "   tm_create         datetime(6),"   // create time
      "   tm_update         datetime(6),"   // update time.

      "   primary key(dpma_uuid, sequence)"   ///
      ");";


  ret = resource_exec_file_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_DP_DIALPLAN);
    return false;
  }

  return true;
}

/**
 * Check there is default dialplan for originate to device.
 * If not, create default.
 * @return
 */
static bool init_default_originate_to_device(void)
{
  int ret;
  char* dpma_uuid;
  json_t* j_dpma;
  json_t* j_tmp;

  dpma_uuid = dialplan_get_default_dpma_originate_to_device();
  if(dpma_uuid == NULL) {
    slog(LOG_ERR, "Could not get default_dpma_originate_to_device info.");
    return false;
  }

  j_dpma = dialplan_get_dpma_info(dpma_uuid);
  if(j_dpma != NULL) {
    // already exist.
    sfree(dpma_uuid);
    json_decref(j_dpma);
    return true;
  }

  // does not exist. create default.
  j_tmp = json_pack("{s:s, s:s}",
      "name",     "Default originate call to device",
      "detail",   "Default jade dialplan master for call originate to device."
      );

  ret = create_dpma_info_with_uuid(dpma_uuid, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create default dpma for originate to device.");
    sfree(dpma_uuid);
    return false;
  }

  // add dialplans 1
  j_tmp = json_pack("{s:s, s:s, s:s, s:s, s:i}",
      "dpma_uuid",  dpma_uuid,
      "command",    "Exec NoOp \"Jade default dialplan for originate call to device\"",
      "name",       "OCTD ",
      "detail",     "Jade default dialplan for originate call to device",
      "sequence",   1
      );

  ret = dialplan_create_dialplan_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not add dialplan 1.");
    sfree(dpma_uuid);
    return false;
  }

  sfree(dpma_uuid);

  return true;
}

/**
 * Update dpma.
 * @return
 */
bool dialplan_update_dpma_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;
  const char* uuid;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired dialplan_update_dpma_info.");

  uuid = json_string_value(json_object_get(j_data, "uuid"));
  if(uuid == NULL) {
    slog(LOG_NOTICE, "Could not get uuid info.");
    return false;
  }

  ret = is_exist_dpma_info(uuid);
  if(ret == false) {
    slog(LOG_ERR, "The given dpma info is not exist. dpma_uuid[%s]", uuid);
    return false;
  }

  j_tmp = json_deep_copy(j_data);
  json_object_del(j_tmp, "uuid");
  json_object_del(j_tmp, "tm_create");
  json_object_del(j_tmp, "tm_update");

  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  timestamp = utils_get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(timestamp));
  sfree(timestamp);

  ret = db_update_dpma_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dpma info.");
    return false;
  }

  return true;
}

/**
 * Delete dpma.
 * @return
 */
bool dialplan_delete_dpma_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired dialplan_delete_dpma_info. uuid[%s]", uuid);

  ret = db_delete_dpma_info(uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete dpma info.");
    return false;
  }

  return true;
}

/**
 * Create dpma.
 * @return
 */
bool dialplan_create_dpma_info(const json_t* j_data)
{
  int ret;
  char* uuid;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_dp_dpma_info.");

  uuid = utils_gen_uuid();
  ret = create_dpma_info_with_uuid(uuid, j_data);
  sfree(uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dpma contact.");
    return false;
  }

  return true;
}

/**
 * Create dpma.
 * @return
 */
static bool create_dpma_info_with_uuid(const char* uuid, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_dp_dpma_info.");

  j_tmp = json_deep_copy(j_data);
  json_object_del(j_tmp, "tm_create");
  json_object_del(j_tmp, "tm_update");
  json_object_del(j_tmp, "tm_uuid");

  timestamp = utils_get_utc_timestamp();

  json_object_set_new(j_tmp, "uuid", json_string(uuid));
  json_object_set_new(j_tmp, "tm_create", json_string(timestamp));
  sfree(timestamp);

  // insert info
  ret = db_create_dpma_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dpma contact.");
    return false;
  }

  return true;
}


/**
 * Create dialplan.
 * @return
 */
bool dialplan_create_dialplan_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* uuid;
  char* timestamp;
  const char* dpma_uuid;
  int seq;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_dialplan_info.");

  // get dpma_uuid
  dpma_uuid = json_string_value(json_object_get(j_data, "dpma_uuid"));
  if(dpma_uuid == NULL) {
    slog(LOG_ERR, "Could not get dpma_uuid info.");
    return false;
  }

  // get sequence info
  seq = json_integer_value(json_object_get(j_data, "sequence"));
  if(seq <= 0) {
    slog(LOG_ERR, "Could not get correct sequence info.");
    return false;
  }

  // check existence
  ret = is_exist_dialplan_info(dpma_uuid, seq);
  if(ret == true) {
    slog(LOG_ERR, "The given dialplan info is already  exist. dpma_uuid[%s], seq[%d]", dpma_uuid, seq);
    return false;
  }

  j_tmp = json_deep_copy(j_data);
  json_object_del(j_tmp, "uuid");
  json_object_del(j_tmp, "tm_create");
  json_object_del(j_tmp, "tm_update");
  json_object_del(j_tmp, "tm_uuid");

  timestamp = utils_get_utc_timestamp();
  uuid = utils_gen_uuid();

  json_object_set_new(j_tmp, "uuid", json_string(uuid));
  sfree(uuid);
  json_object_set_new(j_tmp, "tm_create", json_string(timestamp));
  sfree(timestamp);

  // insert info
  ret = db_create_dialplan_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dialplan contact.");
    return false;
  }

  return true;
}

/**
 * Update dialplan.
 * @return
 */
bool dialplan_update_dialplan_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;
  const char* dpma_uuid;
  const char* uuid;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_dialplan_info.");

  uuid = json_string_value(json_object_get(j_data, "uuid"));
  if(uuid == NULL) {
    slog(LOG_NOTICE, "Could not get uuid info.");
    return false;
  }

  ret = is_exist_dialplan_info_uuid(uuid);
  if(ret == false) {
    slog(LOG_ERR, "The given dialplan is not exist. uuid[%s]", uuid);
    return false;
  }

  dpma_uuid = json_string_value(json_object_get(j_data, "dpma_uuid"));
  if(dpma_uuid == NULL) {
    slog(LOG_ERR, "Could not get dpma_uuid info.");
    return false;
  }

  // check dpma existence
  ret = is_exist_dpma_info(dpma_uuid);
  if(ret == false) {
    slog(LOG_ERR, "The given dpma is not exist. dpma_uuid[%s]", dpma_uuid);
    return false;
  }

  j_tmp = json_deep_copy(j_data);
  json_object_del(j_tmp, "uuid");
  json_object_del(j_tmp, "tm_create");
  json_object_del(j_tmp, "tm_update");

  timestamp = utils_get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(timestamp));
  sfree(timestamp);

  ret = db_update_dialplan_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dpma info.");
    return false;
  }

  return true;
}

/**
 * Delete dialplan.
 * @return
 */
bool dialplan_delete_dialplan_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_dialplan_info. uuid[%s]", uuid);

  ret = db_delete_dialplan_info(uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete dialplan info.");
    return false;
  }

  return true;
}


static bool is_exist_dpma_info(const char* uuid)
{
  json_t* j_tmp;

  j_tmp = dialplan_get_dpma_info(uuid);
  if(j_tmp == NULL) {
    return false;
  }

  json_decref(j_tmp);
  return true;
}

static bool is_exist_dialplan_info_uuid(const char* uuid)
{
  json_t* j_tmp;

  j_tmp = dialplan_get_dialplan_info(uuid);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;

}

static bool is_exist_dialplan_info(const char* dpma_uuid, int seq)
{
  json_t* j_tmp;

  j_tmp = dialplan_get_dialplan_info_by_dpma_seq(dpma_uuid, seq);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

/**
 * Add the commands to the agi channel.
 * @param unique_id
 * @return
 */
bool dialplan_add_cmds(const char* agi_uuid)
{
  json_t* j_agi;
  json_t* j_dps;
  json_t* j_dp;
  int ret;
  int idx;
  char* uuid;
  const char* jade_dpma_constant;
  const char* jade_dpma_uuid;
  const char* channel;
  const char* dp_uuid;
  const char* command;

  if(agi_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired add_dialplan_cmds. agi_uuid[%s]", agi_uuid);

  // get agi info
  j_agi = get_core_agi_info(agi_uuid);
  if(j_agi == NULL) {
    slog(LOG_ERR, "Could not get agi info.");
    return false;
  }

  // check the given agi is DEF_DIALPLAN_AGI_NAME or not.
  // agi_arg_1 should be DEF_DIALPLAN_AGI_NAME.
  jade_dpma_constant = json_string_value(json_object_get(json_object_get(j_agi, "env"), "agi_arg_1"));
  if(jade_dpma_constant == NULL) {
    slog(LOG_NOTICE, "The given agi is not for the jade_dialplan.");
    json_decref(j_agi);
    return false;
  }
  if(strcasecmp(jade_dpma_constant, DEF_DIALPLAN_JADE_AGI_NAME) != 0) {
    slog(LOG_NOTICE, "Unmatched jade_dialplan. The given agi is not for the jade_dialplan.");
    json_decref(j_agi);
    return false;
  }

  // get agi_arg_2
  // consider agi_arg_2 as a dpma_uuid
  jade_dpma_uuid = json_string_value(json_object_get(json_object_get(j_agi, "env"), "agi_arg_2"));
  if(jade_dpma_uuid == NULL) {
    slog(LOG_NOTICE, "Could not get agi_arg_2 info.");
    json_decref(j_agi);
    return false;
  }

  ret = is_exist_dpma_info(jade_dpma_uuid);
  if(ret == false) {
    slog(LOG_ERR, "The given agi_arg_2 is not exist.");
    json_decref(j_agi);
    return false;
  }

  // get dialplans
  j_dps = dialplan_get_dialplans_by_dpma_uuid_order_sequence(jade_dpma_uuid);
  if(j_dps == NULL) {
    slog(LOG_ERR, "Could not get dialplan info");
    json_decref(j_agi);
    return false;
  }

  // send setvar
  ret = send_setvar_for_agi_call(agi_uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not get send setvar for agi call. uuid[%s]", agi_uuid);
    json_decref(j_agi);
    return false;
  }

  // send agi request
  json_array_foreach(j_dps, idx, j_dp) {
    uuid = utils_gen_uuid();

    channel = json_string_value(json_object_get(j_agi, "channel"));
    command = json_string_value(json_object_get(j_dp, "command"));
    dp_uuid = json_string_value(json_object_get(j_dp, "uuid"));

    // send ami action
    ret = ami_action_agi(channel, command, uuid);
    if(ret == false) {
      sfree(uuid);
      continue;
    }

    // add cmd
    ret = add_core_agi_info_cmd(agi_uuid, uuid, command, dp_uuid);
    sfree(uuid);
    if(ret == false) {
      slog(LOG_ERR, "Could not add cmd info.");
      continue;
    }
  }

  // check default dpma originate to device
  ret = is_default_dpma_originate_to_device(jade_dpma_uuid);
  if(ret == true) {
    slog(LOG_INFO, "The given agi is default dpma for originate to device.");
    add_agi_cmd_for_default_originate_to_device(j_agi);
  }

  json_decref(j_dps);
  json_decref(j_agi);

  return true;
}

static bool send_setvar_for_agi_call(const char* agi_uuid)
{
  int ret;
  json_t* j_agi;
  json_t* j_env;
  json_t* j_val;
  const char* channel;
  const char* key;
  const char* value;

  if(agi_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get agi info
  j_agi = get_core_agi_info(agi_uuid);
  if(j_agi == NULL) {
    slog(LOG_ERR, "Could not get agi info.");
    return false;
  }

  j_env = json_object_get(j_agi, "env");
  if(j_env == NULL) {
    slog(LOG_NOTICE, "There is no env info.");
    json_decref(j_agi);
    return true;
  }

  // get channel
  channel = json_string_value(json_object_get(j_agi, "channel"));
  if(channel == NULL) {
    slog(LOG_ERR, "Could not get channel info.");
    json_decref(j_agi);
    return true;
  }

  // send setvar for each env object
  json_object_foreach(j_env, key, j_val) {
    value = json_string_value(j_val);
    ret = ami_action_setvar(channel, key, value);
    if(ret == false) {
      slog(LOG_ERR, "Coudl not send setvar request. channel[%s], key[%s], value[%s]", channel, key, value);
      continue;
    }
  }
  json_decref(j_agi);

  return true;
}

/**
 * Get all dp_dpma array
 * @return
 */
json_t* dialplan_get_dpmas_all(void)
{
  json_t* j_res;

  j_res = resource_get_file_items(DEF_DB_TABLE_DP_DIALPLANMASTER, "*");
  return j_res;
}

/**
 * Get corresponding dp_dpma detail info.
 * @return
 */
json_t* dialplan_get_dpma_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_dp_dpma_info. key[%s]", key);

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_DP_DIALPLANMASTER, "uuid", key);

  return j_res;
}

/**
 * Create dp dpma info.
 * @param j_data
 * @return
 */
static bool db_create_dpma_info(const json_t* j_data)
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
  ret = resource_insert_file_item(DEF_DB_TABLE_DP_DIALPLANMASTER, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dpma info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = dialplan_get_dpma_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dpma info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_dp_dpma(DEF_PUB_TYPE_CREATE, j_tmp);
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
static bool db_update_dpma_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_update_dpma_info.");

  // update
  ret = resource_update_file_item(DEF_DB_TABLE_DP_DIALPLANMASTER, "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dp_dpma info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = dialplan_get_dpma_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dpma info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_dp_dpma(DEF_PUB_TYPE_UPDATE, j_tmp);
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
static bool db_delete_dpma_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_delete_dpma_info. key[%s]", key);

  // get info
  j_tmp = dialplan_get_dpma_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dpma info. uuid[%s]", key);
    return false;
  }

  ret = resource_delete_file_items_string(DEF_DB_TABLE_DP_DIALPLANMASTER, "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete dp_dpma info. key[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publication_publish_event_dp_dpma(DEF_PUB_TYPE_DELETE, j_tmp);
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
json_t* dialplan_get_dialplans_all(void)
{
  json_t* j_res;

  j_res = resource_get_file_items(DEF_DB_TABLE_DP_DIALPLAN, "*");
  return j_res;
}

/**
 * Get all dp_dialplans by dpma_uuid order by sequence
 * @return
 */
json_t* dialplan_get_dialplans_by_dpma_uuid_order_sequence(const char* dpma_uuid)
{
  json_t* j_res;
  json_t* j_obj;

  if(dpma_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_obj = json_pack("{s:s}", "dpma_uuid", dpma_uuid);

  j_res = resource_get_file_detail_items_by_obj_order(DEF_DB_TABLE_DP_DIALPLAN, j_obj, "sequence");
  json_decref(j_obj);

  return j_res;
}

/**
 * Get corresponding dp_dialplan detail info.
 * @return
 */
json_t* dialplan_get_dialplan_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_dp_dialplan_info. key[%s]", key);

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_DP_DIALPLAN, "uuid", key);

  return j_res;
}

/**
 * Get corresponding dp_dialplan detail info.
 * @return
 */
json_t* dialplan_get_dialplan_info_by_dpma_seq(const char* dpma_uuid, int seq)
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

  j_res = resource_get_file_detail_item_by_obj(DEF_DB_TABLE_DP_DIALPLAN, j_obj);
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
static bool db_create_dialplan_info(const json_t* j_data)
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
  ret = resource_insert_file_item(DEF_DB_TABLE_DP_DIALPLAN, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert dp_dialplan contact.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = dialplan_get_dialplan_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dialplan info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_dp_dialplan(DEF_PUB_TYPE_CREATE, j_tmp);
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
static bool db_update_dialplan_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_update_dialplan_info.");

  // update
  ret = resource_update_file_item(DEF_DB_TABLE_DP_DIALPLAN, "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update dp_dialplan info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "uuid"));
  j_tmp = dialplan_get_dialplan_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dialplan info. uuid[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_dp_dialplan(DEF_PUB_TYPE_UPDATE, j_tmp);
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
static bool db_delete_dialplan_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_delete_dialplan_info. key[%s]", key);

  // get info
  j_tmp = dialplan_get_dialplan_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get dp_dialplan info. uuid[%s]", key);
    return false;
  }

  ret = resource_delete_file_items_string(DEF_DB_TABLE_DP_DIALPLAN, "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete dp_dialplan info. key[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publication_publish_event_dp_dialplan(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

char* dialplan_get_default_dpma_originate_to_device(void)
{
  json_t* j_dp_conf;
  const char* tmp_const;
  char* res;

  j_dp_conf = json_object_get(g_app->j_conf, "dialplan");
  if(j_dp_conf == NULL) {
    slog(LOG_ERR, "Could not get dialplan configuration info.");
    return NULL;
  }

  // get default_dpma_originate_to_device
  tmp_const = json_string_value(json_object_get(j_dp_conf, "default_dpma_originate_to_device"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get default_dpma_originate_to_device info.");
    return NULL;
  }

  res = strdup(tmp_const);
  return res;
}

static bool is_default_dpma_originate_to_device(const char* agi_uuid)
{
  int ret;
  char* default_dpma;

  if(agi_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  default_dpma = dialplan_get_default_dpma_originate_to_device();
  if(default_dpma == NULL) {
    slog(LOG_ERR, "Could not get default_dpma info.");
    return false;
  }

  ret = strcmp(agi_uuid, default_dpma);
  sfree(default_dpma);
  if(ret != 0) {
    return false;
  }

  return true;
}

static bool add_agi_cmd_for_default_originate_to_device(const json_t* j_agi)
{
  int ret;
  const char* target;
  char* cmd;
  char* uuid;
  const char* uuid_agi;
  const char* channel;

  if(j_agi == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired add_agi_cmd_for_default_originate_to_device.");

  // get target
  target = json_string_value(json_object_get(json_object_get(j_agi, "env"), "agi_arg_3"));
  if(target == NULL) {
    slog(LOG_NOTICE, "Could not get target device info.");
    return false;
  }

  // create command
  asprintf(&cmd, "Exec Dial PJSIP/%s", target);
  slog(LOG_DEBUG, "Created originate to device cmd. cmd[%s]", cmd);

  uuid = utils_gen_uuid();
  channel = json_string_value(json_object_get(j_agi, "channel"));

  // send agi
  ret = ami_action_agi(channel, cmd, uuid);
  if(ret == false) {
    sfree(cmd);
    sfree(uuid);
    return false;
  }

  // add cmd info
  uuid_agi = json_string_value(json_object_get(j_agi, "uuid"));
  ret = add_core_agi_info_cmd(uuid_agi, uuid, cmd, "");
  sfree(uuid);
  sfree(cmd);
  if(ret == false) {
    slog(LOG_ERR, "Could not add cmd info.");
    return false;
  }

  return true;
}

static bool cfg_create_sdialplan_info(const char* name, const json_t* j_data)
{
  int ret;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_create_sdialplan_info. name[%s]", name);

  ret = is_exist_sdialplan(name);
  if(ret == true) {
    slog(LOG_NOTICE, "Already exist sdialplan. name[%s]", name);
    return false;
  }

  ret = conf_create_ast_section_array(DEF_JADE_DIALPLAN_CONFNAME, name, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create config for pjsip aor.");
    return false;
  }

  return true;
}

static bool cfg_update_sdialplan_info(const json_t* j_data)
{
  int ret;
  const char* name;
  json_t* j_contaents;
  json_t* j_content;
  json_t* j_dialplans;
  json_t* j_tmp;
  int idx;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_update_sdialplan_info.");

  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_NOTICE, "Could not get name info.");
    return false;
  }

  j_contaents = json_object_get(j_data, "data");
  if(j_contaents == NULL) {
    slog(LOG_NOTICE, "Could not get contents info.");
    return false;
  }

  j_dialplans = json_array();
  json_array_foreach(j_contaents, idx, j_content) {
    j_tmp = json_pack("{s:s}",
        json_string_value(json_object_get(j_content, "type")),
        json_string_value(json_object_get(j_content, "data"))
        );

    json_array_append_new(j_dialplans, j_tmp);
  }

  ret = conf_update_ast_section_array(DEF_JADE_DIALPLAN_CONFNAME, name, j_dialplans);
  json_decref(j_dialplans);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update config for pjsip aor.");
    return false;
  }

  return true;
}

static bool cfg_delete_sdialplan_info(const char* name)
{
  int ret;

  if((name == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cfg_delete_aor_info. name[%s]", name);

  ret = conf_delete_ast_section_array(DEF_JADE_DIALPLAN_CONFNAME, name);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update config for pjsip aor.");
    return false;
  }

  return true;
}

json_t* dialplan_get_sdialplans_all(void)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_confs;
  json_t* j_conf;
  const char* tmp_const;

  j_confs = conf_get_ast_current_config_info_array(DEF_JADE_DIALPLAN_CONFNAME);
  if(j_confs == NULL) {
    return NULL;
  }

  j_res = json_array();
  json_object_foreach(j_confs, tmp_const, j_conf) {
    j_tmp = json_pack("{s:s, s:O}",
        "name",       tmp_const,
        "data",   j_conf
        );
    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_confs);

  return j_res;
}

/**
 * Returns given name of static dialplan info.
 * @param name
 * @return
 */
json_t* dialplan_get_sdialplan_info(const char* name)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_confs;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get config info
  j_confs = conf_get_ast_current_config_info_array(DEF_JADE_DIALPLAN_CONFNAME);
  if(j_confs == NULL) {
    slog(LOG_NOTICE, "Could not get config info.");
    return NULL;
  }

  j_tmp = json_object_get(j_confs, name);
  if(j_tmp == NULL) {
    json_decref(j_confs);
    return NULL;
  }

  j_res = json_pack("{s:s, s:O}",
      "name",     name,
      "data", j_tmp
      );
  json_decref(j_confs);

  return j_res;
}

/**
 * Create the given static dialplan info.
 * @param j_data
 * @return
 */
bool dialplan_create_sdialplan_info(const json_t* j_data)
{
  int ret;
  const char* name;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired dialplan_create_sdialplan_info.");

  name = json_string_value(json_object_get(j_data, "name"));
  j_tmp = json_object_get(j_data, "data");
  if((j_tmp == NULL) || (json_is_array(j_tmp) == false)) {
    slog(LOG_NOTICE, "Could not get correct data info.");
    return false;
  }

  ret = cfg_create_sdialplan_info(name, j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Update the given static dialplan info.
 * @param j_data
 * @return
 */
bool dialplan_update_sdialplan_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired dialplan_update_sdialplan_info.");

  // update
  ret = cfg_update_sdialplan_info(j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update sdialplan info.");
    return false;
  }

  return true;
}

/**
 * Delete the given static dialplan info.
 * @param j_data
 * @return
 */
bool dialplan_delete_sdialplan_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired dialplan_delete_sdialplan_info. name[%s]", name);

  ret = cfg_delete_sdialplan_info(name);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Return true if the given sdialplan name is already exsit.
 * @param name
 * @return
 */
static bool is_exist_sdialplan(const char* name)
{
  json_t* j_tmp;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = dialplan_get_sdialplan_info(name);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

json_t* dialplan_get_configurations_all(void)
{
  json_t* j_res;
  json_t* j_tmp;

  j_res = json_array();

  // get current config
  j_tmp = conf_get_ast_current_config_info_text(DEF_AST_DIALPLAN_CONFNAME);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get current config info.");
    json_decref(j_res);
    return NULL;
  }
  json_array_append_new(j_res, j_tmp);

  // get backup configs
  j_tmp = conf_get_ast_backup_configs_text_all(DEF_AST_DIALPLAN_CONFNAME);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get backup configs info.");
    json_decref(j_res);
    return NULL;
  }
  json_array_extend(j_res, j_tmp);
  json_decref(j_tmp);

  return j_res;
}

json_t* dialplan_get_configuration_info(const char* name)
{
  int ret;
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  ret = strcmp(name, DEF_AST_DIALPLAN_CONFNAME);
  if(ret == 0) {
    j_res = conf_get_ast_current_config_info_text(DEF_AST_DIALPLAN_CONFNAME);
  }
  else {
    j_res = conf_get_ast_backup_config_info_text(name);
  }

  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

bool dialplan_update_configuration_info(const json_t* j_data)
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

  ret = strcmp(name, DEF_AST_DIALPLAN_CONFNAME);
  if(ret != 0) {
    slog(LOG_NOTICE, "The only current configuration can update.");
    return false;
  }

  ret = conf_update_ast_current_config_info_text_data(DEF_AST_DIALPLAN_CONFNAME, data);
  if(ret == false) {
    return false;
  }

  return true;
}

bool dialplan_delete_configuration_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = strcmp(name, DEF_AST_DIALPLAN_CONFNAME);
  if(ret == 0) {
    slog(LOG_NOTICE, "The current configuration is not deletable.");
    return false;
  }

  ret = conf_remove_ast_backup_config_info_valid(name, DEF_AST_DIALPLAN_CONFNAME);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete backup config info.");
    return false;
  }

  return true;
}

/**
 * Reload asterisk dialplan module
 * @return
 */
bool dialplan_reload_asterisk(void)
{
  int ret;

  ret = ami_action_moduleload("pbx_config", "reload");
  if(ret == false) {
    return false;
  }

  return true;
}


