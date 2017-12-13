/*
 * pjsip_handler.h
 *
 *  Created on: Dec 13, 2017
 *      Author: pchero
 */

#ifndef SRC_PJSIP_HANDLER_H_
#define SRC_PJSIP_HANDLER_H_

#include <evhtp.h>

void htp_get_pjsip_endpoints(evhtp_request_t *req, void *data);
void htp_get_pjsip_endpoints_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_aors(evhtp_request_t *req, void *data);
void htp_get_pjsip_aors_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_auths(evhtp_request_t *req, void *data);
void htp_get_pjsip_auths_detail(evhtp_request_t *req, void *data);



#endif /* SRC_PJSIP_HANDLER_H_ */
