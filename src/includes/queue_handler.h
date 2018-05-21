/*
 * queue_handler.h
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */

#ifndef SRC_QUEUE_HANDLER_H_
#define SRC_QUEUE_HANDLER_H_

#include <evhtp.h>
#include <stdbool.h>
#include <jansson.h>

bool queue_init_handler(void);
bool queue_reload_handler(void);
bool queue_term_handler(void);

// param
json_t* queue_get_queue_param_info(const char* name);
json_t* queue_get_queue_params_all(void);
bool queue_create_param_info(const json_t* j_tmp);

// member
json_t* queue_get_members_all(void);
json_t* queue_get_members_all_by_queuename(const char* name);
json_t* queue_get_member_info(const char* id);
bool queue_create_member_info(const json_t* j_data);
bool queue_update_member_info(const json_t* j_data);
bool queue_delete_member_info(const char* key);

// entry
json_t* queue_get_entries_all_by_queuename(const char* name);
json_t* queue_get_entries_all(void);
json_t* queue_get_entry_info(const char* key);
bool queue_create_entry_info(const json_t* j_tmp);
bool queue_delete_entry_info(const char* key);

// cfg
json_t* queue_cfg_get_queues_all(void);
json_t* queue_cfg_get_queue_info(const char* name);
bool queue_cfg_create_queue_info(const json_t* j_data);
bool queue_cfg_update_queue_info(const json_t* j_data);
bool queue_cfg_delete_queue_info(const char* name);

// configuration
json_t* queue_get_configurations_all(void);
json_t* queue_get_configuration_info(const char* name);
bool queue_update_configuration_info(const json_t* j_data);
bool queue_delete_configuration_info(const char* name);

// action
bool queue_action_update_member_paused_penalty(const json_t* j_data);
bool queue_action_delete_member_from_queue(const char* id);
bool queue_action_add_member_to_queue(const json_t* j_data);

// callback
bool queue_register_callback_db_entry(bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*));
bool queue_register_callback_db_member(bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*));

#endif /* SRC_QUEUE_HANDLER_H_ */
