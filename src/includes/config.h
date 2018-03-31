/*
 * config.h
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_CONFIG_H_
#define BACKEND_SRC_CONFIG_H_

#include <stdbool.h>
#include <jansson.h>

bool config_init(void);
bool config_update_filename(const char* filename);

#endif /* BACKEND_SRC_CONFIG_H_ */
