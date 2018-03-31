/*
 * user_handler.h
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */

#ifndef SRC_MAIN_USER_HANDLER_H_
#define SRC_MAIN_USER_HANDLER_H_

#include <stdbool.h>
#include <evhtp.h>


#define DEF_USER_CONTACT_TYPE_PEER       "sip_peer"
#define DEF_USER_CONTACT_TYPE_ENDPOINT   "pjsip_endpoint"

bool user_init_handler(void);
void user_term_handler(void);
bool user_reload_handler(void);

void user_htp_post_user_login(evhtp_request_t *req, void *data);
void user_htp_delete_user_login(evhtp_request_t *req, void *data);

void user_htp_get_user_contacts(evhtp_request_t *req, void *data);
void user_htp_post_user_contacts(evhtp_request_t *req, void *data);

void user_htp_get_user_contacts_detail(evhtp_request_t *req, void *data);
void user_htp_put_user_contacts_detail(evhtp_request_t *req, void *data);
void user_htp_delete_user_contacts_detail(evhtp_request_t *req, void *data);

void user_htp_post_user_users(evhtp_request_t *req, void *data);
void user_htp_get_user_users(evhtp_request_t *req, void *data);

void user_htp_get_user_permissions(evhtp_request_t *req, void *data);
void user_htp_post_user_permissions(evhtp_request_t *req, void *data);
void user_htp_delete_user_permissions_detail(evhtp_request_t *req, void *data);

void user_htp_get_user_users_detail(evhtp_request_t *req, void *data);
void user_htp_put_user_users_detail(evhtp_request_t *req, void *data);
void user_htp_delete_user_users_detail(evhtp_request_t *req, void *data);


json_t* user_get_userinfo_info(const char* key);
json_t* user_get_userinfo_info_by_username(const char* key);
json_t* user_get_userinfo_info_by_username_password(const char* username, const char* pass);
json_t* user_get_userinfo_by_authtoken(const char* authtoken);
bool user_update_userinfo_info(const json_t* j_data);
bool user_delete_userinfo_info(const char* key);
json_t* user_get_userinfos_all(void);

json_t* user_get_authtokens_all(void);
json_t* user_get_authtoken_info(const char* key);
bool user_create_authtoken_info(const json_t* j_data);
bool user_update_authtoken_info(const json_t* j_data);
bool user_update_authtoken_tm_update(const char* uuid);
bool user_delete_authtoken_info(const char* key);

bool user_create_permission_info(const json_t* j_data);
json_t* user_get_permissions_all(void);
json_t* user_get_permission_info_by_useruuid_perm(const char* useruuid, const char* perm);
bool user_delete_permission_info(const char* key);

json_t* user_get_contacts_all(void);
json_t* user_get_contacts_by_user_uuid(const char* user_uuid);
json_t* user_get_contact_info(const char* key);
bool user_create_contact_info(const json_t* j_data);
bool user_update_contact_info(const json_t* j_data);
bool user_delete_contact_info(const char* key);


bool user_is_user_exist(const char* user_uuid);

#endif /* SRC_MAIN_USER_HANDLER_H_ */
