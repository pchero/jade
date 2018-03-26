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

bool init_chat_handler(void);
bool term_chat_handler(void);

json_t* get_chat_rooms_by_useruuid(const char* user_uuid);
bool create_chat_room(json_t* j_member);
bool delete_chat_room(const char* uuid);

json_t* get_chat_userrooms_by_useruuid(const char* user_uuid);
bool create_chat_userroom(const char* uuid_user, const char* uuid_room, const char* name, const char* detail);
bool delete_chat_userroom(const char* userroom_uuid, const char* user_uuid);

bool create_chat_message(const char* uuid, const char* message);
json_t* get_chat_messages_newest(const char* uuid, const char* timestamp, const unsigned int count);

#endif /* SRC_INCLUDES_CHAT_HANDLER_H_ */
