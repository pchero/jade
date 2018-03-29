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
#include "user_handler.h"
#include "publish_handler.h"

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
static json_t* db_get_chat_room_info_by_type_members(const enum EN_CHAT_ROOM_TYPE type, const json_t* j_members);
static bool db_create_chat_room_info(const json_t* j_data);
static bool db_update_chat_room_info(json_t* j_data);
static bool db_delete_chat_room_info(const char* uuid);
static char* db_create_tablename_chat_message(const char* uuid);

static json_t* db_get_chat_userroom_info(const char* uuid);
static json_t* db_get_chat_userroom_info_by_user_room(const char* uuid_user, const char* uuid_room);
static json_t* db_get_chat_userrooms_info_by_useruuid(const char* user_uuid);
static bool db_create_chat_userroom_info(const json_t* j_data);
static bool db_update_chat_userroom_info(const json_t* j_data);
static bool db_delete_chat_userroom_info(const char* uuid);

static bool db_create_chat_message_info(const char* table_name, json_t* j_data);
static json_t* db_get_chat_messages_info_newest(const char* table_name, const char* timestamp, const unsigned int count);

static bool create_message(const char* uuid_room, const char* uuid_user, const char* message);

static bool create_userroom(const json_t* j_data);
static bool update_userroom(const json_t* j_data);
static bool delete_userroom(const char* uuid);


static bool create_room(const char* uuid, const char* uuid_user, enum EN_CHAT_ROOM_TYPE type, const json_t* j_members);
static json_t* get_room_by_type_members(enum EN_CHAT_ROOM_TYPE type, json_t* j_members);
static bool delete_room_member(const char* uuid_room, const char* uuid_user);

static char* get_room_uuid_by_type_members(enum EN_CHAT_ROOM_TYPE type, json_t* j_members);

static bool is_room_exist(const char* uuid);
static bool is_userroom_exist(const char* uuid);
static bool is_userroom_exist_by_user_room(const char* uuid_user, const char* uuid_room);
static bool is_room_exist_by_type_members(enum EN_CHAT_ROOM_TYPE type, const json_t* j_members);

static bool is_user_owned_userroom(const char* uuid_user, const char* uuid_userroom);
static bool is_user_in_room(json_t* j_room, const char* uuid);

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
    "   type              int,"             // chat room type. see EN_CHAT_ROOM_TYPE

    // owner, creator
    "   uuid_creator      varchar(255),"    // creator user uuid
    "   uuid_owner        varchar(255),"    // owner user uuid

    "   members text,"    // json array of user_uuid.

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

    // timestamp. UTC."
    "   tm_create     datetime(6),"  // create time
    "   tm_update     datetime(6),"  // latest updated time

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

      "   uuid_owner        varchar(255),"    // message owner's uuid
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
    slog(LOG_ERR, "Could not initiate database. database[%s]", table_name);
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
 * Get chat_userroom info of given uuid.
 * @param uuid
 * @return
 */
static json_t* db_get_chat_userroom_info_by_user_room(const char* uuid_user, const char* uuid_room)
{
  json_t* j_res;
  json_t* j_data;

  if((uuid_user == NULL) || (uuid_room == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_data = json_pack("{"
      "s:s, s:s "
      "}",

      "uuid_user",    uuid_user,
      "uuid_room",    uuid_room
      );

  j_res = get_jade_detail_item_by_obj(DEF_DB_TABLE_CHAT_USERROOM, j_data);
  json_decref(j_data);
  if(j_res == NULL) {
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

  if(user_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = get_jade_detail_items_key_string(DEF_DB_TABLE_CHAT_USERROOM, "uuid_user", user_uuid);
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

static bool db_update_chat_userroom_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // update
  ret = update_jade_item(DEF_DB_TABLE_CHAT_USERROOM, "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update chat_userroom.");
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

static json_t* db_get_chat_room_info_by_type_members(const enum EN_CHAT_ROOM_TYPE type, const json_t* j_members)
{
  json_t* j_res;
  json_t* j_data;
  json_t* j_tmp;

  if(j_members == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // sort member
  j_tmp = sort_resource_json_array_string(j_members, EN_SORT_ASC);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not sort members info.");
    return NULL;
  }

  j_data = json_pack("{"
      "s:i, s:o "
      "}",

      "type",     type,
      "members",  j_tmp
      );

  j_res = get_jade_detail_item_by_obj(DEF_DB_TABLE_CHAT_ROOM, j_data);
  json_decref(j_data);
  if(j_res == NULL) {
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

static bool db_update_chat_room_info(json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = update_jade_item(DEF_DB_TABLE_CHAT_ROOM, "uuid", j_data);
  if(ret == false) {
    return false;
  }

  return true;
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
 * Get chat room info.
 * @param uuid
 * @return
 */
json_t* get_chat_room(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_chat_room. uuid[%s]", uuid);

  j_res = db_get_chat_room_info(uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
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

/**
 * Interface for get userroom info.
 * @param uuid
 * @return
 */
json_t* get_chat_userroom(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = db_get_chat_userroom_info(uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Interface for create new chat_userroom
 * j_data : {"name":"<string>", "detail":"<string>", "type": <integer>, "members": ["<string>",]}
 * @param uuid_user
 * @param j_data
 * @return
 */
bool create_chat_userroom(const char* uuid_user, const char* uuid_userroom, const json_t* j_data)
{
  int ret;
  int ret_room;
  int type;
  json_t* j_members;
  json_t* j_tmp;
  char* uuid_room;
  char* tmp;

  if((uuid_user == NULL) || (uuid_userroom == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check user existence
  ret = is_user_user_exist(uuid_user);
  if(ret == false) {
    slog(LOG_NOTICE, "The given user is not exist. uuid[%s]", uuid_user);
    return false;
  }

  // get member
  j_members = json_object_get(j_data, "members");
  if(j_members == NULL) {
    slog(LOG_NOTICE, "Could not get member info.");
    return false;
  }

  // get type
  type = json_integer_value(json_object_get(j_data, "type"));
  if(type == 0) {
    slog(LOG_NOTICE, "Could not get type info.");
    return false;
  }

  // check room existence
  ret = is_room_exist_by_type_members(type, j_members);
  if(ret == false) {
    // if not exist, create new
    tmp = gen_uuid();
    ret_room = create_room(tmp, uuid_user, type, j_members);
    sfree(tmp);
    if(ret_room == false) {
      slog(LOG_ERR, "Could not create room.");
      return false;
    }
  }

  // get room uuid.
  uuid_room = get_room_uuid_by_type_members(type, j_members);
  if(uuid_room == NULL) {
    slog(LOG_ERR, "Could not get room uuid.");
    return false;
  }

  // check existence
  ret = is_userroom_exist_by_user_room(uuid_user, uuid_room);
  if(ret == true) {
    slog(LOG_NOTICE, "The same userroom is already exist.");
    return false;
  }

  // create userroom request
  j_tmp = json_pack("{"
      "s:s, s:s, s:s,"
      "s:s, s:s"
      "}",

      "uuid",         uuid_userroom,
      "uuid_room",    uuid_room,
      "uuid_user",    uuid_user,

      "name",   json_string_value(json_object_get(j_data, "name"))? : "",
      "detail", json_string_value(json_object_get(j_data, "detail"))? : ""
      );
  sfree(uuid_room);

  // create userroom
  ret = create_userroom(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create userroom info.");
    delete_chat_room(uuid_room);
    return false;
  }

  return true;
}

/**
 * Interface for create new chat_userroom
 * @param uuid_user
 * @param j_data
 * @return
 */
bool update_chat_userroom(const char* uuid_user, const char* uuid_userroom, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;

  if((uuid_user == NULL) || (uuid_userroom == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check user existence
  ret = is_user_user_exist(uuid_user);
  if(ret == false) {
    slog(LOG_NOTICE, "The given user is not exist. uuid[%s]", uuid_user);
    return false;
  }

  j_tmp = db_get_chat_userroom_info(uuid_userroom);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get userroom info. uuid[%s]", uuid_userroom);
    return false;
  }

  json_object_set(j_tmp, "name", json_object_get(j_data, "name"));
  json_object_set(j_tmp, "detail", json_object_get(j_data, "detail"));

  ret = update_userroom(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

///**
// * Create userroom info and room.
// * @param uuid_user
// * @param j_userroom
// * @return
// */
//static bool create_userroom_and_room(const char* uuid_user, const char* uuid_userroom, const json_t* j_data)
//{
//  int ret;
//  int type;
//  json_t* j_members;
//  char* uuid_room;
//  const char* name;
//  const char* detail;
//
//  if((uuid_user == NULL) || (uuid_userroom == NULL) || (j_data == NULL)) {
//    slog(LOG_WARNING, "Wrong input parameter.");
//    return false;
//  }
//  slog(LOG_DEBUG, "Fired create_userroom_and_room.");
//
//  // get type
//  type = json_integer_value(json_object_get(j_data, "type"));
//  if(type == 0) {
//    slog(LOG_ERR, "Could not get type info.");
//    return false;
//  }
//
//  // get member
//  j_members = json_object_get(j_data, "members");
//  if(j_members == NULL) {
//    slog(LOG_ERR, "Could not get member info.");
//    return false;
//  }
//
//  // create uuid for room.
//  uuid_room = gen_uuid();
//
//  // create room
//  ret = create_room(uuid_room, uuid_user, type, j_members);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create chat room info.");
//    sfree(uuid_room);
//    return false;
//  }
//
//  // get info
//  name = json_string_value(json_object_get(j_data, "name"));
//  detail = json_string_value(json_object_get(j_data, "detail"));
//
//  // create userroom
//  ret = create_userroom(uuid_user, uuid_room, name, detail);
//  sfree(uuid_room);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create userroom info.");
//    return false;
//  }
//
//  return true;
//}
//
//static bool create_userroom_by_room(const char* uuid_user, const json_t* j_data, const json_t* j_room)
//{
//  int ret;
//  const char* uuid_room;
//  const char* name;
//  const char* detail;
//
//  if((uuid_user == NULL) || (j_data == NULL) || (j_room == NULL)) {
//    slog(LOG_WARNING, "Wrong input parameter.");
//    return false;
//  }
//  slog(LOG_DEBUG, "Fired create_userroom_by_room.");
//
//  uuid_room = json_string_value(json_object_get(j_room, "uuid"));
//  if(uuid_room == NULL) {
//    slog(LOG_ERR, "Could not get room uuid.");
//    return false;
//  }
//
//  // check existence
//  ret = is_userroom_exist_by_user_room(uuid_user, uuid_room);
//  if(ret == true) {
//    slog(LOG_ERR, "The userroom is already exist.");
//    return false;
//  }
//
//  name = json_string_value(json_object_get(j_data, "name"));
//  detail = json_string_value(json_object_get(j_data, "detail"));
//
//  ret = create_userroom(uuid_user, uuid_room, name, detail);
//  if(ret == false) {
//    slog(LOG_ERR, "Could not create userroom.");
//    return false;
//  }
//
//  return true;
//}

/**
 * Create userroom info.
 * @return
 */
static bool create_userroom(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_chat_userroom.");

  j_tmp = json_deep_copy(j_data);

  timestamp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(timestamp));
  sfree(timestamp);

  ret = db_create_chat_userroom_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create userroom info.");
    return false;
  }

  return true;
}

static bool update_userroom(const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;

  if(j_data == NULL) {
    return false;
  }

  j_tmp = json_deep_copy(j_data);

  timestamp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(timestamp));
  sfree(timestamp);

  ret = db_update_chat_userroom_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool delete_userroom(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_delete_chat_userroom_info(uuid);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Delete chat userroom.
 * Delete also related info.
 * @param uuid_user
 * @param uuid_userroom
 * @return
 */
bool delete_chat_userroom(const char* uuid_user, const char* uuid_userroom)
{
  int ret;
  const char* uuid_room;
  json_t* j_userroom;

  if(uuid_userroom == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_chat_userroom. uuid[%s]", uuid_userroom);

  // is owned
  ret = is_user_owned_userroom(uuid_user, uuid_userroom);
  if(ret == false) {
    slog(LOG_ERR, "User does not owned given userroom. user_uuid[%s], userroom_uuid[%s]", uuid_user, uuid_userroom);
    return false;
  }

  // get userroom
  j_userroom = db_get_chat_userroom_info(uuid_userroom);
  if(j_userroom == NULL) {
    slog(LOG_ERR, "Could not get userroom info. uuid[%s]", uuid_userroom);
    return false;
  }

  // get uur_room
  uuid_room = json_string_value(json_object_get(j_userroom, "uuid_room"));
  if(uuid_room == NULL) {
    slog(LOG_ERR, "Could not get room uuid info.");
    json_decref(j_userroom);
    return false;
  }

  // delete member
  ret = delete_room_member(uuid_room, uuid_user);
  if(ret == false) {
    slog(LOG_ERR, "Could not dlete room member info.");
    json_decref(j_userroom);
    return false;
  }

  // delete userroom
  ret = delete_userroom(uuid_userroom);
  json_decref(j_userroom);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete userroom info.");
    return false;
  }

  return true;
}

/**
 * Create chat room
 * @param uuid
 * @return
 */
static bool create_room(
    const char* uuid,
    const char* uuid_user,
    enum EN_CHAT_ROOM_TYPE type,
    const json_t* j_members
    )
{
  char* table_name;
  int ret;
  json_t* j_tmp;
  json_t* j_tmp_members;
  char* timestamp;

  if((uuid == NULL) || (uuid_user == NULL) || (j_members == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check room existence
  ret = is_room_exist(uuid);
  if(ret == true) {
    slog(LOG_NOTICE, "The given chat room is already exsit.");
    return false;
  }

  // sort members
  j_tmp_members = sort_resource_json_array_string(j_members, EN_SORT_ASC);
  if(j_tmp_members == NULL) {
    slog(LOG_ERR, "Could not get sort members info.");
    return false;
  }

  // check room member existenece
  ret = is_room_exist_by_type_members(type, j_tmp_members);
  if(ret == true) {
    // already exist.
    slog(LOG_NOTICE, "The room member is already exist.");
    json_decref(j_tmp_members);
    return false;
  }

  // create table name for chat_message
  table_name = db_create_tablename_chat_message(uuid);
  if(table_name == NULL) {
    slog(LOG_ERR, "Could not create table name.");
    json_decref(j_tmp_members);
    return false;
  }

  // create table for chat_message
  ret = db_create_table_chat_message(table_name);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table for chat message. table_name[%s]", table_name);
    json_decref(j_tmp_members);
    sfree(table_name);
    return false;
  }

  // create request data
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:s, s:i, s:o, "

      "s:s "
      "}",

      "uuid",           uuid,
      "uuid_creator",   uuid_user,
      "uuid_owner",     uuid_user,

      "message_table",  table_name,
      "type",           type,
      "members",        j_tmp_members,

      "tm_create",  timestamp
      );
  sfree(timestamp);

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

static json_t* get_room_by_type_members(enum EN_CHAT_ROOM_TYPE type, json_t* j_members)
{
  json_t* j_res;

  if(j_members == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = db_get_chat_room_info_by_type_members(type, j_members);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
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
static bool create_message(const char* uuid_room, const char* uuid_user, const char* message)
{
  int ret;
  char* timestamp;
  json_t* j_room;
  json_t* j_data;
  const char* message_table;
  char* uuid_message;

  if((uuid_room == NULL) || (uuid_user == NULL) || (message == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check chat room existence
  ret = is_room_exist(uuid_room);
  if(ret == false) {
    slog(LOG_ERR, "The given chat room is not exist. uuid_room[%s]", uuid_room);
    return false;
  }

  // check user existence
  ret = is_user_user_exist(uuid_user);
  if(ret == false) {
    slog(LOG_ERR, "The given user user is not exist. user_uuid[%s]", uuid_user);
    return false;
  }

  // get chat room info
  j_room = db_get_chat_room_info(uuid_room);
  if(j_room == NULL) {
    slog(LOG_ERR, "Could not get chat room info. uuid[%s]", uuid_room);
    return false;
  }

  // check user in the room
  ret = is_user_in_room(j_room, uuid_user);
  if(ret == false) {
    json_decref(j_room);
    slog(LOG_WARNING, "The user is not in the room.");
    return false;
  }

  // get message table
  message_table = json_string_value(json_object_get(j_room, "message_table"));
  if(message_table == NULL) {
    slog(LOG_ERR, "Could not get message_table info.");
    json_decref(j_room);
    return false;
  }

  // create data info
  uuid_message = gen_uuid();
  timestamp = get_utc_timestamp();
  j_data = json_pack("{"
      "s:s, s:s, s:s, "
      "s:s "
      "}"

      "uuid",       uuid_message,
      "uuid_owner", uuid_user,
      "message",    message,

      "tm_create",    timestamp
      );
  sfree(timestamp);
  sfree(uuid_message);

  // create message
  ret = db_create_chat_message_info(message_table, j_data);
  json_decref(j_room);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create message info.");
    return false;
  }

  return true;
}

/**
 * Create(Add) the message to the given userroom.
 * @param uuid_userroom
 * @param uuid_user
 * @param message
 * @return
 */
bool create_chat_message_to_userroom(const char* uuid_userroom, const char* uuid_user, const char* message)
{
  int ret;
  const char* uuid_room;
  json_t* j_userroom;

  if((uuid_userroom == NULL) || (uuid_user == NULL) || (message == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = is_userroom_exist(uuid_userroom);
  if(ret == false) {
    slog(LOG_ERR, "The given userroom info is not exist. uuid[%s]", uuid_userroom);
    return false;
  }

  // get userroom info
  j_userroom = db_get_chat_userroom_info(uuid_userroom);
  if(j_userroom == NULL) {
    slog(LOG_ERR, "Could not get userroom info. uuid[%s]", uuid_userroom);
    return false;
  }

  // get room uuid
  uuid_room = json_string_value(json_object_get(j_userroom, "uuid_room"));
  if(uuid_room == NULL) {
    slog(LOG_ERR, "Could not get chat room uuid.");
    json_decref(j_userroom);
    return false;
  }

  // create message
  ret = create_message(uuid_room, uuid_user, message);
  json_decref(j_userroom);
  if(ret == false) {
    slog(LOG_ERR, "Could not create chat message. uuid_room[%s], uuid_user[%s]", uuid_room, uuid_user);
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
  ret = is_room_exist(uuid);
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

static bool is_room_exist(const char* uuid)
{
  json_t* j_tmp;

  j_tmp = db_get_chat_room_info(uuid);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

static bool is_userroom_exist(const char* uuid)
{
  json_t* j_tmp;

  j_tmp = db_get_chat_userroom_info(uuid);
  if(j_tmp == NULL) {
    return false;
  }

  return true;
}

/**
 * Return true if the userroom info is exist of given user's uuid and room's uuid.
 * @param uuid_user
 * @param uuid_room
 * @return
 */
static bool is_userroom_exist_by_user_room(const char* uuid_user, const char* uuid_room)
{
  json_t* j_tmp;

  if((uuid_user == NULL) || (uuid_room == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = db_get_chat_userroom_info_by_user_room(uuid_user, uuid_room);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

static bool is_user_in_room(json_t* j_room, const char* uuid)
{
  json_t* j_tmp;

  if((j_room == NULL) || (uuid == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_object_get(json_object_get(j_room, "member"), uuid);
  if(j_tmp == NULL) {
    return false;
  }

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

/**
 * Return true if there is same members of chat room
 * @param uuid_user
 * @param j_member
 * @return
 */
static bool is_room_exist_by_type_members(enum EN_CHAT_ROOM_TYPE type, const json_t* j_members)
{
  json_t* j_tmp;

  if(j_members == NULL) {
    return false;
  }

  j_tmp = db_get_chat_room_info_by_type_members(type, j_members);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

/**
 * Remove member info of given uuid_user from given uuid_room.
 * @param uuid_room
 * @param uuid_user
 * @return
 */
static bool delete_room_member(const char* uuid_room, const char* uuid_user)
{
  int ret;
  json_t* j_room;
  json_t* j_members;
  json_t* j_member;
  const char* tmp_const;
  int idx;

  if((uuid_room == NULL) || (uuid_user == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get room info
  j_room = get_chat_room(uuid_room);
  if(j_room == NULL) {
    slog(LOG_ERR, "Could not get room info.");
    return false;
  }

  // get members info
  j_members = json_object_get(j_room, "members");
  if(j_members == NULL) {
    slog(LOG_WARNING, "Could not get members info.");
    json_decref(j_room);
    return false;
  }

  // delete member
  json_array_foreach(j_members, idx, j_member) {
    tmp_const = json_string_value(j_member);
    if(tmp_const == NULL) {
      continue;
    }

    ret = strcmp(tmp_const, uuid_user);
    if(ret != 0) {
      continue;
    }

    // delete
    json_array_remove(j_room, idx);
    break;
  }

  // update
  ret = db_update_chat_room_info(j_room);
  json_decref(j_room);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Return the string of room_uuid of gvien type and members
 * @param type
 * @param j_members
 * @return
 */
static char* get_room_uuid_by_type_members(enum EN_CHAT_ROOM_TYPE type, json_t* j_members)
{
  char* res;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_members == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get room info
  j_tmp = get_room_by_type_members(type, j_members);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get room info.");
    return NULL;
  }

  // get room_uuid
  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    json_decref(j_tmp);
    slog(LOG_ERR, "Could not get room uuid.");
    return NULL;
  }

  res = strdup(tmp_const);
  json_decref(j_tmp);

  return res;
}
