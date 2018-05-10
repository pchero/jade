/*
 * manager_handler.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_MANAGER_HANDLER_H_
#define SRC_INCLUDES_MANAGER_HANDLER_H_

#include <stdbool.h>
#include <evhtp.h>

bool manager_init_handler(void);
bool manager_term_handler(void);
bool manager_reload_handler(void);

json_t* manager_get_subscribable_topics_all(const json_t* j_user);

// http handlers
void manager_htp_post_manager_login(evhtp_request_t *req, void *data);

void manager_htp_get_manager_info(evhtp_request_t *req, void *data);
void manager_htp_put_manager_info(evhtp_request_t *req, void *data);

void manager_htp_get_manager_users(evhtp_request_t *req, void *data);
void manager_htp_post_manager_users(evhtp_request_t *req, void *data);

void manager_htp_get_manager_users_detail(evhtp_request_t *req, void *data);
void manager_htp_put_manager_users_detail(evhtp_request_t *req, void *data);
void manager_htp_delete_manager_users_detail(evhtp_request_t *req, void *data);

void manager_htp_post_manager_trunks(evhtp_request_t *req, void *data);
void manager_htp_get_manager_trunks(evhtp_request_t *req, void *data);

void manager_htp_get_manager_trunks_detail(evhtp_request_t *req, void *data);
void manager_htp_put_manager_trunks_detail(evhtp_request_t *req, void *data);
void manager_htp_delete_manager_trunks_detail(evhtp_request_t *req, void *data);

#endif /* SRC_INCLUDES_MANAGER_HANDLER_H_ */
