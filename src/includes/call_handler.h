/*
 * call_handler.h
 *
 *  Created on: Apr 13, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_CALL_HANDLER_H_
#define SRC_INCLUDES_CALL_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

bool call_init_handler(void);
bool call_term_handler(void);
bool call_reload_handler(void);

json_t* call_get_channels_all(void);
json_t* call_get_channel_info(const char* unique_id);
bool call_create_channel_info(const json_t* j_data);
int call_update_channel_info(const json_t* j_tmp);
int call_delete_channel_info(const char* key);


#endif /* SRC_INCLUDES_CALL_HANDLER_H_ */
