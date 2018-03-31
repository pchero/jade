/*
 * action_handler.h
 *
 *  Created on: Feb 15, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_ACTION_HANDLER_H_
#define BACKEND_SRC_ACTION_HANDLER_H_

#include <stdio.h>
#include <stdbool.h>
#include <jansson.h>

typedef enum _ACTION_RES {
  ACTION_RES_COMPLETE = 1,
  ACTION_RES_CONTINUE = 2,
  ACTION_RES_ERROR    = 3,
} ACTION_RES;

bool action_insert(const char* id, const char* type, const json_t* j_data);
bool action_delete(const char* id);
json_t* action_get_and_delete(const char* id);
json_t* action_get(const char* id);


#endif /* BACKEND_SRC_ACTION_HANDLER_H_ */
