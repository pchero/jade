/*
 * resource_handler.h
 *
 *  Created on: Feb 10, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_RESOURCE_HANDLER_H_
#define BACKEND_SRC_RESOURCE_HANDLER_H_

json_t* get_peers_all_name(void);

// database
json_t* get_databases_all_key(void);
json_t* get_database_info(const char* key);

// registry
json_t* get_registries_all_account(void);
json_t* get_registry_info(const char* account);

#endif /* BACKEND_SRC_RESOURCE_HANDLER_H_ */
