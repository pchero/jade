/*
 * resource_handler.h
 *
 *  Created on: Feb 10, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_RESOURCE_HANDLER_H_
#define BACKEND_SRC_RESOURCE_HANDLER_H_

#include <jansson.h>

// peer
json_t* get_peers_all_peer(void);
json_t* get_peers_all(void);
json_t* get_peer_detail(const char* name);

// database
json_t* get_databases_all_key(void);
json_t* get_database_info(const char* key);

// registry
json_t* get_registries_all_account(void);
json_t* get_registries_all(void);
json_t* get_registry_info(const char* account);


// queue
json_t* get_queue_entry_info(const char* key);
json_t* get_queue_entry_info_by_id_name(const char* channel, const char* queue_name);
json_t* get_queue_entries_all_unique_id_queue_name(void);
json_t* get_queue_entries_all(void);
json_t* get_queue_members_all(void);
json_t* get_queue_members_all_name_queue(void);
json_t* get_queue_member_info(const char* name, const char* queue_name);
json_t* get_queue_param_info(const char* name);
json_t* get_queue_params_all(void);
json_t* get_queue_params_all_name(void);


// channel
json_t* get_channels_all_unique_id(void);
json_t* get_channels_all(void);
json_t* get_channel_info(const char* unique_id);
int update_channel_info(const json_t* j_tmp);
int delete_channel_info(const char* key);

// agent
json_t* get_agents_all_id(void);
json_t* get_agents_all(void);
json_t* get_agent_info(const char* id);

// system
json_t* get_systems_all_id(void);
json_t* get_systems_all(void);
json_t* get_system_info(const char* id);

// device_state
json_t* get_device_states_all_device(void);
json_t* get_device_state_info(const char* device);

// park
json_t* get_park_parkinglots_all(void);
json_t* get_park_parkinglots_all_name(void);
json_t* get_park_parkinglot_info(const char* name);
int create_park_parkinglot_info(const json_t* j_tmp);
json_t* get_park_parkedcalls_all();
json_t* get_park_parkedcalls_all_parkee_unique_id(void);
json_t* get_park_parkedcall_info(const char* parkee_unique_id);
int create_park_parkedcall_info(const json_t* j_tmp);
int delete_park_parkedcall_info(const char* key);


// pjsip
json_t* get_pjsip_contact_info(const char* uri);
json_t* get_pjsip_endpoints_all(void);
json_t* get_pjsip_endpoint_info(const char* name);
json_t* get_pjsip_aors_all(void);
json_t* get_pjsip_aor_info(const char* key);
json_t* get_pjsip_auths_all(void);
json_t* get_pjsip_auth_info(const char* key);
json_t* get_pjsip_contacts_all(void);
json_t* get_pjsip_contact_info(const char* key);


// voicemail
json_t* get_voicemail_user_info(const char* context, const char* mailbox);
json_t* get_voicemail_users_all();


#endif /* BACKEND_SRC_RESOURCE_HANDLER_H_ */
