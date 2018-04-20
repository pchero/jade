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

// userinfo
json_t* user_get_userinfo_info(const char* key);
json_t* user_get_userinfo_info_by_username(const char* key);
json_t* user_get_userinfo_info_by_username_password(const char* username, const char* pass);
json_t* user_get_userinfo_by_authtoken(const char* authtoken);
bool user_create_userinfo(const char* uuid, json_t* j_data);
bool user_update_userinfo_info(const char* uuid_user, const json_t* j_data);
bool user_delete_userinfo_info(const char* key);
bool user_delete_info_by_useruuid(const char* uuid_user);
json_t* user_get_userinfos_all(void);

// authtoken
json_t* user_get_authtokens_all(void);
json_t* user_get_authtokens_user_type(const char* uuid_user, const char* type);
json_t* user_get_authtoken_info(const char* key);
bool user_update_authtoken_tm_update(const char* uuid);
bool user_delete_authtoken_info(const char* key);

// permission
json_t* user_get_permissions_all(void);
json_t* user_get_permissions_by_useruuid(const char* uuid_user);
bool user_create_permission_info(const json_t* j_data);
json_t* user_get_permission_info_by_useruuid_perm(const char* useruuid, const char* perm);
bool user_delete_permissions_by_useruuid(const char* uuid_user);

// contact
json_t* user_get_contacts_all(void);
json_t* user_get_contacts_by_user_uuid(const char* user_uuid);
json_t* user_get_contact_info(const char* key);
bool user_create_contact_info(const json_t* j_data);

// buddy
json_t* user_get_buddies_info_by_owneruuid(const char* uuid_user);
json_t* user_get_buddy_info(const char* uuid);
bool user_create_buddy_info(const char* uuid, const json_t* j_data);
bool user_update_buddy_info(const json_t* j_data);
bool user_delete_buddy_info(const char* uuid);

bool user_is_user_exist(const char* user_uuid);
bool user_is_user_owned_buddy(const char* uuid_owner, const char* uuid_buddy);

// authtoken
char* user_create_authtoken(const char* username, const char* password, const char* type);


// http handlers
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

#endif /* SRC_MAIN_USER_HANDLER_H_ */
