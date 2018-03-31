/*
 * queue_handler.h
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */

#ifndef SRC_QUEUE_HANDLER_H_
#define SRC_QUEUE_HANDLER_H_

#include <evhtp.h>

bool queue_init_handler(void);
bool queue_reload_handler(void);
bool queue_term_handler(void);

void queue_htp_get_queue_entries(evhtp_request_t *req, void *data);
void queue_htp_get_queue_entries_detail(evhtp_request_t *req, void *data);
void queue_htp_get_queue_members(evhtp_request_t *req, void *data);
void queue_htp_get_queue_members_detail(evhtp_request_t *req, void *data);

void queue_htp_get_queue_queues(evhtp_request_t *req, void *data);
void queue_htp_post_queue_queues(evhtp_request_t *req, void *data);

void queue_htp_get_queue_queues_detail(evhtp_request_t *req, void *data);
void queue_htp_put_queue_queues_detail(evhtp_request_t *req, void *data);
void queue_htp_delete_queue_queues_detail(evhtp_request_t *req, void *data);

void queue_htp_get_queue_config(evhtp_request_t *req, void *data);
void queue_htp_put_queue_config(evhtp_request_t *req, void *data);

void queue_htp_get_queue_configs(evhtp_request_t *req, void *data);

void queue_htp_get_queue_configs_detail(evhtp_request_t *req, void *data);
void queue_htp_delete_queue_configs_detail(evhtp_request_t *req, void *data);

void queue_htp_get_queue_settings(evhtp_request_t *req, void *data);
void queue_htp_post_queue_settings(evhtp_request_t *req, void *data);

void queue_htp_get_queue_settings_detail(evhtp_request_t *req, void *data);
void queue_htp_put_queue_settings_detail(evhtp_request_t *req, void *data);
void queue_htp_delete_queue_settings_detail(evhtp_request_t *req, void *data);

void queue_htp_get_queue_statuses(evhtp_request_t *req, void *data);
void queue_htp_get_queue_statuses_detail(evhtp_request_t *req, void *data);


#endif /* SRC_QUEUE_HANDLER_H_ */
