/*
 * dialplan_handler.h
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#ifndef SRC_DIALPLAN_HANDLER_H_
#define SRC_DIALPLAN_HANDLER_H_


#include <evhtp.h>


void htp_get_dialplan_setting(evhtp_request_t *req, void *data);
void htp_put_dialplan_setting(evhtp_request_t *req, void *data);


#endif /* SRC_DIALPLAN_HANDLER_H_ */
