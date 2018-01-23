/*
 * dialplan_handler.h
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#ifndef SRC_DIALPLAN_HANDLER_H_
#define SRC_DIALPLAN_HANDLER_H_


#include <evhtp.h>

void htp_get_dp_dpmas(evhtp_request_t *req, void *data);
void htp_post_dp_dpmas(evhtp_request_t *req, void *data);

void htp_get_dp_dpmas_detail(evhtp_request_t *req, void *data);
void htp_put_dp_dpmas_detail(evhtp_request_t *req, void *data);
void htp_delete_dp_dpmas_detail(evhtp_request_t *req, void *data);

void htp_get_dp_dialplans(evhtp_request_t *req, void *data);
void htp_post_dp_dialplans(evhtp_request_t *req, void *data);

void htp_get_dp_dialplans_detail(evhtp_request_t *req, void *data);
void htp_put_dp_dialplans_detail(evhtp_request_t *req, void *data);
void htp_delete_dp_dialplans_detail(evhtp_request_t *req, void *data);

void htp_get_dp_setting(evhtp_request_t *req, void *data);
void htp_put_dp_setting(evhtp_request_t *req, void *data);


#endif /* SRC_DIALPLAN_HANDLER_H_ */
