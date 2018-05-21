/*
 * dialplan_handler.h
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#ifndef SRC_DIALPLAN_HANDLER_H_
#define SRC_DIALPLAN_HANDLER_H_

#include <stdbool.h>
#include <evhtp.h>

bool dialplan_init_handler(void);
bool dialplan_term_handler(void);
bool dialplan_reload_handler(void);

bool dialplan_reload_asterisk(void);


// dpma
json_t* dialplan_get_dpmas_all(void);
json_t* dialplan_get_dpma_info(const char* key);
bool dialplan_create_dpma_info(const json_t* j_data);
bool dialplan_update_dpma_info(const json_t* j_data);
bool dialplan_delete_dpma_info(const char* uuid);

// dialplan
json_t* dialplan_get_dialplans_all(void);
json_t* dialplan_get_dialplans_by_dpma_uuid_order_sequence(const char* dpma_uuid);
json_t* dialplan_get_dialplan_info(const char* key);
json_t* dialplan_get_dialplan_info_by_dpma_seq(const char* dpma_uuid, int seq);

bool dialplan_create_dialplan_info(const json_t* j_data);
bool dialplan_update_dialplan_info(const json_t* j_data);
bool dialplan_delete_dialplan_info(const char* uuid);

bool dialplan_add_cmds(const char* agi_uuid);

char* dialplan_get_default_dpma_originate_to_device(void);

// static dialplan
json_t* dialplan_get_sdialplans_all(void);
json_t* dialplan_get_sdialplan_info(const char* name);
bool dialplan_create_sdialplan_info(const json_t* j_data);
bool dialplan_update_sdialplan_info(const json_t* j_data);
bool dialplan_delete_sdialplan_info(const char* name);

// configuration
json_t* dialplan_get_configurations_all(void);
json_t* dialplan_get_configuration_info(const char* name);
bool dialplan_update_configuration_info(const json_t* j_data);
bool dialplan_delete_configuration_info(const char* name);



#define DEF_DIALPLAN_JADE_AGI_NAME   "jade_dialplan"

#endif /* SRC_DIALPLAN_HANDLER_H_ */
