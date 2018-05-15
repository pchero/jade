/*
 * park_handler.h
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */

#ifndef SRC_PARK_HANDLER_H_
#define SRC_PARK_HANDLER_H_

#include <evhtp.h>

bool park_init_handler(void);
bool park_term_handler(void);
bool park_reload_handler(void);

json_t* park_get_parkinglots_all(void);
json_t* park_get_parkinglot_info(const char* name);
bool park_create_parkinglot_info(const json_t* j_tmp);

json_t* park_get_parkedcalls_all();
json_t* park_get_parkedcall_info(const char* parkee_unique_id);
bool park_create_parkedcall_info(const json_t* j_tmp);
bool park_update_parkedcall_info(const json_t* j_data);
bool park_delete_parkedcall_info(const char* key);


// cfg
bool park_cfg_create_parkinglot_info(const json_t* j_data);
bool park_cfg_update_parkinglot_info(const json_t* j_data);
bool park_cfg_delete_parkinglot_info(const char* parkinglot);


#endif /* SRC_PARK_HANDLER_H_ */
