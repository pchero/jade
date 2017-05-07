/*
 * db_handler_ctx.h
 *
 *  Created on: Apr 27, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_DB_HANDLER_CTX_H_
#define BACKEND_SRC_DB_HANDLER_CTX_H_

#include <sqlite3.h>
#include <stdbool.h>

#include "common.h"

typedef struct _db_ctx_t
{
  struct sqlite3* db;

  struct sqlite3_stmt* stmt;
} db_ctx_t;

db_ctx_t* db_ctx_init(const char* name);
void db_ctx_term(db_ctx_t* ctx);

bool db_ctx_exec(db_ctx_t* ctx, const char* query);
bool db_ctx_query(db_ctx_t* ctx, const char* query);
json_t* db_ctx_get_record(db_ctx_t* ctx);

bool db_ctx_free(db_ctx_t* ctx);


#endif /* BACKEND_SRC_DB_HANDLER_CTX_H_ */
