/*
 * destination_handler.h
 *
 *  Created on: Oct 19, 2016
 *      Author: pchero
 */

#ifndef SRC_DESTINATION_HANDLER_H_
#define SRC_DESTINATION_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

typedef enum _E_DESTINATION_TYPE {
  DESTINATION_EXTEN       = 0,
  DESTINATION_APPLICATION = 1,
} E_DESTINATION_TYPE;

#define DEF_DESTINATION_AVAIL_CNT_UNLIMITED  -1

json_t* ob_create_destination(json_t* j_dest);
json_t* ob_delete_destination(const char* uuid);
json_t* ob_get_destination(const char* uuid);
json_t* ob_get_destinations_all(void);
json_t* ob_get_destinations_all_uuid(void);
json_t* ob_update_destination(const json_t* j_dest);

int ob_get_destination_available_count(json_t* j_dest);
json_t* ob_create_dial_destination_info(json_t* j_dest);

bool ob_validate_destination(json_t* j_data);

bool ob_is_exist_destination(const char* uuid);
bool ob_is_deletable_destination(const char* uuid);

#endif /* SRC_DESTINATION_HANDLER_H_ */
