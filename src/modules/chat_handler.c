/*
 * chat_handler.c
 *
 *  Created on: Mar 26, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <string.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "resource_handler.h"

#include "chat_handler.h"


#define DEF_DB_TABLE_CHAT_ROOM  "chat_room"
#define DEF_DB_TABLE_CHAT_USERROOM  "chat_userroom"


static bool init_chat_databases(void);
static bool init_chat_database_room(void);
static bool init_chat_database_userroom(void);

static bool db_create_table_chat_message(const char* table_name);
static bool db_delete_table_chat_message(const char* table_name);

static json_t* db_get_chat_rooms_info_by_useruuid(const char* user_uuid);
static json_t* db_get_chat_room_info(const char* uuid);
static bool db_create_chat_room_info(const json_t* j_data);
static bool db_delete_chat_room_info(const char* uuid);
static char* db_create_tablename_chat_message(const char* uuid);

static json_t* db_get_chat_userroom_info(const char* uuid);
static json_t* db_get_chat_userrooms_info_by_useruuid(const char* user_uuid);
static bool db_create_chat_userroom_info(const json_t* j_data);
static bool db_delete_chat_userroom_info(const char* uuid);


static bool db_create_chat_message_info(const char* table_name, json_t* j_data);
static json_t* db_get_chat_messages_info_newest(const char* table_name, const char* timestamp, const unsigned int count);

static bool is_exist_chat_room(const char* uuid);
static bool is_user_owned_userroom(const char* uuid_user, const char* uuid_userroom);


bool init_chat_handler(void)
{
  int ret;

  slog(LOG_DEBUG, "Fired init_chat_handler.");

  ret = init_chat_databases();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database.");
    return false;
  }

  return true;
}

bool term_chat_handler(void)
{
  // nothing to do

  return true;
}

/**
 * Initiate chat databases
 * @return
 */
static bool init_chat_databases(void)
{
  int ret;

  slog(LOG_DEBUG, "Fired init_chat_databases.");

  // init room
  ret = init_chat_database_room();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database room.");
    return false;
  }

  // init userroom
  ret = init_chat_database_userroom();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database userroom.");
    return false;
  }

  return true;
}

/**
 * Initiate chat database. room.
 * @return
 */
static bool init_chat_database_room(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_CHAT_ROOM " ("

    // basic info
    "   uuid              varchar(255),"    // uuid(chat_room)
    "   message_table     varchar(255),"    // chat message table name

    "   member  text,"    // json object of user_uuid.

    // timestamp. UTC."
    "   tm_create     datetime(6),"  // create time
    "   tm_update     datetime(6),"  // latest updated time

    "   primary key(uuid)"
    ");";

  // execute
  ret = exec_jade_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_CHAT_ROOM);
    return false;
  }

  return true;
}

/**
 * Initiate chat database. userroom.
 * @return
 */
static bool init_chat_database_userroom(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_CHAT_USERROOM " ("

    // basic info
    "   uuid          varchar(255),"
    "   uuid_user     varchar(255),"    // uuid(user)
    "   uuid_room     varchar(255),"    // uuid(chat_room)

    "   name      varchar(255),"    // room name
    "   detail    varchar(1023),"   // room detail

    "   primary key(uuid_user, uuid_room)"
    ");";

  // execute
  ret = exec_jade_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_CHAT_USERROOM);
    return false;
  }

  return true;
}

static bool db_create_table_chat_message(const char* table_name)
{
  int ret;
  char* create_table;

  if(table_name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_table_chat_message. table_name[%s]", table_name);

  asprintf(&create_table,
      "create table %s ("

      // basic info
      "   uuid              varchar(255),"    // uuid

      "   message           text,"            // message

      // timestamp. UTC."
      "   tm_create     datetime(6),"   // create time

      "   primary key(uuid)"
      ");",
      table_name
      );

  // execute
  ret = exec_jade_sql(create_table);
  sfree(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_CHAT_ROOM);
    return false;
  }

  return true;
}

/**
 * Delete chat message table of given table name.
 * @param table_name
 * @return
 */
static bool db_delete_table_chat_message(const char* table_name)
{
  int ret;
  char* drop_table;

  if(table_name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_table_chat_message. table_name[%s]", table_name);

  // todo: validate table_name

  asprintf(&drop_table, "drop table %s", table_name);

  // execute
  ret = exec_jade_sql(drop_table);
  sfree(drop_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete chat_message table. table_name[%s]", table_name);
    return false;
  }

  return true;
}

/**
 * Get chat_userroom info of given uuid.
 * @param uuid
 * @return
 */
static json_t* db_get_chat_userroom_info(const char* uuid)
{
  json_t* j_res;

  j_res = get_jade_detail_item_key_string(DEF_DB_TABLE_CHAT_USERROOM, "uuid", uuid);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get chat_userroom info. uuid[%s]", uuid);
    return NULL;
  }

  return j_res;
}

/**
 * Get list of chat_userrooms info which related given user_uuid.
 * @param uuid
 * @return
 */
static json_t* db_get_chat_userrooms_info_by_useruuid(const char* user_uuid)
{
  json_t* j_res;

  j_res = get_jade_detail_item_key_string(DEF_DB_TABLE_CHAT_USERROOM, "uuid_user", user_uuid);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get list of chat_userrooms info. user_uuid[%s]", user_uuid);
    return NULL;
  }

  return j_res;
}


static bool db_create_chat_userroom_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert info
  ret = insert_jade_item(DEF_DB_TABLE_CHAT_USERROOM, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert chat_userroom.");
    return false;
  }

  return true;
}

static bool db_delete_chat_userroom_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = delete_jade_items_string(DEF_DB_TABLE_CHAT_USERROOM, "uuid", uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete chat userroom info. uuid[%s]", uuid);
    return false;
  }

  return true;
}

static bool db_create_chat_room_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // insert info
  ret = insert_jade_item(DEF_DB_TABLE_CHAT_ROOM, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert chat_room.");
    return false;
  }

  return true;
}

/**
 * Get chat_room info of given uuid.
 * @param uuid
 * @return
 */
static json_t* db_get_chat_room_info(const char* uuid)
{
  json_t* j_res;

  j_res = get_jade_detail_item_key_string(DEF_DB_TABLE_CHAT_ROOM, "uuid", uuid);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get chat_room info. uuid[%s]", uuid);
    return NULL;
  }

  return j_res;
}

/**
 * Get all chat_rooms info which related given user_uuid.
 * @param uuid
 * @return
 */
static json_t* db_get_chat_rooms_info_by_useruuid(const char* user_uuid)
{
  char* condition;
  json_t* j_res;

  if(user_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired db_get_chat_rooms_by_useruuid. user_uuid[%s]", user_uuid);

  asprintf(&condition, "where member like '%%%s%%'", user_uuid);

  j_res = get_jade_detail_items_by_condtion(DEF_DB_TABLE_CHAT_ROOM, condition);
  sfree(condition);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get chat rooms info. user_uuid[%s]", user_uuid);
    return NULL;
  }

  return j_res;
}


static bool db_delete_chat_room_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = delete_jade_items_string(DEF_DB_TABLE_CHAT_ROOM, "uuid", uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete chat room info. uuid[%s]", uuid);
    return false;
  }

  return true;
}

/**
 * Get all chat rooms info of given useruuid.
 * @param user_uuid
 * @return
 */
json_t* get_chat_rooms_by_useruuid(const char* user_uuid)
{
  json_t* j_res;

  if(user_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_chat_rooms_by_useruuid. user_uuid[%s]", user_uuid);

  j_res = db_get_chat_rooms_info_by_useruuid(user_uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* get_chat_userrooms_by_useruuid(const char* user_uuid)
{
  json_t* j_res;

  j_res = db_get_chat_userrooms_info_by_useruuid(user_uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

bool create_chat_userroom(
    const char* uuid_user,
    const char* uuid_room,
    const char* name,
    const char* detail
    )
{
  int ret;
  json_t* j_data;
  char* uuid;

  if((uuid_user == NULL) || (uuid_room == NULL) || (name == NULL) || (detail == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_chat_userroom. uuid_user[%s], uuid_room[%s], name[%s], detail[%s]",
      uuid_user, uuid_room, name, detail
      );

  uuid = gen_uuid();
  j_data = json_pack("{"
      "s:s,"
      "s:s, s:s,"
      "s:s, s:s"
      "}",

      "uuid",       uuid,

      "uuid_user",  uuid_user,
      "uuid_room",  uuid_room,

      "name",     name,
      "detail",   detail
      );
  sfree(uuid);

  ret = db_create_chat_userroom_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create userroom info.");
    return false;
  }

  return true;
}

bool delete_chat_userroom(const char* userroom_uuid, const char* user_uuid)
{
  int ret;

  if(userroom_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_chat_userroom. uuid[%s]", userroom_uuid);

  // is owned
  ret = is_user_owned_userroom(user_uuid, userroom_uuid);
  if(ret == false) {
    slog(LOG_ERR, "User does not owned given userroom. user_uuid[%s], userroom_uuid[%s]", user_uuid, userroom_uuid);
    return false;
  }

  // delete
  ret = db_delete_chat_userroom_info(userroom_uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete userroom. userroom_uuid[%s]", userroom_uuid);
    return false;
  }

  return true;
}

/**
 * Create chat room
 * @param uuid
 * @return
 */
bool create_chat_room(json_t* j_member)
{
  char* table_name;
  char* uuid;
  int ret;
  json_t* j_tmp;
  json_t* j_tmp_member;
  char* timestamp;

  if(j_member == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  j_tmp_member = json_deep_copy(j_member);

  // create chat room uuid
  uuid = gen_uuid();
  if(uuid == NULL) {
    slog(LOG_ERR, "Could not create uuid.");
    return NULL;
  }

  // create table name for chat_message
  table_name = db_create_tablename_chat_message(uuid);

  // create table for chat_message
  ret = db_create_table_chat_message(table_name);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table for chat message. table_name[%s]", table_name);
    return false;
  }

  // create chat_room
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s,"

      "s:s "
      "}",

      "uuid",           uuid,
      "message_table",  table_name,

      "tm_create",  timestamp
      );
  json_object_set_new(j_tmp, "member", j_tmp_member);
  sfree(uuid);

  // create chat_room info
  ret = db_create_chat_room_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create chat_room.");
    db_delete_table_chat_message(table_name);
    sfree(table_name);
    return false;
  }
  sfree(table_name);

  return true;
}

/**
 * Delete chat room of given uuid.
 * Delete also related chat info.
 * @param uuid
 * @return
 */
bool delete_chat_room(const char* uuid)
{
  int ret;
  json_t* j_tmp;
  const char* message_table;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_chat_room. uuid[%s]", uuid);

  j_tmp = db_get_chat_room_info(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get chat_room info. uuid[%s]", uuid);
    return false;
  }

  // get message table name
  message_table = json_string_value(json_object_get(j_tmp, "message_table"));
  if(message_table != NULL) {
    // delete
    db_delete_table_chat_message(message_table);
  }
  json_decref(j_tmp);

  // delete chat room
  ret = db_delete_chat_room_info(uuid);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete chat room. uuid[%s]", uuid);
    return false;
  }

  return true;
}

/**
 * Create chat message(Add the given message to chat room).
 * @param uuid
 * @param j_data
 * @return
 */
bool create_chat_message(const char* uuid, const char* message)
{
  int ret;
  char* timestamp;
  json_t* j_chat;
  json_t* j_data;
  const char* message_table;
  char* uuid_message;

  if((uuid == NULL) || (message == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check existence
  ret = is_exist_chat_room(uuid);
  if(ret == false) {
    slog(LOG_ERR, "The given chat room is not exist. uuid[%s]", uuid);
    return false;
  }

  // get chat room info
  j_chat = db_get_chat_room_info(uuid);
  if(j_chat == NULL) {
    slog(LOG_ERR, "Could not get chat room info. uuid[%s]", uuid);
    return false;
  }

  // get message table
  message_table = json_string_value(json_object_get(j_chat, "message_table"));
  if(message_table == NULL) {
    slog(LOG_ERR, "Could not get message_table info.");
    json_decref(j_chat);
    return false;
  }

  // create data info
  uuid_message = gen_uuid();
  timestamp = get_utc_timestamp();
  j_data = json_pack("{"
      "s:s, s:s,"
      "s:s "
      "}"

      "uuid",       uuid_message,
      "message",    message,

      "tm_create",    timestamp
      );
  sfree(timestamp);
  sfree(uuid_message);

  // create message
  ret = db_create_chat_message_info(message_table, j_data);
  json_decref(j_chat);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create message info.");
    return false;
  }

  return true;
}

/**
 * Get chat messages of given chat room uuid.
 * Order by newest first of given timestamp.
 * @param uuid
 * @param count
 * @return
 */
json_t* get_chat_messages_newest(const char* uuid, const char* timestamp, const unsigned int count)
{
  int ret;
  json_t* j_chat;
  json_t* j_res;
  const char* table_name;

  if((uuid == NULL) || (timestamp == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
  }

  // check existence
  ret = is_exist_chat_room(uuid);
  if(ret == false) {
    slog(LOG_ERR, "The given chat room is not exist. uuid[%s]", uuid);
    return NULL;
  }

  // get chat_room
  j_chat = db_get_chat_room_info(uuid);
  if(j_chat == NULL) {
    slog(LOG_ERR, "Could not get chat room info.");
    return NULL;
  }

  table_name = json_string_value(json_object_get(j_chat, "message_table"));

  // get messages
  j_res = db_get_chat_messages_info_newest(table_name, timestamp, count);
  json_decref(j_chat);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get chat messages. uuid[%s]", uuid);
    return NULL;
  }

  return j_res;
}

static bool db_create_chat_message_info(const char* table_name, json_t* j_data)
{
  int ret;

  if((table_name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // todo: need to validate table_name

  // insert info
  ret = insert_jade_item(table_name, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert chat_room.");
    return false;
  }

  return true;
}

/**
 *
 * @param table_name
 * @param timestamp
 * @param count
 * @return
 */
static json_t* db_get_chat_messages_info_newest(const char* table_name, const char* timestamp, const unsigned int count)
{
  json_t* j_res;
  char* condition;

  if((table_name == NULL) || (timestamp == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // create condition
  asprintf(&condition, "where tm_create < '%s' order by tm_create desc limit %u", timestamp, count);

  j_res = get_jade_detail_items_by_condtion(table_name, condition);
  sfree(condition);

  return j_res;
}

/**
 * Create string for chat_message table name.
 * @param uuid
 * @return
 */
static char* db_create_tablename_chat_message(const char* uuid)
{
  char* res;
  char* tmp;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  tmp = string_replace_char(uuid, '-', '_');
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not create table name.");
    return NULL;
  }

  asprintf(&res, "chat_%s_message", tmp);
  sfree(tmp);

  return res;
}

static bool is_exist_chat_room(const char* uuid)
{
  json_t* j_tmp;

  j_tmp = db_get_chat_room_info(uuid);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

/**
 * Return true if given user owned given userroom.
 * @param uuid_user
 * @param uuid_room
 * @return
 */
static bool is_user_owned_userroom(const char* uuid_user, const char* uuid_userroom)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if((uuid_user == NULL) || (uuid_userroom == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = db_get_chat_userroom_info(uuid_userroom);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get userroom info. uuid_user[%s], uuid_room[%s]", uuid_user, uuid_userroom);
    return false;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid_user"));
  if(tmp_const == NULL) {
    json_decref(j_tmp);
    return false;
  }

  ret = strcmp(tmp_const, uuid_user);
  json_decref(j_tmp);
  if(ret != 0) {
    return false;
  }

  return true;
}
