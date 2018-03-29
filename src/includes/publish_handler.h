/*
 * publish_handler.h
 *
 *  Created on: Dec 22, 2017
 *      Author: pchero
 */

#ifndef SRC_PUBLISH_HANDLER_H_
#define SRC_PUBLISH_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

#define DEF_PUB_TYPE_CREATE   "create"
#define DEF_PUB_TYPE_UPDATE   "update"
#define DEF_PUB_TYPE_DELETE   "delete"

enum EN_PUBLISH_TYPES {
  EN_PUBLISH_CREATE   = 1,
  EN_PUBLISH_UPDATE   = 2,
  EN_PUBLISH_DELETE   = 3,
};


bool publish_event_me_chat_message(enum EN_PUBLISH_TYPES type, const char* uuid_user, json_t* j_data);
bool publish_event_me_chat_room(enum EN_PUBLISH_TYPES type, const char* uuid_user, json_t* j_data);

bool publish_event_core_channel(const char* type, json_t* j_data);
bool publish_event_core_agi(const char* type, json_t* j_data);
bool publish_event_core_module(const char* type, json_t* j_data);

bool publish_event_dp_dialplan(const char* type, json_t* j_data);
bool publish_event_dp_dpma(const char* type, json_t* j_data);

bool publish_event_park_parkedcall(const char* type, json_t* j_data);
bool publish_event_park_parkinglot(const char* type, json_t* j_data);

bool publish_event_pjsip_aor(const char* type, json_t* j_data);
bool publish_event_pjsip_auth(const char* type, json_t* j_data);
bool publish_event_pjsip_contact(const char* type, json_t* j_data);
bool publish_event_pjsip_endpoint(const char* type, json_t* j_data);


bool publish_event_queue_member(const char* type, json_t* j_data);
bool publish_event_queue_queue(const char* type, json_t* j_data);
bool publish_event_queue_entry(const char* type, json_t* j_data);


bool publish_event_sip_peer(const char* type, json_t* j_data);
bool publish_event_sip_registry(const char* type, json_t* j_data);


#endif /* SRC_PUBLISH_HANDLER_H_ */
