/*
 * ob_dlma_handler.h
 *
 *  Created on: Mar 21, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_OB_DLMA_HANDLER_H_
#define BACKEND_SRC_OB_DLMA_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

bool validate_ob_dlma(json_t* j_data);
json_t* create_ob_dlma(json_t* j_dlma);
json_t* update_ob_dlma(const json_t* j_dlma);
json_t* delete_ob_dlma(const char* uuid, bool force);

json_t* get_ob_dlmas_all(void);
json_t* get_ob_dlmas_all_uuid(void);
json_t* get_ob_dlma(const char* uuid);
json_t* get_deleted_ob_dlma(const char* uuid);
char* get_ob_dlma_table_name(const char* dlma_uuid);

#endif /* BACKEND_SRC_OB_DLMA_HANDLER_H_ */
