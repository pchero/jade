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

bool zmq_init_handler(void);
void zmq_term_handler(void);

bool zmq_publish_message(const char* pub_target, json_t* j_data);
void* zmq_get_context(void);
const char* zmq_get_pub_addr(void);

int zmq_send_string(void* socket, const char* data);
char* zmq_recv_string(void* socket);

#endif /* SRC_ZMQ_HANDLER_H_ */
