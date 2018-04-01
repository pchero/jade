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

enum EN_SORT_TYPES {
  EN_SORT_ASC,
  EN_SORT_DESC,
};


bool resource_init_handler(void);
void resource_term_handler(void);

// ast
bool resource_exec_ast_sql(const char* sql);
bool resource_clear_ast_table(const char* table);
bool resource_insert_ast_item(const char* table, const json_t* j_data);
bool resource_insrep_ast_item(const char* table, const json_t* j_data);
bool resource_update_ast_item(const char* table, const char* key_column, const json_t* j_data);
json_t* resource_get_ast_items(const char* table, const char* item);
json_t* resource_get_ast_detail_items_by_condtion(const char* table, const char* condition);
json_t* resource_get_ast_detail_item_key_string(const char* table, const char* key, const char* val);
json_t* resource_get_ast_detail_items_key_string(const char* table, const char* key, const char* val);
bool resource_delete_ast_items_string(const char* table, const char* key, const char* val);

// jade
bool resource_exec_jade_sql(const char* sql);
bool resource_insert_jade_item(const char* table, const json_t* j_data);
bool resource_insrep_jade_item(const char* table, const json_t* j_data);
bool resource_update_jade_item(const char* table, const char* key_column, const json_t* j_data);
bool resource_delete_jade_items_string(const char* table, const char* key, const char* val);
bool resource_delete_jade_items_by_obj(const char* table, json_t* j_obj);
json_t* resource_get_jade_items(const char* table, const char* item);
json_t* resource_get_jade_detail_item_key_string(const char* table, const char* key, const char* val);
json_t* resource_get_jade_detail_item_by_obj(const char* table, json_t* j_obj);
json_t* resource_get_jade_detail_items_key_string(const char* table, const char* key, const char* val);
json_t* resource_get_jade_detail_items_by_obj(const char* table, json_t* j_obj);
json_t* resource_get_jade_detail_items_by_obj_order(const char* table, json_t* j_obj, const char* order);
json_t* resource_get_jade_detail_items_by_condtion(const char* table, const char* condition);

// etc
json_t* resource_sort_json_array_string(const json_t* j_data, enum EN_SORT_TYPES type);



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
bool update_core_agi_info_cmd_result_done(const char* agi_uuid, const char* cmd_id, const char* result_org);

bool delete_core_agi_info(const char* key);
bool add_core_agi_info_cmd(const char* agi_uuid, const char* cmd_uuid, const char* command, const char* dp_uuid);
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
json_t* get_dp_dialplans_by_dpma_uuid_order_sequence(const char* dpma_uuid);
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
bool clear_queue_entry(void);
json_t* get_queue_entry_info(const char* key);
json_t* get_queue_entry_info_by_id_name(const char* channel, const char* queue_name);
json_t* get_queue_entries_all(void);
json_t* get_queue_entries_all_by_queuename(const char* name);
json_t* get_queue_entries_all_unique_id_queue_name(void);
int delete_queue_entry_info(const char* key);
int create_queue_entry_info(const json_t* j_tmp);
// queue members
bool clear_queue_member(void);
json_t* get_queue_members_all(void);
json_t* get_queue_members_all_by_queuename(const char* name);
json_t* get_queue_members_all_name_queue(void);
json_t* get_queue_member_info(const char* id);
bool create_queue_member_info(const json_t* j_data);
bool update_queue_member_info(const json_t* j_data);
bool delete_queue_member_info(const char* key);
// queue param
bool clear_queue_param(void);
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
bool clear_park_parkinglot(void);
json_t* get_park_parkinglots_all(void);
json_t* get_park_parkinglots_all_name(void);
json_t* get_park_parkinglot_info(const char* name);
int create_park_parkinglot_info(const json_t* j_tmp);
// parkedcalls
bool clear_park_parkedcall(void);
json_t* get_park_parkedcalls_all();
json_t* get_park_parkedcalls_all_parkee_unique_id(void);
json_t* get_park_parkedcall_info(const char* parkee_unique_id);
bool create_park_parkedcall_info(const json_t* j_tmp);
bool update_park_parkedcall_info(const json_t* j_data);
bool delete_park_parkedcall_info(const char* key);


////// voicemail

// voicemail
bool create_voicemail_user_info(json_t* j_tmp);
json_t* get_voicemail_user_info(const char* key);
json_t* get_voicemail_users_all();


#endif /* BACKEND_SRC_RESOURCE_HANDLER_H_ */
