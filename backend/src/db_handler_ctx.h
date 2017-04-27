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
  static sqlite3* db;

  struct sqlite3_stmt* stmt;
} db_ctx_t;

bool      db_init(void);
void      db_term(void);
db_res_t* db_query(const char* query);
bool      db_exec(const char* query);
void      db_free(db_res_t* ctx);
bool      db_insert(const char* table, const struct json_t* j_data);
bool      db_insert_or_replace(const char* table, const struct json_t* j_data);
char*     db_get_update_str(const struct json_t* j_data);
struct json_t*  db_get_record(db_res_t* ctx);

// need ctx

#endif /* SRC_DB_SQLITE3_HANDLER_H_ */




#endif /* BACKEND_SRC_DB_HANDLER_CTX_H_ */
