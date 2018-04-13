/*
 * call_handler.c
 *
 * Asterisk's channel resource handler.
 * And provide useful interface for call/channel.
 *  Created on: Apr 13, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>


#include "slog.h"
#include "resource_handler.h"
#include "publication_handler.h"
#include "utils.h"
#include "dialplan_handler.h"

#include "call_handler.h"


static bool init_databases(void);
static bool init_database_channel(void);


#define DEF_DB_TABLE_CALL_CHANNEL "channel"


bool call_init_handler(void)
{
  int ret;

  ret = init_databases();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database.");
    return false;
  }

  return true;
}

bool call_term_handler(void)
{
  return true;
}

bool call_reload_handler(void)
{
  int ret;

  ret = call_term_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not terminate call_handler.");
    return false;
  }

  ret = call_init_handler();
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


/**
 * create core_channel info.
 * @return
 */
bool call_create_channel_info(const json_t* j_data)
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
  ret = resource_insert_mem_item(DEF_DB_TABLE_CALL_CHANNEL, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert core_channel.");
    return false;
  }

  // publish
  // get data info
  tmp_const = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = call_get_channel_info(tmp_const);
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
 * Get all channel's array
 * @return
 */
json_t* call_get_channels_all(void)
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
json_t* call_get_channels_by_devicename(const char* device_name)
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
json_t* call_get_channel_info(const char* unique_id)
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
 * update channel info.
 * @return
 */
int call_update_channel_info(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* key;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // update
  ret = resource_update_mem_item(DEF_DB_TABLE_CALL_CHANNEL, "unique_id", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core_channel info.");
    return false;
  }

  // get updated info
  key = json_string_value(json_object_get(j_data, "unique_id"));
  j_tmp = call_get_channel_info(key);
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
int call_delete_channel_info(const char* key)
{
  json_t* j_tmp;
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = call_get_channel_info(key);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "The channel is already deleted. key[%s]", key);
    return false;
  }

  ret = resource_delete_mem_items_string(DEF_DB_TABLE_CALL_CHANNEL, "unique_id", key);
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
 * Originate call from the source device to destination device.
 * @param source
 * @param destination
 * @return
 */
bool call_originate_call_to_device(const char* source, const char* destination)
{
  json_t* j_cmd;
  char* data;
  char* channel;
  char* dpma_uuid;

  if((source == NULL) || (destination == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired call_originate_call_to_device. source[%s], destination[%s]", source, destination);

  // get default dpma for originate to device
  dpma_uuid = dialplan_get_default_dpma_originate_to_device();
  if(dpma_uuid == NULL) {
    slog(LOG_ERR, "Could not get default dpma originate to device.");
    return false;
  }

  // create data
  asprintf(&data, "aig:async,%s,%s,%s",
      DEF_DIALPLAN_JADE_AGI_NAME,
      dpma_uuid,
      destination
      );
  sfree(dpma_uuid);

  // create channel
  asprintf(&channel, "PJSIP/%s", source);

  //  Action: Originate
  //  ActionID: <value>
  //  Channel: <value>
  //  Exten: <value>
  //  Context: <value>
  //  Priority: <value>
  //  Application: <value>
  //  Data: <value>
  //  Timeout: <value>
  //  CallerID: <value>
  //  Variable: <value>
  //  Account: <value>
  //  EarlyMedia: <value>
  //  Async: <value>
  //  Codecs: <value>
  //  ChannelId: <value>
  //  OtherChannelId: <value>
  j_cmd = json_pack("{s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s}",
      "Action",         "Originate",
      "Async",          "true",
      "Channel",        channel,
      "Application",    "AGI"
      "Data",           data
      );
  sfree(channel);
  sfree(data);
  if(j_cmd == NULL) {
    slog(LOG_ERR, "Could not create default originate request info.");
    return false;
  }

  return true;
}
