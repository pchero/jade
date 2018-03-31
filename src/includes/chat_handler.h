/*
 * chat_handler.h
 *
 *  Created on: Mar 26, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_CHAT_HANDLER_H_
#define SRC_INCLUDES_CHAT_HANDLER_H_


#include <stdbool.h>
#include <jansson.h>

enum EN_CHAT_ROOM_TYPE {
  EN_CHAT_ROOM_SINGLE   = 1,
  EN_CHAT_ROOM_GROUP    = 2,
};

bool chat_init_handler(void);
bool chat_term_handler(void);

json_t* chat_get_room(const char* uuid);
json_t* chat_get_rooms_by_useruuid(const char* user_uuid);
bool chat_delete_room(const char* uuid);

json_t* chat_get_userrooms_by_useruuid(const char* user_uuid);

json_t* chat_get_userroom(const char* uuid);
bool chat_create_userroom(const char* uuid_user, const char* uuid_userroom, const json_t* j_data);
bool chat_update_userroom(const char* uuid_userroom, const json_t* j_data);
bool chat_delete_userroom(const char* uuid_userroom);

bool chat_create_message_to_userroom(const char* uuid_message, const char* uuid_userroom, const char* uuid_user, const json_t* message);
json_t* chat_get_userroom_messages_newest(const char* uuid_userroom, const char* timestamp, const unsigned int count);

json_t* chat_get_room_messages_newest(const char* uuid, const char* timestamp, const unsigned int count);
json_t* chat_get_userroom_message(const char* uuid_message, const char* uuid_userroom);
char* chat_get_uuidroom_by_uuiduserroom(const char* uuid_userroom);
json_t* chat_get_members_by_userroom(const char* uuid_userroom);

bool chat_is_user_userroom_owned(const char* uuid_user, const char* uuid_userroom);


#endif /* SRC_INCLUDES_CHAT_HANDLER_H_ */
