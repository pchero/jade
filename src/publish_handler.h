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

bool publish_event_core_channel(const char* type, json_t* j_data);
bool publish_event_core_agi(const char* type, json_t* j_data);
bool publish_event_core_module(const char* type, json_t* j_data);


bool publish_event_park_parkedcall(const char* type, json_t* j_data);
bool publish_event_park_parkinglot(const char* type, json_t* j_data);

bool publish_event_queue_member(const char* type, json_t* j_data);
bool publish_event_queue_queue(const char* type, json_t* j_data);
bool publish_event_queue_entry(const char* type, json_t* j_data);



#endif /* SRC_PUBLISH_HANDLER_H_ */
