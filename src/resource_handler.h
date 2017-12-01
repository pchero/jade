/*
 * resource_handler.h
 *
 *  Created on: Feb 10, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_RESOURCE_HANDLER_H_
#define BACKEND_SRC_RESOURCE_HANDLER_H_

// peer
json_t* get_peers_all_peer(void);
json_t* get_peer_detail(const char* name);

// database
json_t* get_databases_all_key(void);
json_t* get_database_info(const char* key);

// registry
json_t* get_registries_all_account(void);
json_t* get_registry_info(const char* account);

// queue param
json_t* get_queue_params_all_name(void);
json_t* get_queue_param_info(const char* name);

// queue member
json_t* get_queue_members_all_name_queue(void);
json_t* get_queue_member_info(const char* name, const char* queue_name);

// queue entry
json_t* get_queue_entries_all_unique_id_queue_name(void);
json_t* get_queue_entry_info(const char* channel, const char* queue_name);

// channel
json_t* get_channels_all_unique_id(void);
json_t* get_channel_info(const char* unique_id);

// agent
json_t* get_agents_all_id(void);
json_t* get_agent_info(const char* id);

// system
json_t* get_systems_all_id(void);
json_t* get_system_info(const char* id);

// device_state
json_t* get_device_states_all_device(void);
json_t* get_device_state_info(const char* device);

// parking_lot
json_t* get_parking_lots_all_name(void);
json_t* get_parking_lot_info(const char* parkee_unique_id);

// parked_call
json_t* get_parked_calls_all_parkee_unique_id(void);
json_t* get_parked_call_info(const char* parkee_unique_id);

// pjsip
json_t* get_pjsip_contact_status_info(const char* uri);
json_t* get_pjsip_endpoint_info(const char* name);

#endif /* BACKEND_SRC_RESOURCE_HANDLER_H_ */
