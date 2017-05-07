/*
 * cb_handler_ctx.c
 *
 *  Created on: Apr 27, 2017
 *      Author: pchero
 */

#include <stdio.h>
#include <stdlib.h>

#include "db_handler_ctx.h"
#include "slog.h"
#include "utils.h"

typedef struct {
  int max_retry;  /* Max retry times. */
  int sleep_ms;   /* Time to sleep before retry again. */
} busy_handler_attr;


static bool db_ctx_connect(db_ctx_t* ctx, const char* filename);
static db_ctx_t* db_ctx_create(void);
static int db_ctx_busy_handler(void *data, int retry);

static db_ctx_t* db_ctx_create(void)
{
  db_ctx_t* ctx;

  ctx = calloc(1, sizeof(db_ctx_t));
  if(ctx == NULL) {
    slog(LOG_ERR, "Could not create new db context.");
    return NULL;
  }
  ctx->db = NULL;
  ctx->stmt = NULL;

  return ctx;
}

/**
 Connect to db.

 @return Success:TRUE, Fail:FALSE
 */
static bool db_ctx_connect(db_ctx_t* ctx, const char* filename)
{
  int ret;

  if((ctx == NULL) || (filename == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  if(ctx->db != NULL) {
    slog(LOG_INFO, "Database is already connected.");
    return true;
  }

  ret = sqlite3_open(filename, &ctx->db);
  if(ret != SQLITE_OK) {
    slog(LOG_ERR, "Could not initiate database. err[%s]", sqlite3_errmsg(ctx->db));
    return false;
  }
  slog(LOG_DEBUG, "Connected to database ctx. filename[%s]", filename);

  return true;
}

/**
 * free ctx stmt
 * @param ctx
 */
bool db_ctx_free(db_ctx_t* ctx)
{
  int ret;

  if(ctx == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
  }

  // check already freed
  if(ctx->stmt == NULL) {
    return true;
  }

  ret = sqlite3_finalize(ctx->stmt);
  if(ret != SQLITE_OK) {
    slog(LOG_ERR, "Could not finalize stme. ret[%d]", ret);
    return false;
  }
  ctx->stmt = NULL;

  return true;
}

/*
 * Busy callback handler for all operations. If the busy callback handler is
 * NULL, then SQLITE_BUSY or SQLITE_IOERR_BLOCKED is returned immediately upon
 * encountering the lock. If the busy callback is not NULL, then the callback
 * might be invoked.
 *
 * This callback will be registered by SQLite's API:
 *    int sqlite3_busy_handler(sqlite3*, int(*)(void*,int), void*);
 * That's very useful to deal with SQLITE_BUSY event automatically. Otherwise,
 * you have to check the return code, reset statement and do retry manually.
 *
 * We've to use ANSI C declaration here to eliminate warnings in Visual Studio.
 */
static int db_ctx_busy_handler(void *data, int retry)
{
  busy_handler_attr* attr;

  attr = (busy_handler_attr*)data;

  if (retry < attr->max_retry) {
    /* Sleep a while and retry again. */
    slog(LOG_DEBUG, "Hits SQLITE_BUSY %d times, retry again.", retry);
    sqlite3_sleep(attr->sleep_ms);

    /* Return non-zero to let caller retry again. */
    return 1;
  }

  /* Return zero to let caller return SQLITE_BUSY immediately. */
  slog(LOG_ERR, "Retried too many, exit. retry[%d]", retry);
  return 0;
}


/**
 * Init database
 * @return
 */
db_ctx_t* db_ctx_init(const char* name)
{
  int ret;
  db_ctx_t* db_ctx;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Initiate db context. name[%s]", name);

  // create new db_ctx
  db_ctx = db_ctx_create();
  if(db_ctx == NULL) {
    slog(LOG_ERR, "Could not create db context.");
    return NULL;
  }

  // connect db
  ret = db_ctx_connect(db_ctx, name);
  if(ret == false) {
    sfree(db_ctx);
    return NULL;
  }

  return db_ctx;
}

/**
 Disconnect to db.
 */
void db_ctx_term(db_ctx_t* ctx)
{
  int ret;

  if(ctx == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  db_ctx_free(ctx);

  ret = sqlite3_close(ctx->db);
  if(ret != SQLITE_OK) {
    slog(LOG_ERR, "Could not close the database correctly. err[%s]", sqlite3_errmsg(ctx->db));
    return;
  }

  slog(LOG_DEBUG, "Released database context.");
  ctx->db = NULL;

  sfree(ctx);
}

/**
 database query function. (select)
 @param query
 @return Success:, Fail:NULL
 */
bool db_ctx_query(db_ctx_t* ctx, const char* query)
{
  int ret;
  sqlite3_stmt* result;

  if((ctx == NULL) || (query == NULL) || (ctx->db == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // free ctx stmt if exists
  db_ctx_free(ctx);

  ret = sqlite3_prepare_v2(ctx->db, query, -1, &result, NULL);
  if(ret != SQLITE_OK) {
    slog(LOG_ERR, "Could not prepare query. query[%s], err[%s]", query, sqlite3_errmsg(ctx->db));
    return false;
  }

  ctx->stmt = result;
  return true;
}

/**
 * database query execute function. (update, delete, insert)
 * @param query
 * @return  success:true, fail:false
 */
bool db_ctx_exec(db_ctx_t* ctx, const char* query)
{
  int ret;
  char* err;
  busy_handler_attr bh_attr;

  if((ctx == NULL) || (query == NULL) || (ctx->db == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  /* Setup busy handler for all following operations. */
  bh_attr.max_retry = 100;  /* Max retry times */
  bh_attr.sleep_ms = 100;   /* Sleep 100ms before each retry */
  sqlite3_busy_handler(ctx->db, db_ctx_busy_handler, &bh_attr);

  // execute
  ret = sqlite3_exec(ctx->db, query, NULL, 0, &err);
  if(ret != SQLITE_OK) {
    slog(LOG_ERR, "Could not execute query. query[%s], err[%s]", query, err);
    sqlite3_free(err);
    return false;
  }
  sqlite3_free(err);

  return true;
}

/**
 * Return 1 record info by json.
 * If there's no more record or error happened, it will return NULL.
 * @param res
 * @return  success:json_t*, fail:NULL
 */
json_t* db_ctx_get_record(db_ctx_t* ctx)
{
	int ret;
	int cols;
	int i;
	json_t* j_res;
	json_t* j_tmp;
	int type;
	const char* tmp_const;

  if(ctx == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

	ret = sqlite3_step(ctx->stmt);
	if(ret != SQLITE_ROW) {
		if(ret != SQLITE_DONE) {
		  slog(LOG_ERR, "Could not patch the result. ret[%d], err[%s]", ret, sqlite3_errmsg(ctx->db));
		}
		return NULL;
	}

	cols = sqlite3_column_count(ctx->stmt);
	j_res = json_object();
	for(i = 0; i < cols; i++) {
		j_tmp = NULL;
		type = sqlite3_column_type(ctx->stmt, i);
		switch(type) {
			case SQLITE_INTEGER: {
				j_tmp = json_integer(sqlite3_column_int(ctx->stmt, i));
			}
			break;

			case SQLITE_FLOAT: {
				j_tmp = json_real(sqlite3_column_double(ctx->stmt, i));
			}
			break;

			case SQLITE_NULL: {
				j_tmp = json_null();
			}
			break;

			case SQLITE3_TEXT:
			{
			  // if the text is loadable, create json object.
			  tmp_const = (const char*)sqlite3_column_text(ctx->stmt, i);
			  if(tmp_const == NULL) {
			    j_tmp = json_null();
			  }
			  else {
	        j_tmp = json_loads(tmp_const, JSON_DECODE_ANY, NULL);
	        if(j_tmp == NULL) {
	          j_tmp = json_string((const char*)sqlite3_column_text(ctx->stmt, i));
	        }
	        else {
	          // check type
            // the only array/object/string types are allowed
	          // especially, we don't allow the JSON_NULL type at this point.
	          // Cause the json_loads() consider the "null" string to JSON_NULL.
	          // It's should be done at the above.
            ret = json_typeof(j_tmp);
            if((ret != JSON_ARRAY) && (ret != JSON_OBJECT) && (ret != JSON_STRING)) {
              json_decref(j_tmp);
              j_tmp = json_string((const char*)sqlite3_column_text(ctx->stmt, i));
            }
	        }
			  }
			}
			break;

			case SQLITE_BLOB:
			default:
			{
				// not done yet.
			  slog(LOG_NOTICE, "Not supported type. type[%d]", type);
				j_tmp = json_null();
			}
			break;
		}

		if(j_tmp == NULL) {
		  slog(LOG_ERR, "Could not parse result column. name[%s], type[%d]",
					sqlite3_column_name(ctx->stmt, i), type);
			j_tmp = json_null();
		}
		json_object_set_new(j_res, sqlite3_column_name(ctx->stmt, i), j_tmp);
	}

	return j_res;
}
