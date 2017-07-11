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

bool init_event_handler(void);
void term_event_handler(void);

void add_event_handler(struct event* ev);

#endif /* BACKEND_SRC_EVENT_HANDLER_H_ */
