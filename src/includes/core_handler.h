/*
 * core_handler.h
 *
 *  Created on: Dec 12, 2017
 *      Author: pchero
 */

#ifndef SRC_CORE_HANDLER_H_
#define SRC_CORE_HANDLER_H_

#include <evhtp.h>
#include <stdbool.h>
#include <jansson.h>

bool core_init_handler(void);
bool core_term_handler(void);
bool core_reload_handler(void);

// channel
json_t* core_get_channels_all(void);
json_t* core_get_channels_by_devicename(const char* device_name);
json_t* core_get_channel_info(const char* unique_id);
bool core_create_channel_info(const json_t* j_data);
int core_update_channel_info(const json_t* j_tmp);
int core_delete_channel_info(const char* key);

// module
json_t* core_get_modules_all(void);
json_t* core_get_module_info(const char* key);
bool core_create_module(json_t* j_tmp);
bool core_update_module_info(const json_t* j_data);

// system
json_t* core_get_systems_all(void);
json_t* core_get_system_info(const char* id);
bool core_create_system_info(const json_t* j_data);
bool core_update_system_info(const json_t* j_data);


// etc
bool core_module_load(const char* name);
bool core_module_reload(const char* name);
bool core_module_unload(const char* name);


#endif /* SRC_CORE_HANDLER_H_ */
