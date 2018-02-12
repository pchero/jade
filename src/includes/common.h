/*
 * common.h
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_COMMON_H_
#define BACKEND_SRC_COMMON_H_

#include <jansson.h>
#include <event.h>

#include "db_ctx_handler.h"

typedef struct _app {
  json_t*             j_conf;   ///< jade configuration
  struct event_base*  evt_base; ///< event base
} app;

typedef enum _E_USE
{
  E_USE_NO = 0,
  E_USE_OK = 1,
} E_USE;

extern db_ctx_t* g_db_ast;  ///< sqlite3 database for asterisk.
extern app* g_app;

#endif /* BACKEND_SRC_COMMON_H_ */
