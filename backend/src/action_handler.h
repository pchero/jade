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

bool insert_action(const char* id, const char* type);
json_t* get_action_and_delete(const char* id);

#endif /* BACKEND_SRC_ACTION_HANDLER_H_ */
