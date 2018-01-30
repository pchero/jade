/*
 * agent_handler.h
 *
 *  Created on: Dec 12, 2017
 *      Author: pchero
 */

#ifndef SRC_AGENT_HANDLER_H_
#define SRC_AGENT_HANDLER_H_

#include <evhtp.h>

void htp_get_agent_agents(evhtp_request_t *req, void *data);
void htp_get_agent_agents_detail(evhtp_request_t *req, void *data);


#endif /* SRC_AGENT_HANDLER_H_ */
