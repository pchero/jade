/*
 * resource_handler.h
 *
 *  Created on: Feb 10, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_RESOURCE_HANDLER_H_
#define BACKEND_SRC_RESOURCE_HANDLER_H_

json_t* get_peers_all_name(void);

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


#endif /* BACKEND_SRC_RESOURCE_HANDLER_H_ */
