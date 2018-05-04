/*
 * configfile_handler.h
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#ifndef SRC_CONF_HANDLER_H_
#define SRC_CONF_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

bool conf_init_handler(void);

json_t* conf_get_ast_current_config_info_object(const char* filename);
json_t* conf_get_ast_current_config_info_array(const char* filename);
char* conf_get_ast_current_config_info_text(const char* filename);

bool conf_update_ast_current_config_info(const char* filename, json_t* j_conf);
bool conf_update_ast_current_config_info_text(const char* filename, const char* data);

bool conf_create_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool conf_update_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool conf_delete_ast_current_config_content(const char* filename, const char* section, const char* key);

bool conf_create_ast_setting(const char* filename, const char* name, const json_t* j_data);
json_t* conf_get_ast_settings_all(const char* filename);
json_t* conf_get_ast_setting(const char* filename, const char* name);
bool conf_update_ast_setting(const char* filename, const char* name, const json_t* j_data);
bool conf_remove_ast_setting(const char* filename, const char* name);

json_t* conf_get_ast_backup_config_info_json(const char* filename);
char* conf_get_ast_backup_config_info_text(const char* filename);
char* conf_get_ast_backup_config_info_text_valid(const char* filename, const char* valid);

json_t* conf_get_ast_backup_configs_info_all(const char* filename);
bool conf_remove_ast_backup_config_info_valid(const char* filename, const char* valid);


#endif /* SRC_CONF_HANDLER_H_ */
