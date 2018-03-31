/*
 * event_handler.h
 *
 *  Created on: Feb 5, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_EVENT_HANDLER_H_
#define BACKEND_SRC_EVENT_HANDLER_H_

#include <stdbool.h>
#include <event.h>

bool event_init_handler(void);
void event_term_handler(void);

void event_add_handler(struct event* ev);

#endif /* BACKEND_SRC_EVENT_HANDLER_H_ */
