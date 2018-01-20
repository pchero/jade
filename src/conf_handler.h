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
char* get_ast_current_config_info_raw(const char* filename);

int update_ast_current_config_info(const char* filename, json_t* j_conf);
int update_ast_config_info_raw(const char* filename, const char* data);

bool create_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool update_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool delete_ast_current_config_content(const char* filename, const char* section, const char* key);

bool delete_ast_current_config_section(const char* filename, const char* section);
bool update_ast_current_config_section_data(const char* filename, const char* section, json_t* j_data);
bool create_ast_current_config_section_data(const char* filename, const char* section, json_t* j_data);


json_t* get_ast_backup_config_info(const char* filename);
json_t* get_ast_backup_configs_info_all(const char* filename);
int remove_ast_backup_config_info(const char* filename);


#endif /* SRC_CONF_HANDLER_H_ */
