/*
 * sqlite3_handler.c
 *
 *  Created on: Oct 11, 2016
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <jansson.h>

#include "slog.h"
#include "utils.h"
#include "db_handler.h"

extern app* g_app;
static sqlite3* g_db = NULL;

#define MAX_BIND_BUF 4096
#define DELIMITER   0x02
#define MAX_MEMDB_LOCK_RELEASE_TRY 100
#define MAX_MEMDB_LOCK_RELEASE_TRY 100

typedef struct {
	int max_retry;  /* Max retry times. */
	int sleep_ms;   /* Time to sleep before retry again. */
} busy_handler_attr;


static bool db_sqlite3_connect(const char* filename);
static bool db_insert_basic(const char* table, const json_t* j_data, int replace);
//static bool db_sqlite3_lock(void);
//static bool db_sqlite3_release(void);
//static void db_sqlite3_msleep(unsigned long milisec);
static int db_sqlite3_busy_handler(void *data, int retry);

bool db_init(void)
{
	int ret;
	const char* name;

	// get database name
	name = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "database_name"));
	if(name == NULL) {
	  slog(LOG_ERR, "Could not get database name.");
	  return false;
	}

	// db connect
	ret = db_sqlite3_connect(name);
	if(ret == false) {
		slog(LOG_ERR, "Could not initiate sqlite3 database.");
		return false;
	}

	return true;
}

/**
 Connect to db.

 @return Success:TRUE, Fail:FALSE
 */
static bool db_sqlite3_connect(const char* filename)
{
	int ret;

	if(g_db != NULL) {
		slog(LOG_NOTICE, "Database is already connected.");
		return true;
	}

	ret = sqlite3_open(filename, &g_db);
	if(ret != SQLITE_OK) {
		slog(LOG_ERR, "Could not initiate database. err[%s]", sqlite3_errmsg(g_db));
		return false;
	}
	slog(LOG_DEBUG, "Connected to database. filename[%s]", filename);

	return true;
}

/**
 Disconnect to db.
 */
void db_term(void)
{
	int ret;

	if(g_db == NULL) {
		slog(LOG_NOTICE, "Released database context already.");
		return;
	}

	ret = sqlite3_close(g_db);
	if(ret != SQLITE_OK) {
		slog(LOG_WARNING, "Could not close the database correctly. err[%s]", sqlite3_errmsg(g_db));
		return;
	}

	slog(LOG_NOTICE, "Released database context.");
	g_db = NULL;
}

/**
 database query function. (select)
 @param query
 @return Success:, Fail:NULL
 */
db_res_t* db_query(const char* query)
{
	int ret;
	sqlite3_stmt* result;
	db_res_t* db_res;

	if(query == NULL) {
		slog(LOG_WARNING, "Could not query NULL query.");
		return NULL;
	}

	ret = sqlite3_prepare_v2(g_db, query, -1, &result, NULL);
	if(ret != SQLITE_OK) {
		slog(LOG_ERR, "Could not prepare query. query[%s], err[%s]", query, sqlite3_errmsg(g_db));
		return NULL;
	}

	db_res = calloc(1, sizeof(db_res_t));
	db_res->res = result;

	return db_res;
}

/**
 * database query execute function. (update, delete, insert)
 * @param query
 * @return  success:true, fail:false
 */
bool db_exec(const char* query)
{
	int ret;
	char* err;
	busy_handler_attr bh_attr;

	if(query == NULL) {
		slog(LOG_WARNING, "Could not execute NULL query.");
		return false;
	}

	/* Setup busy handler for all following operations. */
	bh_attr.max_retry = 100;  /* Max retry times */
	bh_attr.sleep_ms = 100;   /* Sleep 100ms before each retry */
	sqlite3_busy_handler(g_db, db_sqlite3_busy_handler, &bh_attr);

	// execute
	ret = sqlite3_exec(g_db, query, NULL, 0, &err);
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
json_t* db_get_record(db_res_t* ctx)
{
	int ret;
	int cols;
	int i;
	json_t* j_res;
	json_t* j_tmp;
	int type;
	const char* tmp_const;

	ret = sqlite3_step(ctx->res);
	if(ret != SQLITE_ROW) {
		if(ret != SQLITE_DONE) {
			slog(LOG_ERR, "Could not patch the result. ret[%d], err[%s]", ret, sqlite3_errmsg(g_db));
		}
		return NULL;
	}

	cols = sqlite3_column_count(ctx->res);
	j_res = json_object();
	for(i = 0; i < cols; i++) {
		j_tmp = NULL;
		type = sqlite3_column_type(ctx->res, i);
		switch(type) {
			case SQLITE_INTEGER: {
				j_tmp = json_integer(sqlite3_column_int(ctx->res, i));
			}
			break;

			case SQLITE_FLOAT: {
				j_tmp = json_real(sqlite3_column_double(ctx->res, i));
			}
			break;

			case SQLITE_NULL: {
				j_tmp = json_null();
			}
			break;

			case SQLITE3_TEXT:
			{
			  // if the text is loadable, create json object.
			  tmp_const = (const char*)sqlite3_column_text(ctx->res, i);
			  if(tmp_const == NULL) {
			    j_tmp = json_null();
			  }
			  else {
	        j_tmp = json_loads(tmp_const, JSON_DECODE_ANY, NULL);
	        if(j_tmp == NULL) {
	          j_tmp = json_string((const char*)sqlite3_column_text(ctx->res, i));
	        }
	        else {
	          // check type
            // the only array/object/string/null types are allowed
            ret = json_typeof(j_tmp);
            if((ret != JSON_ARRAY) && (ret != JSON_OBJECT) && (ret != JSON_STRING) && (ret != JSON_NULL)) {
              json_decref(j_tmp);
              j_tmp = json_string((const char*)sqlite3_column_text(ctx->res, i));
            }
	        }
			  }
			}
			break;

			case SQLITE_BLOB:
			default:
			{
				// not done yet.
				slog(LOG_WARNING, "Not supported type. type[%d]", type);
				j_tmp = json_null();
			}
			break;
		}

		if(j_tmp == NULL) {
			slog(LOG_WARNING, "Could not parse result column. name[%s], type[%d]",
					sqlite3_column_name(ctx->res, i), type);
			j_tmp = json_null();
		}
		json_object_set_new(j_res, sqlite3_column_name(ctx->res, i), j_tmp);
	}

	return j_res;
}

/**
 *
 * @param ctx
 */
void db_free(db_res_t* db_res)
{
	if(db_res == NULL) {
		return;
	}

	sqlite3_finalize(db_res->res);
	sfree(db_res);

	return;
}

/**
 * Insert j_data into table.
 * @param table
 * @param j_data
 * @return
 */
static bool db_insert_basic(const char* table, const json_t* j_data, int replace)
{
  char* sql;
  json_t*     j_data_cp;
  char*       tmp;
  const char* key;
  json_t*     j_val;
  int         ret;
  json_type   type;
  char*       sql_keys;
  char*       sql_values;
  char*       tmp_sub;
  char*       tmp_sqlite_buf; // sqlite3_mprintf

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "db_insert_basic.");

  // copy original.
  j_data_cp = json_deep_copy(j_data);

  tmp = NULL;
  sql_keys  = NULL;
  sql_values = NULL;
  tmp_sub = NULL;
  json_object_foreach(j_data_cp, key, j_val) {

    // key
    if(sql_keys == NULL) {
      asprintf(&tmp, "%s", key);
    }
    else {
      asprintf(&tmp, "%s, %s", sql_keys, key);
    }
    sfree(sql_keys);
    asprintf(&sql_keys, "%s", tmp);
    sfree(tmp);

    // value
    sfree(tmp_sub);

    // get type.
    type = json_typeof(j_val);
    switch(type) {
      // string
      case JSON_STRING: {
        asprintf(&tmp_sub, "\'%s\'", json_string_value(j_val));
      }
      break;

      // numbers
      case JSON_INTEGER: {
        asprintf(&tmp_sub, "%lld", json_integer_value(j_val));
      }
      break;

      case JSON_REAL: {
        asprintf(&tmp_sub, "%f", json_real_value(j_val));
      }
      break;

      // true
      case JSON_TRUE: {
        asprintf(&tmp_sub, "\"%s\"", "true");
      }
      break;

      // false
      case JSON_FALSE: {
        asprintf(&tmp_sub, "\"%s\"", "false");
      }
      break;

      case JSON_NULL: {
        asprintf(&tmp_sub, "%s", "null");
      }
      break;

      case JSON_ARRAY:
      case JSON_OBJECT: {
        tmp = json_dumps(j_val, JSON_ENCODE_ANY);
        tmp_sqlite_buf = sqlite3_mprintf("%q", tmp);
        sfree(tmp);

        asprintf(&tmp_sub, "'%s'", tmp_sqlite_buf);
        sqlite3_free(tmp_sqlite_buf);
      }
      break;

      // object
      // array
      default: {
        // Not done yet.

        // we don't support another types.
        slog(LOG_WARNING, "Wrong type input. We don't handle this.");
        asprintf(&tmp_sub, "\"%s\"", "null");
      }
      break;
    }

    if(sql_values == NULL) {
      asprintf(&tmp, "%s", tmp_sub);
    }
    else {
      asprintf(&tmp, "%s, %s", sql_values, tmp_sub);
    }
    sfree(tmp_sub);
    sfree(sql_values);
    sql_values = strdup(tmp);
    sfree(tmp);
  }
  json_decref(j_data_cp);

  if(replace == true) {
    asprintf(&sql, "insert or replace into %s(%s) values (%s);", table, sql_keys, sql_values);
  }
  else {
    asprintf(&sql, "insert into %s(%s) values (%s);", table, sql_keys, sql_values);
  }
  sfree(sql_keys);
  sfree(sql_values);

  ret = db_exec(sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert data.");
    return false;
  }

  return true;
}

/**
 * Insert j_data into table.
 * @param table
 * @param j_data
 * @return
 */
bool db_insert(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_insert");

  ret = db_insert_basic(table, j_data, false);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert data.");
    return false;
  }

  return true;
}

/**
 * Insert or replace j_data into table.
 * @param table
 * @param j_data
 * @return
 */
bool db_insert_or_replace(const char* table, const json_t* j_data)
{
  int ret;

  if((table == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_insert_or_replace");

  ret = db_insert_basic(table, j_data, true);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert data.");
    return false;
  }

  return true;
}


/**
 * Return part of update sql.
 * @param j_data
 * @return
 */
char* db_get_update_str(const json_t* j_data)
{
  char*   res;
  char*   tmp;
  char*   tmp_sub;
  char*   tmp_sqlite_buf;
  json_t*  j_val;
  json_t*  j_data_cp;
  const char* key;
  bool        is_first;
  json_type   type;

  // copy original data.
  j_data_cp = json_deep_copy(j_data);

  is_first = true;
  res = NULL;
  tmp = NULL;
  tmp_sub = NULL;

  json_object_foreach(j_data_cp, key, j_val) {

    // create update string
    type = json_typeof(j_val);
    switch(type) {
      // string
      case JSON_STRING: {
        asprintf(&tmp_sub, "%s = \'%s\'", key, json_string_value(j_val));
      }
      break;

      // numbers
      case JSON_INTEGER: {
        asprintf(&tmp_sub, "%s = %lld", key, json_integer_value(j_val));
      }
      break;

      case JSON_REAL: {
        asprintf(&tmp_sub, "%s = %lf", key, json_real_value(j_val));
      }
      break;

      // true
      case JSON_TRUE: {
        asprintf(&tmp_sub, "%s = \"%s\"", key, "true");
      }
      break;

      // false
      case JSON_FALSE: {
        asprintf(&tmp_sub, "%s = \"%s\"", key, "false");
      }
      break;

      case JSON_NULL: {
        asprintf(&tmp_sub, "%s = %s", key, "null");
      }
      break;

      case JSON_ARRAY:
      case JSON_OBJECT: {
        tmp = json_dumps(j_val, JSON_ENCODE_ANY);
        tmp_sqlite_buf = sqlite3_mprintf("%q", tmp);
        sfree(tmp);

        asprintf(&tmp_sub, "%s = '%s'", key, tmp_sqlite_buf);
        sqlite3_free(tmp_sqlite_buf);
      }
      break;

      default: {
        // Not done yet.
        // we don't support another types.
        slog(LOG_WARNING, "Wrong type input. We don't handle this.");
        asprintf(&tmp_sub, "%s = %s", key, "null");
      }
      break;
    }

    // copy/set previous sql.
    sfree(tmp);
    if(is_first == true) {
      asprintf(&tmp, "%s", tmp_sub);
      is_first = false;
    }
    else {
      asprintf(&tmp, "%s, %s", res, tmp_sub);
    }
    sfree(res);
    sfree(tmp_sub);

    res = strdup(tmp);
    sfree(tmp);
  }
  json_decref(j_data_cp);

  return res;
}

///**
// * Do the database lock.
// * Keep try MAX_DB_ACCESS_TRY.
// * Each try has 100 ms delay.
// */
//static bool db_sqlite3_lock(void)
//{
//	int ret;
//	int i;
//	char* err;
//
//	for(i = 0; i < MAX_MEMDB_LOCK_RELEASE_TRY; i++) {
//		ret = sqlite3_exec(g_db, "BEGIN IMMEDIATE", NULL, 0, &err);
//		sqlite3_free(err);
//		if(ret == SQLITE_OK) {
//			break;
//		}
//		db_sqlite3_msleep(100);
//	}
//
//	if(ret != SQLITE_OK) {
//		return false;
//	}
//	return true;
//}
//
//
///**
// * Release database lock.
// * Keep try MAX_MEMDB_LOCK_RELEASE_TRY.
// * Each try has 100 ms delay.
// */
//static bool db_sqlite3_release(void)
//{
//	int ret;
//	int i;
//	char* err;
//
//	for(i = 0; i < MAX_MEMDB_LOCK_RELEASE_TRY; i++) {
//		ret = sqlite3_exec(g_db, "COMMIT", NULL, 0, &err);
//		sqlite3_free(err);
//		if(ret == SQLITE_OK) {
//			break;
//		}
//		db_sqlite3_msleep(100);
//	}
//
//	if(ret != SQLITE_OK) {
//		return false;
//	}
//	return true;
//}

///**
// * Millisecond sleep.
// * @param milisec
// */
//static void db_sqlite3_msleep(unsigned long milisec)
//{
//    struct timespec req = {0, 0};
//    time_t sec = (int)(milisec/1000);
//    milisec = milisec - (sec * 1000);
//
//    req.tv_sec = sec;
//    req.tv_nsec = milisec * 1000000L;
//
//    while(nanosleep(&req, &req) == -1)
//    {
//        continue;
//    }
//    return;
//}

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
static int db_sqlite3_busy_handler(void *data, int retry)
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
