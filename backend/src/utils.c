/*
 * utils.c
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
#include <ctype.h>

#include "utils.h"

#define DEF_UUID_STR_LEN 37

void sfree(void* tmp)
{
  if(tmp == NULL) {
    return;
  }

  free(tmp);

  return;
}

/**
 *
 * @param s
 */
void trim(char* s)
{
	char* p;
	int l;

	if(s == NULL) {
		return;
	}

	p = s;
	l = strlen(p);

	while(isspace(p[l - 1])) p[--l] = 0;
	while(* p && isspace(* p)) ++p, --l;

	memmove(s, p, l + 1);
}

/**
 * Generate uuid.
 * Return value should be free after used.
 * @param prefix
 * @return
 */
char* gen_uuid(void)
{
  uuid_t uuid;
  char tmp[DEF_UUID_STR_LEN];
  char* res;

  uuid_generate(uuid);
  uuid_unparse_lower(uuid, tmp);

  res = strdup(tmp);

  return res;
}

/**
 * return utc time.
 * YYYY-MM-DDTHH:mm:ssZ
 * @return
 */
char* get_utc_timestamp(void)
{
  char  timestr[128];
  char*   res;
  struct  timespec timeptr;
  time_t  tt;
  struct tm *t;

  clock_gettime(CLOCK_REALTIME, &timeptr);
  tt = (time_t)timeptr.tv_sec;
  t = gmtime(&tt);

  strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", t);
  asprintf(&res, "%s.%ldZ", timestr, timeptr.tv_nsec);

  return res;
}

/**
 * return utc time.
 * YYYY-MM-DDTHH:mm:ssZ
 * @return
 */
char* get_utc_timestamp_using_timespec(struct timespec timeptr)
{
  char  timestr[128];
  char*   res;
  time_t  tt;
  struct tm *t;

  tt = (time_t)timeptr.tv_sec;
  t = gmtime(&tt);

  strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", t);
  asprintf(&res, "%s.%ldZ", timestr, timeptr.tv_nsec);

  return res;
}

/**
 * return utc time.
 * YYYY-MM-DD
 * @return
 */
char* get_utc_timestamp_time(void)
{
  char  timestr[128];
  char*   res;
  struct  timespec timeptr;
  time_t  tt;
  struct tm *t;

  clock_gettime(CLOCK_REALTIME, &timeptr);
  tt = (time_t)timeptr.tv_sec;
  t = gmtime(&tt);

  strftime(timestr, sizeof(timestr), "%H:%M:%S", t);
  asprintf(&res, "%s", timestr);

  return res;
}

/**
 * return utc time.
 * YYYY-MM-DD
 * @return
 */
char* get_utc_timestamp_date(void)
{
  char  timestr[128];
  char*   res;
  struct  timespec timeptr;
  time_t  tt;
  struct tm *t;

  clock_gettime(CLOCK_REALTIME, &timeptr);
  tt = (time_t)timeptr.tv_sec;
  t = gmtime(&tt);

  strftime(timestr, sizeof(timestr), "%Y-%m-%d", t);
  asprintf(&res, "%s", timestr);

  return res;
}

/**
 * return utc day.
 * 0=Sunday, 1=Monday, ..., 6=Saturday
 * @return
 */
int get_utc_timestamp_day(void)
{
  struct tm tm;
  time_t time;
  char* tmp;

  tmp = get_utc_timestamp_date();
  if(strptime(tmp, "%Y-%m-%d", &tm) == NULL) {
    sfree(tmp);
    return -1;
  }
  sfree(tmp);

  return localtime(&time)->tm_wday;
}

///**
// *
// * @param str
// * @return
// */
//char* get_variables_info_ami_str_from_string(const char* str)
//{
//  struct json* j_tmp;
//  struct ast_json_iter* iter;
//  char* variable;
//  char* variables;
//
//  if((str == NULL) || (strlen(str) == 0)) {
//    return NULL;
//  }
//
//  j_tmp = ast_json_load_string(str, NULL);
//
//  variables = NULL;
//  for(iter = ast_json_object_iter(j_tmp);
//      iter != NULL;
//      iter = ast_json_object_iter_next(j_tmp, iter))
//  {
//    ast_asprintf(&variable, "%sVariable: %s=%s\r\n",
//        variables? : "",
//        ast_json_object_iter_key(iter)? : "",
//        ast_json_string_get(ast_json_object_iter_value(iter))? : ""
//        );
//    if(variables != NULL) {
//      ast_free(variables);
//    }
//    variables = variable;
//  }
//  AST_JSON_UNREF(j_tmp);
//
//  return variables;
//}
//
//char* get_variables_info_ami_str(struct ast_json* j_obj, const char* name)
//{
//  const char* tmp_const;
//  char* variables;
//
//
//  if((j_obj == NULL) || (name == NULL)) {
//    return NULL;
//  }
//
//  tmp_const = ast_json_string_get(ast_json_object_get(j_obj, name));
//  if((tmp_const == NULL) || (strlen(tmp_const) == 0)) {
//    return NULL;
//  }
//
//  variables = get_variables_info_ami_str_from_string(tmp_const);
//
//  return variables;
//
//}
//
//struct ast_json* get_variables_info_json_object(struct ast_json* j_obj, const char* name)
//{
//  const char* tmp_const;
//  struct ast_json* j_res;
//
//  if((j_obj == NULL) || (name == NULL)) {
//    return NULL;
//  }
//
//  tmp_const = ast_json_string_get(ast_json_object_get(j_obj, name));
//  if((tmp_const == NULL) || (strlen(tmp_const) == 0)) {
//    return NULL;
//  }
//
//  j_res = ast_json_load_string(tmp_const, NULL);
//
//  return j_res;
//}
//
//char* get_variables_info_ami_str_from_json_array(struct ast_json* j_arr)
//{
//  int ret;
//  int i;
//  int size;
//  const char* tmp_const;
//  struct ast_json* j_tmp;
//
//  char* variable;
//  char* variables;
//  char* tmp;
//
//  if(j_arr == NULL) {
//    return NULL;
//  }
//
//  ret = ast_json_typeof(j_arr);
//  if(ret != AST_JSON_ARRAY) {
//    return NULL;
//  }
//
//  variables = NULL;
//  variable = NULL;
//  size = ast_json_array_size(j_arr);
//  for(i = 0; i < size; i++) {
//    j_tmp = ast_json_array_get(j_arr, i);
//    if(j_tmp == NULL) {
//      continue;
//    }
//
//    // string only
//    ret = ast_json_typeof(j_tmp);
//    if(ret != AST_JSON_STRING) {
//      continue;
//    }
//
//    tmp_const = ast_json_string_get(j_tmp);
//
//    tmp = get_variables_info_ami_str_from_string(tmp_const);
//    if(tmp == NULL) {
//      continue;
//    }
//
//    ast_asprintf(&variable, "%s\r\n%s",
//        variables?:"",
//        tmp
//        );
//    ast_free(tmp);
//    ast_free(variables);
//    variables = variable;
//  }
//  return variables;
//}
//
//const char* message_get_header(const struct message *m, char *var)
//{
//  int x, l = strlen(var);
//  const char *result = NULL;
//
//  if (!m) {
//    return result;
//  }
//
//  for (x = 0; x < m->hdrcount; x++) {
//    const char *h = m->headers[x];
//    if (!strncasecmp(var, h, l) && h[l] == ':') {
//      const char *value = h + l + 1;
//      value = ast_skip_blanks(value); /* ignore leading spaces in the value */
//      return value;
//    }
//  }
//
//  return result;
//}
