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

json_t* create_ob_destination(const json_t* j_dest);
bool delete_ob_destination(const char* uuid);
json_t* get_ob_destination(const char* uuid);
json_t* get_ob_destinations_all(void);
json_t* get_ob_destinations_all_uuid(void);
bool update_ob_destination(const json_t* j_dest);

int get_ob_destination_available_count(json_t* j_dest);
json_t* create_ob_dial_destination_info(json_t* j_dest);

#endif /* SRC_DESTINATION_HANDLER_H_ */
