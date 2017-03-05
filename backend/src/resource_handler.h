/*
 * resource_handler.h
 *
 *  Created on: Feb 10, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_RESOURCE_HANDLER_H_
#define BACKEND_SRC_RESOURCE_HANDLER_H_

json_t* get_peers_all_name(void);
json_t* get_databases_all_key(void);
json_t* get_database_info(const char* key);


#endif /* BACKEND_SRC_RESOURCE_HANDLER_H_ */
