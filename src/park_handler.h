/*
 * park_handler.h
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */

#ifndef SRC_PARK_HANDLER_H_
#define SRC_PARK_HANDLER_H_

#include <evhtp.h>

void htp_get_park_parkinglots(evhtp_request_t *req, void *data);
void htp_get_park_parkinglots_detail(evhtp_request_t *req, void *data);
void htp_get_park_parkedcalls(evhtp_request_t *req, void *data);
void htp_get_park_parkedcalls_detail(evhtp_request_t *req, void *data);


#endif /* SRC_PARK_HANDLER_H_ */
