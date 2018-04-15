/*
 * websocket_handler.h
 *
 *  Created on: Jan 17, 2018
 *      Author: pchero
 */

#ifndef SRC_WEBSOCKET_HANDLER_H_
#define SRC_WEBSOCKET_HANDLER_H_

#include <stdbool.h>

bool websocket_init_handler(void);
void websocket_term_handler(void);


void* websocket_get_subscription_socket(const char* authtoken);

#endif /* SRC_WEBSOCKET_HANDLER_H_ */
