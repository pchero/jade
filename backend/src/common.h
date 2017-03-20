/*
 * common.h
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_COMMON_H_
#define BACKEND_SRC_COMMON_H_

#include <jansson.h>

typedef struct _app {
  json_t* j_conf;
} app;

typedef enum _E_USE
{
  E_USE_NO = 0,
  E_USE_OK = 1,
} E_USE;



#endif /* BACKEND_SRC_COMMON_H_ */
