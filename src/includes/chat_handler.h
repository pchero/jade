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

bool init_chat_handler(void);
bool term_chat_handler(void);

json_t* get_chat_rooms_by_useruuid(const char* user_uuid);
bool delete_chat_room(const char* uuid);

json_t* get_chat_userrooms_by_useruuid(const char* user_uuid);

bool create_chat_userroom(const char* uuid_user, const json_t* j_data);
bool delete_chat_userroom(const char* userroom_uuid, const char* user_uuid);

bool create_chat_message_to_userroom(const char* uuid_userroom, const char* uuid_user, const char* message);
json_t* get_chat_messages_newest(const char* uuid, const char* timestamp, const unsigned int count);

#endif /* SRC_INCLUDES_CHAT_HANDLER_H_ */
