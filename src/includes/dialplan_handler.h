/*
 * dialplan_handler.h
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#ifndef SRC_DIALPLAN_HANDLER_H_
#define SRC_DIALPLAN_HANDLER_H_


#include <evhtp.h>

void dialplan_htp_get_dp_dpmas(evhtp_request_t *req, void *data);
void dialplan_htp_post_dp_dpmas(evhtp_request_t *req, void *data);

void dialplan_htp_get_dp_dpmas_detail(evhtp_request_t *req, void *data);
void dialplan_htp_put_dp_dpmas_detail(evhtp_request_t *req, void *data);
void dialplan_htp_delete_dp_dpmas_detail(evhtp_request_t *req, void *data);

void dialplan_htp_get_dp_dialplans(evhtp_request_t *req, void *data);
void dialplan_htp_post_dp_dialplans(evhtp_request_t *req, void *data);

void dialplan_htp_get_dp_dialplans_detail(evhtp_request_t *req, void *data);
void dialplan_htp_put_dp_dialplans_detail(evhtp_request_t *req, void *data);
void dialplan_htp_delete_dp_dialplans_detail(evhtp_request_t *req, void *data);

void dialplan_htp_get_dp_config(evhtp_request_t *req, void *data);
void dialplan_htp_put_dp_config(evhtp_request_t *req, void *data);


bool dialplan_add_cmds(const char* agi_uuid);


#endif /* SRC_DIALPLAN_HANDLER_H_ */
