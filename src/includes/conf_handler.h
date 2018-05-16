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

// object
json_t* conf_get_ast_current_config_info(const char* filename);
bool conf_update_ast_current_config_info(const char* filename, json_t* j_conf);

bool conf_create_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool conf_update_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool conf_delete_ast_current_config_content(const char* filename, const char* section, const char* key);

json_t* conf_get_ast_sections_all(const char* filename);
json_t* conf_get_ast_section(const char* filename, const char* name);
bool conf_create_ast_section(const char* filename, const char* name, const json_t* j_data);
bool conf_update_ast_section(const char* filename, const char* name, const json_t* j_data);
bool conf_delete_ast_section(const char* filename, const char* name);

json_t* conf_get_ast_backup_configs_text_all(const char* filename);
json_t* conf_get_ast_backup_config_info(const char* filename);
bool conf_remove_ast_backup_config_info_valid(const char* filename, const char* valid);

// array
json_t* conf_get_ast_current_config_info_array(const char* filename);
bool conf_update_ast_current_config_info_array(const char* filename, json_t* j_conf);

bool conf_create_ast_section_array(const char* filename, const char* name, const json_t* j_data);
bool conf_update_ast_section_array(const char* filename, const char* name, const json_t* j_data);
bool conf_delete_ast_section_array(const char* filename, const char* name);

// text
json_t* conf_get_ast_current_config_info_text(const char* filename);
bool conf_update_ast_current_config_info_text(const char* filename, const char* data);

json_t* conf_get_ast_backup_config_info_text(const char* filename);
json_t* conf_get_ast_backup_config_info_text_valid(const char* filename, const char* valid);

// etc
bool conf_add_external_config_file(const char* filename, const char* external_filename);
bool conf_is_exist_config_file(const char* filename);

#endif /* SRC_CONF_HANDLER_H_ */
