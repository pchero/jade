/*
 * ami_event_handler.c
 *
 *  Created on: Mar 4, 2017
 *      Author: pchero
 */

#include <string.h>
#include <jansson.h>

#include "utils.h"
#include "slog.h"
#include "db_handler.h"

void ami_response_handler_databaseshow(json_t* j_msg)
{
  char* tmp;
  const char* tmp_const;
  int idx;
  json_t* j_val;
  json_t* j_tmp;
  char* value;
  char* dump;
  char* key;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_response_handler_databaseshow.");

  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "received message. msg[%s]", tmp);
  sfree(tmp);

  // need to parse
  json_array_foreach(json_object_get(j_msg, "Output"), idx, j_val) {
    tmp_const = json_string_value(j_val);

    // get key/value
    value = strdup(tmp_const);
    dump = value;
    key = strsep(&value, ":");
    if((key == NULL) || (value == NULL)) {
      sfree(dump);
      continue;
    }
    trim(key);
    trim(value);

    slog(LOG_DEBUG, "Check database key/value. key[%s], value[%s]", key, value);
    j_tmp = json_pack("{s:s, s:s}",
        "key",    key,
        "value",  value
        );

    ret = db_insert_or_replace("database", j_tmp);
    json_decref(j_tmp);
    if(ret == false) {
      slog(LOG_ERR, "Could not insert/replace the database data. key[%s], value[%s]", key, value);
    }

    sfree(dump);
  }
}
