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

json_t* admin_get_subscribable_topics_all(const json_t* j_user);

// http handlers


//// ^/admin/core
void admin_htp_get_admin_core_channels(evhtp_request_t *req, void *data);
void admin_htp_get_admin_core_channels_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_core_channels_detail(evhtp_request_t *req, void *data);
void admin_htp_get_admin_core_modules(evhtp_request_t *req, void *data);
void admin_htp_get_admin_core_modules_detail(evhtp_request_t *req, void *data);
void admin_htp_post_admin_core_modules_detail(evhtp_request_t *req, void *data);
void admin_htp_put_admin_core_modules_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_core_modules_detail(evhtp_request_t *req, void *data);
void admin_htp_get_admin_core_systems(evhtp_request_t *req, void *data);
void admin_htp_get_admin_core_systems_detail(evhtp_request_t *req, void *data);

//// ^/admin/login
void admin_htp_post_admin_login(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_login(evhtp_request_t *req, void *data);


//// ^/admin/info
void admin_htp_get_admin_info(evhtp_request_t *req, void *data);
void admin_htp_put_admin_info(evhtp_request_t *req, void *data);




//// ^/admin/park
void admin_htp_get_admin_park_parkedcalls(evhtp_request_t *req, void *data);
void admin_htp_get_admin_park_parkedcalls_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_park_parkinglots_detail(evhtp_request_t *req, void *data);

void admin_htp_get_admin_park_parkinglots(evhtp_request_t *req, void *data);
void admin_htp_get_admin_park_parkinglots_detail(evhtp_request_t *req, void *data);

void admin_htp_get_admin_park_cfg_parkinglots(evhtp_request_t *req, void *data);
void admin_htp_post_admin_park_cfg_parkinglots(evhtp_request_t *req, void *data);

void admin_htp_get_admin_park_cfg_parkinglots_detail(evhtp_request_t *req, void *data);
void admin_htp_put_admin_park_cfg_parkinglots_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_park_cfg_parkinglots_detail(evhtp_request_t *req, void *data);

void admin_htp_get_admin_park_configurations(evhtp_request_t *req, void *data);

void admin_htp_get_admin_park_configurations_detail(evhtp_request_t *req, void *data);
void admin_htp_put_admin_park_configurations_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_park_configurations_detail(evhtp_request_t *req, void *data);



//// ^/admin/queue
void admin_htp_get_admin_queue_entries(evhtp_request_t *req, void *data);
void admin_htp_get_admin_queue_entries_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_queue_entries_detail(evhtp_request_t *req, void *data);

void admin_htp_get_admin_queue_queues(evhtp_request_t *req, void *data);
void admin_htp_get_admin_queue_queues_detail(evhtp_request_t *req, void *data);

void admin_htp_get_admin_queue_members(evhtp_request_t *req, void *data);
void admin_htp_post_admin_queue_members(evhtp_request_t *req, void *data);

void admin_htp_get_admin_queue_members_detail(evhtp_request_t *req, void *data);
void admin_htp_put_admin_queue_members_detail(evhtp_request_t *req, void *data);
void admin_htp_delete_admin_queue_member_detail(evhtp_request_t *req, void *data);




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
