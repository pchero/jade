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

bool init_conf_handler(void);

json_t* get_ast_current_config_info(const char* filename);
char* get_ast_current_config_info_text(const char* filename);

bool update_ast_current_config_info(const char* filename, json_t* j_conf);
bool update_ast_current_config_info_text(const char* filename, const char* data);

bool create_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool update_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool delete_ast_current_config_content(const char* filename, const char* section, const char* key);

bool create_ast_setting(const char* filename, const char* name, const json_t* j_data);
json_t* get_ast_settings_all(const char* filename);
json_t* get_ast_setting(const char* filename, const char* name);
bool update_ast_setting(const char* filename, const char* name, const json_t* j_data);
bool remove_ast_setting(const char* filename, const char* name);

json_t* get_ast_backup_config_info_json(const char* filename);
char* get_ast_backup_config_info_text(const char* filename);
char* get_ast_backup_config_info_text_valid(const char* filename, const char* valid);

json_t* get_ast_backup_configs_info_all(const char* filename);
bool remove_ast_backup_config_info_valid(const char* filename, const char* valid);


#endif /* SRC_CONF_HANDLER_H_ */
