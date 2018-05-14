/*
 * admin_handler.h
 *
 *  Created on: Apr 19, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_ADMIN_HANDLER_H_
#define SRC_INCLUDES_ADMIN_HANDLER_H_

#include <stdbool.h>

bool admin_init_handler(void);
bool admin_term_handler(void);
bool admin_reload_handler(void);


// http handlers

//// ^/admin/login
void admin_htp_post_admin_login(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_login(evhtp_request_t *req, void *data);


//// ^/admin/queue
void admin_htp_get_admin_queue_queues(evhtp_request_t *req, void *data);
void admin_htp_get_admin_queue_queues_detail(evhtp_request_t *req, void *data);




//// ^/admin/user

// users
void admin_htp_get_admin_user_users(evhtp_request_t *req, void *data);
void admin_htp_post_admin_user_users(evhtp_request_t *req, void *data);

void admin_htp_get_admin_user_users_detail(evhtp_request_t *req, void *data);
void admin_htp_put_admin_user_users_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_user_users_detail(evhtp_request_t *req, void *data);

// contacts
void admin_htp_get_admin_user_contacts(evhtp_request_t *req, void *data);
void admin_htp_post_admin_user_contacts(evhtp_request_t *req, void *data);

void admin_htp_get_admin_user_contacts_detail(evhtp_request_t *req, void *data);
void admin_htp_put_admin_user_contacts_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_user_contacts_detail(evhtp_request_t *req, void *data);

// permissions
void admin_htp_get_admin_user_permissions(evhtp_request_t *req, void *data);
void admin_htp_post_admin_user_permissions(evhtp_request_t *req, void *data);

void admin_htp_get_admin_user_permissions_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_user_permissions_detail(evhtp_request_t *req, void *data);




#endif /* SRC_INCLUDES_ADMIN_HANDLER_H_ */
