/*
 * config.h
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_CONFIG_H_
#define BACKEND_SRC_CONFIG_H_

bool init_config(void);
bool update_config_filename(const char* filename);

json_t* get_ast_current_config_info(const char* filename);
char* get_ast_current_config_info_raw(const char* filename);

int update_ast_current_config_info(const char* filename, json_t* j_conf);
int update_ast_config_info_raw(const char* filename, const char* data);

bool update_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);
bool create_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val);

bool update_ast_current_config_section_data(const char* filename, const char* section, json_t* j_data);
bool create_ast_current_config_section_data(const char* filename, const char* section, json_t* j_data);

json_t* get_ast_backup_config_info(const char* filename);
json_t* get_ast_backup_configs_info_all(const char* filename);
int remove_ast_backup_config_info(const char* filename);

#endif /* BACKEND_SRC_CONFIG_H_ */
