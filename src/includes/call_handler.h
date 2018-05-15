/*
 * call_handler.h
 *
 *  Created on: Apr 13, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_CALL_HANDLER_H_
#define SRC_INCLUDES_CALL_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

bool call_hangup_by_unique_id(const char* unique_id);
bool call_originate_call_to_device(const char* source, const char* destination);

#endif /* SRC_INCLUDES_CALL_HANDLER_H_ */
