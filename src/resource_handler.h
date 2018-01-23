/*
 * resource_handler.h
 *
 *  Created on: Feb 10, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_RESOURCE_HANDLER_H_
#define BACKEND_SRC_RESOURCE_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

bool init_resource_handler(void);
void term_resource_handler(void);

//////// agent
// agent
bool create_agent_agent_info(const json_t* j_data);
bool update_agent_agent_info(const json_t* j_data);
bool delete_agent_agent_info(const char* key);
json_t* get_agent_agents_all_id(void);
json_t* get_agent_agents_all(void);
json_t* get_agent_agent_info(const char* id);


////// core
// core_agi
json_t* get_core_agis_all(void);
json_t* get_core_agi_info(const char* unique_id);
bool create_core_agi_info(const json_t* j_data);
bool update_core_agi_info(const json_t* j_data);
bool update_core_agi_info_cmd_result(const char* key, const char* cmd_id, const char* result);
bool delete_core_agi_info(const char* key);
// core_channel
json_t* get_core_channels_all_unique_id(void);
json_t* get_core_channels_all(void);
json_t* get_core_channel_info(const char* unique_id);
bool create_core_channel_info(const json_t* j_data);
int update_core_channel_info(const json_t* j_tmp);
int delete_core_channel_info(const char* key);
// core_system
json_t* get_core_systems_all_id(void);
json_t* get_core_systems_all(void);
json_t* get_core_system_info(const char* id);
// core_module
int create_core_module(json_t* j_tmp);
json_t* get_core_modules_all(void);
json_t* get_core_module_info(const char* key);
bool update_core_module_info(const json_t* j_data);


///////// dialplan
// dpma
json_t* get_dp_dpmas_all(void);
json_t* get_dp_dpma_info(const char* key);
bool create_dp_dpma_info(const json_t* j_data);
bool update_dp_dpma_info(const json_t* j_data);
bool delete_dp_dpma_info(const char* key);
// dialplan
json_t* get_dp_dialplans_all(void);
json_t* get_dp_dialplan_info(const char* key);
json_t* get_dp_dialplan_info_by_dpma_seq(const char* dpma_uuid, int seq);
bool create_dp_dialplan_info(const json_t* j_data);
bool update_dp_dialplan_info(const json_t* j_data);
bool delete_dp_dialplan_info(const char* key);


// database
json_t* get_databases_all_key(void);
json_t* get_database_info(const char* key);



///////// queue
// queue entry
json_t* get_queue_entry_info(const char* key);
json_t* get_queue_entry_info_by_id_name(const char* channel, const char* queue_name);
json_t* get_queue_entries_all(void);
json_t* get_queue_entries_all_by_queuename(const char* name);
json_t* get_queue_entries_all_unique_id_queue_name(void);
int delete_queue_entry_info(const char* key);
int create_queue_entry_info(const json_t* j_tmp);
// queue members
json_t* get_queue_members_all(void);
json_t* get_queue_members_all_by_queuename(const char* name);
json_t* get_queue_members_all_name_queue(void);
json_t* get_queue_member_info(const char* id);
int create_queue_member_info(const json_t* j_data);
int delete_queue_member_info(const char* key);
// queue param
json_t* get_queue_param_info(const char* name);
json_t* get_queue_params_all(void);
json_t* get_queue_params_all_name(void);
int create_queue_param_info(const json_t* j_tmp);
int delete_queue_param_info(const char* key);
// queue status
json_t* get_queue_status_info(const char* name);
json_t* get_queue_statuses_all(void);


// device_state
json_t* get_device_states_all_device(void);
json_t* get_device_state_info(const char* device);


/////// park
// parkinglot
json_t* get_park_parkinglots_all(void);
json_t* get_park_parkinglots_all_name(void);
json_t* get_park_parkinglot_info(const char* name);
int create_park_parkinglot_info(const json_t* j_tmp);
// parkedcalls
json_t* get_park_parkedcalls_all();
json_t* get_park_parkedcalls_all_parkee_unique_id(void);
json_t* get_park_parkedcall_info(const char* parkee_unique_id);
bool create_park_parkedcall_info(const json_t* j_tmp);
bool update_park_parkedcall_info(const json_t* j_data);
bool delete_park_parkedcall_info(const char* key);


//////// pjsip
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

////// sip
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


// voicemail
bool create_voicemail_user_info(json_t* j_tmp);
json_t* get_voicemail_user_info(const char* key);
json_t* get_voicemail_users_all();


#endif /* BACKEND_SRC_RESOURCE_HANDLER_H_ */
