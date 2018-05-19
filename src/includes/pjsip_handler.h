/*
 * pjsip_handler.h
 *
 *  Created on: Dec 13, 2017
 *      Author: pchero
 */

#ifndef SRC_PJSIP_HANDLER_H_
#define SRC_PJSIP_HANDLER_H_

#include <evhtp.h>


bool pjsip_init_handler(void);
bool pjsip_reload_handler(void);
bool pjsip_term_handler(void);

bool pjsip_register_callback_module(bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*));
bool pjsip_register_callback_db_registration_outbound(bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*));
bool pjsip_register_callback_cfg_endpoint(bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*));

// endpoint
bool pjsip_create_endpoint_info(const json_t* j_data);
bool pjsip_update_endpoint_info(const json_t* j_data);
bool pjsip_delete_endpoint_info(const char* key);
json_t* pjsip_get_endpoints_all(void);
json_t* pjsip_get_endpoint_info(const char* name);

// auth
bool pjsip_create_auth_info(const json_t* j_data);
bool pjsip_update_auth_info(const json_t* j_data);
bool pjsip_delete_auth_info(const char* key);
json_t* pjsip_get_auths_all(void);
json_t* pjsip_get_auth_info(const char* key);

// aor
bool pjsip_create_aor_info(const json_t* j_data);
bool pjsip_update_aor_info(const json_t* j_data);
bool pjsip_delete_aor_info(const char* key);
json_t* pjsip_get_aors_all(void);
json_t* pjsip_get_aor_info(const char* key);

// contact
bool pjsip_create_contact_info(const json_t* j_data);
bool pjsip_update_contact_info(const json_t* j_data);
bool pjsip_delete_contact_info(const char* key);
json_t* pjsip_get_contacts_all(void);
json_t* pjsip_get_contact_info(const char* key);

// registration_inbound
bool pjsip_create_registration_inbound_info(const json_t* j_data);
bool pjsip_update_registration_inbound_info(const json_t* j_data);
bool pjsip_delete_registration_inbound_info(const char* key);

// registration outbound
json_t* pjsip_get_registration_outbounds_all(void);
json_t* pjsip_get_registration_outbound_info(const char* key);
bool pjsip_create_registration_outbound_info(const json_t* j_data);
bool pjsip_update_registration_outbound_info(const json_t* j_data);
bool pjsip_delete_registration_outbound_info(const char* key);

// account(aor, auth, endpoint)
bool pjsip_create_account_with_default_setting(const char* target, const char* context);
bool pjsip_delete_account(const char* target_name);

bool pjsip_is_exist_endpoint(const char* target);
bool pjsip_reload_config(void);

json_t* pjsip_cfg_get_aor_info_data(const char* name);
bool pjsip_cfg_create_aor_info(const char* name, const char* contact);
bool pjsip_cfg_update_aor_info_data(const char* name, const json_t* j_data);
bool pjsip_cfg_delete_aor_info(const char* name);

json_t* pjsip_cfg_get_auth_info_data(const char* name);
bool pjsip_cfg_create_auth_info(const char* name, const char* username, const char* password);
bool pjsip_cfg_update_auth_info(const char* name, const json_t* j_data);
bool pjsip_cfg_delete_auth_info(const char* name);

json_t* pjsip_cfg_get_contact_info_data(const char* name);
bool pjsip_cfg_update_contact_info(const json_t* j_data);
bool pjsip_cfg_delete_contact_info(const char* name);

json_t* pjsip_cfg_get_endpoint_info_data(const char* name);
bool pjsip_cfg_create_endpoint_with_default_info(const char* name, const char* context);
bool pjsip_cfg_update_endpoint_info_data(const char* name, const json_t* j_data);
json_t* pjsip_cfg_get_endpoint_info(const char* name);
bool pjsip_cfg_delete_endpoint_info(const char* name);

json_t* pjsip_cfg_get_identify_info_data(const char* name);
bool pjsip_cfg_create_identify_info(const char* name, const char* match);
bool pjsip_cfg_update_identify_info_data(const char* name, const json_t* j_data);
bool pjsip_cfg_delete_identify_info(const char* name);

json_t* pjsip_cfg_get_registrations_all(void);
json_t* pjsip_cfg_get_registration_info_data(const char* name);
bool pjsip_cfg_create_registration_with_default_info(const char* name, const char* server_uri, const char* client_uri);
json_t* pjsip_cfg_get_registration_info(const char* name);
bool pjsip_cfg_create_registration_info(const json_t* j_data);
bool pjsip_cfg_update_registration_info(const json_t* j_data);
bool pjsip_cfg_delete_registration_info(const char* name);

json_t* pjsip_cfg_get_transport_info(const char* name);
bool pjsip_cfg_create_transport_info(const json_t* j_data);
bool pjsip_cfg_update_transport_info(const json_t* j_data);
bool pjsip_cfg_delete_transport_info(const char* name);

// configurations
json_t* pjsip_get_configurations_all(void);
json_t* pjsip_get_configuration_info(const char* name);
bool pjsip_update_configuration_info(const json_t* j_data);
bool pjsip_delete_configuration_info(const char* name);

#endif /* SRC_PJSIP_HANDLER_H_ */
