/*
 * pjsip_handler.h
 *
 *  Created on: Dec 13, 2017
 *      Author: pchero
 */

#ifndef SRC_PJSIP_HANDLER_H_
#define SRC_PJSIP_HANDLER_H_

#include <evhtp.h>


bool init_pjsip_handler(void);
bool reload_pjsip_handler(void);
bool term_pjsip_handler(void);

void htp_get_pjsip_endpoints(evhtp_request_t *req, void *data);
void htp_get_pjsip_endpoints_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_aors(evhtp_request_t *req, void *data);
void htp_get_pjsip_aors_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_auths(evhtp_request_t *req, void *data);
void htp_get_pjsip_auths_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_contacts(evhtp_request_t *req, void *data);
void htp_get_pjsip_contacts_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_config(evhtp_request_t *req, void *data);
void htp_put_pjsip_config(evhtp_request_t *req, void *data);
void htp_get_pjsip_configs(evhtp_request_t *req, void *data);
void htp_get_pjsip_configs_detail(evhtp_request_t *req, void *data);
void htp_delete_pjsip_configs_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_settings_detail(evhtp_request_t *req, void *data);
void htp_put_pjsip_settings_detail(evhtp_request_t *req, void *data);
void htp_delete_pjsip_settings_detail(evhtp_request_t *req, void *data);
void htp_get_pjsip_settings(evhtp_request_t *req, void *data);
void htp_post_pjsip_settings(evhtp_request_t *req, void *data);


bool clear_pjsip(void);

// endpoint
bool create_pjsip_endpoint_info(const json_t* j_data);
bool update_pjsip_endpoint_info(const json_t* j_data);
bool delete_pjsip_endpoint_info(const char* key);
json_t* get_pjsip_endpoints_all(void);
json_t* get_pjsip_endpoint_info(const char* name);

// auth
bool create_pjsip_auth_info(const json_t* j_data);
bool update_pjsip_auth_info(const json_t* j_data);
bool delete_pjsip_auth_info(const char* key);
json_t* get_pjsip_auths_all(void);
json_t* get_pjsip_auth_info(const char* key);

// aor
bool create_pjsip_aor_info(const json_t* j_data);
bool update_pjsip_aor_info(const json_t* j_data);
bool delete_pjsip_aor_info(const char* key);
json_t* get_pjsip_aors_all(void);
json_t* get_pjsip_aor_info(const char* key);

// contact
bool create_pjsip_contact_info(const json_t* j_data);
bool update_pjsip_contact_info(const json_t* j_data);
bool delete_pjsip_contact_info(const char* key);
json_t* get_pjsip_contacts_all(void);
json_t* get_pjsip_contact_info(const char* key);


#endif /* SRC_PJSIP_HANDLER_H_ */
