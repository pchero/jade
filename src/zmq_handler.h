/*
 * zmq_handler.h
 *
 *  Created on: Dec 20, 2017
 *      Author: pchero
 */

#ifndef SRC_ZMQ_HANDLER_H_
#define SRC_ZMQ_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

bool init_zmq_handler(void);
void term_zmq_handler(void);

bool publish_message(const char* pub_target, json_t* j_data);


#endif /* SRC_ZMQ_HANDLER_H_ */
