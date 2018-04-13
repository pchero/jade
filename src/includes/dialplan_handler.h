/*
 * dialplan_handler.h
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#ifndef SRC_DIALPLAN_HANDLER_H_
#define SRC_DIALPLAN_HANDLER_H_

#include <stdbool.h>
#include <evhtp.h>

bool dialplan_init_handler(void);
bool dialplan_term_handler(void);
bool dialplan_reload_handler(void);

// dpma
json_t* dialplan_get_dpmas_all(void);
json_t* dialplan_get_dpma_info(const char* key);
bool dialplan_create_dpma_info(const json_t* j_data);
bool dialplan_update_dpma_info(const json_t* j_data);
bool dialplan_delete_dpma_info(const char* key);

// dialplan
json_t* dialplan_get_dialplans_all(void);
json_t* dialplan_get_dialplans_by_dpma_uuid_order_sequence(const char* dpma_uuid);
json_t* dialplan_get_dialplan_info(const char* key);
json_t* dialplan_get_dialplan_info_by_dpma_seq(const char* dpma_uuid, int seq);
bool dialplan_create_dialplan_info(const json_t* j_data);
bool dialplan_update_dialplan_info(const json_t* j_data);
bool dialplan_delete_dialplan_info(const char* key);

bool dialplan_add_cmds(const char* agi_uuid);

// http handlers

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




#endif /* SRC_DIALPLAN_HANDLER_H_ */
