/*
 * sip_handler.h
 *
 *  Created on: Dec 12, 2017
 *      Author: pchero
 */

#ifndef SRC_SIP_HANDLER_H_
#define SRC_SIP_HANDLER_H_

#include <evhtp.h>

bool sip_init_handler(void);
bool sip_reload_handler(void);
bool sip_term_handler(void);

void sip_htp_get_sip_config(evhtp_request_t *req, void *data);
void sip_htp_put_sip_config(evhtp_request_t *req, void *data);

void sip_htp_get_sip_configs(evhtp_request_t *req, void *data);

void sip_htp_get_sip_configs_detail(evhtp_request_t *req, void *data);
void sip_htp_delete_sip_configs_detail(evhtp_request_t *req, void *data);

void sip_htp_get_sip_peers_detail(evhtp_request_t *req, void *data);
void sip_htp_get_sip_peers(evhtp_request_t *req, void *data);

void sip_htp_get_sip_registries(evhtp_request_t *req, void *data);
void sip_htp_get_sip_registries_detail(evhtp_request_t *req, void *data);

void sip_htp_get_sip_settings(evhtp_request_t *req, void *data);
void sip_htp_post_sip_settings(evhtp_request_t *req, void *data);

void sip_htp_get_sip_settings_detail(evhtp_request_t *req, void *data);
void sip_htp_put_sip_settings_detail(evhtp_request_t *req, void *data);


// sip_peer
bool sip_create_peer_info(const json_t* j_data);
bool sip_update_peer_info(const json_t* j_data);
bool sip_delete_peer_info(const char* key);
json_t* sip_get_peers_all_peer(void);
json_t* sip_get_peers_all(void);
json_t* sip_get_peer_info(const char* name);

// sip_registry
bool sip_create_registry_info(const json_t* j_data);
bool sip_update_registry_info(const json_t* j_data);
bool sip_delete_registry_info(const char* key);
json_t* sip_get_registries_all_account(void);
json_t* sip_get_registries_all(void);
json_t* sip_get_registry_info(const char* account);

// sip_peeraccount
bool sip_create_peeraccount_info(const json_t* j_data);
json_t* sip_get_peeraccount_info(const char* peer);


#endif /* SRC_SIP_HANDLER_H_ */
