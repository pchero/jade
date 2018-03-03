/*
 * sip_handler.h
 *
 *  Created on: Dec 12, 2017
 *      Author: pchero
 */

#ifndef SRC_SIP_HANDLER_H_
#define SRC_SIP_HANDLER_H_

#include <evhtp.h>

bool init_sip_handler(void);
bool reload_sip_handler(void);
bool term_sip_handler(void);

void htp_get_sip_config(evhtp_request_t *req, void *data);
void htp_put_sip_config(evhtp_request_t *req, void *data);

void htp_get_sip_configs(evhtp_request_t *req, void *data);

void htp_get_sip_configs_detail(evhtp_request_t *req, void *data);
void htp_delete_sip_configs_detail(evhtp_request_t *req, void *data);

void htp_get_sip_peers_detail(evhtp_request_t *req, void *data);
void htp_get_sip_peers(evhtp_request_t *req, void *data);

void htp_get_sip_registries(evhtp_request_t *req, void *data);
void htp_get_sip_registries_detail(evhtp_request_t *req, void *data);

void htp_get_sip_settings(evhtp_request_t *req, void *data);
void htp_post_sip_settings(evhtp_request_t *req, void *data);

void htp_get_sip_settings_detail(evhtp_request_t *req, void *data);
void htp_put_sip_settings_detail(evhtp_request_t *req, void *data);


// sip_peer
bool create_sip_peer_info(const json_t* j_data);
bool update_sip_peer_info(const json_t* j_data);
bool delete_sip_peer_info(const char* key);
json_t* get_sip_peers_all_peer(void);
json_t* get_sip_peers_all(void);
json_t* get_sip_peer_info(const char* name);

// sip_registry
bool create_sip_registry_info(const json_t* j_data);
bool update_sip_registry_info(const json_t* j_data);
bool delete_sip_registry_info(const char* key);
json_t* get_sip_registries_all_account(void);
json_t* get_sip_registries_all(void);
json_t* get_sip_registry_info(const char* account);

// sip_peeraccount
bool create_sip_peeraccount_info(const json_t* j_data);
json_t* get_sip_peeraccount_info(const char* peer);


#endif /* SRC_SIP_HANDLER_H_ */
