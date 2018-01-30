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


#endif /* SRC_SIP_HANDLER_H_ */
