/*
 * core_handler.h
 *
 *  Created on: Dec 12, 2017
 *      Author: pchero
 */

#ifndef SRC_CORE_HANDLER_H_
#define SRC_CORE_HANDLER_H_

#include <evhtp.h>


void htp_get_core_channels_detail(evhtp_request_t *req, void *data);
void htp_get_core_channels(evhtp_request_t *req, void *data);

void htp_get_core_systems_detail(evhtp_request_t *req, void *data);
void htp_get_core_systems(evhtp_request_t *req, void *data);



#endif /* SRC_CORE_HANDLER_H_ */
