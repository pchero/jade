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

bool ob_validate_dlma(json_t* j_data);
json_t* ob_create_dlma(json_t* j_dlma);
json_t* ob_update_dlma(const json_t* j_dlma);
json_t* ob_delete_dlma(const char* uuid);
json_t* ob_get_dlma(const char* uuid);
json_t* ob_get_deleted_dlma(const char* uuid);

json_t* ob_get_dlmas_all(void);
json_t* ob_get_dlmas_all_uuid(void);
char* ob_get_dlma_table_name(const char* dlma_uuid);

bool ob_is_dlma_deletable(const char* uuid);


#endif /* BACKEND_SRC_OB_DLMA_HANDLER_H_ */
