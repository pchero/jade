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

enum EN_RESOURCE_UPDATE_TYPES {
  EN_RESOURCE_CREATE   = 1,
  EN_RESOURCE_UPDATE   = 2,
  EN_RESOURCE_DELETE   = 3,
};



bool resource_init_handler(void);
void resource_term_handler(void);

// memory
bool resource_exec_mem_sql(const char* sql);
bool resource_clear_mem_table(const char* table);
bool resource_insert_mem_item(const char* table, const json_t* j_data);
bool resource_insrep_mem_item(const char* table, const json_t* j_data);
bool resource_update_mem_item(const char* table, const char* key_column, const json_t* j_data);
json_t* resource_get_mem_items(const char* table, const char* item);
json_t* resource_get_mem_detail_item_key_string(const char* table, const char* key, const char* val);
json_t* resource_get_mem_detail_items_by_condtion(const char* table, const char* condition);
json_t* resource_get_mem_detail_items_key_string(const char* table, const char* key, const char* val);
bool resource_delete_mem_items_string(const char* table, const char* key, const char* val);

// file
bool resource_exec_file_sql(const char* sql);
bool resource_insert_file_item(const char* table, const json_t* j_data);
bool resource_insrep_file_item(const char* table, const json_t* j_data);
bool resource_update_file_item(const char* table, const char* key_column, const json_t* j_data);
bool resource_delete_file_items_string(const char* table, const char* key, const char* val);
bool resource_delete_file_items_by_obj(const char* table, json_t* j_obj);
json_t* resource_get_file_items(const char* table, const char* item);
json_t* resource_get_file_detail_item_key_string(const char* table, const char* key, const char* val);
json_t* resource_get_file_detail_item_by_obj(const char* table, json_t* j_obj);
json_t* resource_get_file_detail_items_key_string(const char* table, const char* key, const char* val);
json_t* resource_get_file_detail_items_by_obj(const char* table, json_t* j_obj);
json_t* resource_get_file_detail_items_by_obj_order(const char* table, json_t* j_obj, const char* order);
json_t* resource_get_file_detail_items_by_condtion(const char* table, const char* condition);

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
// core_system
json_t* get_core_systems_all_id(void);
json_t* get_core_systems_all(void);
json_t* get_core_system_info(const char* id);
// core_module
int create_core_module(json_t* j_tmp);
json_t* get_core_modules_all(void);
json_t* get_core_module_info(const char* key);
bool update_core_module_info(const json_t* j_data);




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
