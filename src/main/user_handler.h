/*
 * user_handler.h
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */

#ifndef SRC_MAIN_USER_HANDLER_H_
#define SRC_MAIN_USER_HANDLER_H_

#include <stdbool.h>


#define DEF_USER_CONTACT_TYPE_PEER       "sip_peer"
#define DEF_USER_CONTACT_TYPE_ENDPOINT   "pjsip_endpoint"

bool init_user_handler(void);
void term_user_handler(void);
bool reload_user_handler(void);


void htp_post_user_login(evhtp_request_t *req, void *data);
void htp_delete_user_login(evhtp_request_t *req, void *data);

void htp_get_user_contacts(evhtp_request_t *req, void *data);
void htp_post_user_contacts(evhtp_request_t *req, void *data);

void htp_get_user_contacts_detail(evhtp_request_t *req, void *data);
void htp_put_user_contacts_detail(evhtp_request_t *req, void *data);
void htp_delete_user_contacts_detail(evhtp_request_t *req, void *data);

void htp_post_user_users(evhtp_request_t *req, void *data);
void htp_get_user_users(evhtp_request_t *req, void *data);

void htp_get_user_permissions(evhtp_request_t *req, void *data);
void htp_post_user_permissions(evhtp_request_t *req, void *data);
void htp_delete_user_permissions_detail(evhtp_request_t *req, void *data);

void htp_get_user_users_detail(evhtp_request_t *req, void *data);
void htp_put_user_users_detail(evhtp_request_t *req, void *data);
void htp_delete_user_users_detail(evhtp_request_t *req, void *data);


json_t* get_user_userinfo_info(const char* key);
json_t* get_user_userinfo_info_by_username(const char* key);
json_t* get_user_userinfo_info_by_username_pass(const char* username, const char* pass);
json_t* get_user_userinfo_by_authtoken(const char* authtoken);
bool update_user_userinfo_info(const json_t* j_data);
bool delete_user_userinfo_info(const char* key);
json_t* get_user_userinfos_all(void);

json_t* get_user_authtokens_all(void);
json_t* get_user_authtoken_info(const char* key);
bool create_user_authtoken_info(const json_t* j_data);
bool update_user_authtoken_info(const json_t* j_data);
bool update_user_authtoken_tm_update(const char* uuid);
bool delete_user_authtoken_info(const char* key);

bool create_user_permission_info(const json_t* j_data);
json_t* get_user_permissions_all(void);
json_t* get_user_permission_info_by_useruuid_perm(const char* useruuid, const char* perm);
bool delete_user_permission_info(const char* key);

json_t* get_user_contacts_all(void);
json_t* get_user_contacts_by_user_uuid(const char* user_uuid);
json_t* get_user_contact_info(const char* key);
bool create_user_contact_info(const json_t* j_data);
bool update_user_contact_info(const json_t* j_data);
bool delete_user_contact_info(const char* key);

#endif /* SRC_MAIN_USER_HANDLER_H_ */
