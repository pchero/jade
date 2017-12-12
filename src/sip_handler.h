/*
 * sip_handler.h
 *
 *  Created on: Dec 12, 2017
 *      Author: pchero
 */

#ifndef SRC_SIP_HANDLER_H_
#define SRC_SIP_HANDLER_H_

#include <evhtp.h>

void htp_get_sip_peers_detail(evhtp_request_t *req, void *data);
void htp_get_sip_peers(evhtp_request_t *req, void *data);

void htp_get_sip_registries(evhtp_request_t *req, void *data);
void htp_get_sip_registries_detail(evhtp_request_t *req, void *data);

#endif /* SRC_SIP_HANDLER_H_ */
