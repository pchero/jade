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
#include <jansson.h>
#include <errno.h>

#include "utils.h"
#include "slog.h"

#define DEF_UUID_STR_LEN 37

#define IS_ALNUM(ch) \
    ( ch >= 'a' && ch <= 'z' ) || \
    ( ch >= 'A' && ch <= 'Z' ) || \
    ( ch >= '0' && ch <= '9' ) || \
    ( ch >= '-' && ch <= '.' )


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

time_t get_unixtime_from_utc_timestamp(const char* timestamp)
{
  int ret;
  struct tm ti={0};

  if(timestamp == NULL) {
    // wrong input parameter.
    return 0;
  }

  ret = sscanf(timestamp, "%d-%d-%dT%d:%d:%d",
    &ti.tm_year,
    &ti.tm_mon,
    &ti.tm_mday,
    &ti.tm_hour,
    &ti.tm_min,
    &ti.tm_sec
    );
  if(ret != 6) {
    // could not convert correctly.
    return 0;
  }

  ti.tm_year  -= 1900;
  ti.tm_mon   -= 1;

  return mktime(&ti);
}

/**
 *
 * @param str
 * @return
 */
char* get_variables_info_ami_str_from_string(const char* str)
{
  const char* key;
  json_t* j_tmp;
  json_t* j_val;
  char* res_sub;
  char* res;
  char* tmp;

  if((str == NULL) || (strlen(str) == 0)) {
    return NULL;
  }

  j_tmp = json_loads(str, JSON_DECODE_ANY, NULL);

  json_object_foreach(j_tmp, key, j_val) {
    asprintf(&res_sub, "%s=%s\r\n",
        key,
        json_string_value(j_val)? : ""
        );

    asprintf(&tmp, "%s%s", res, res_sub);
    sfree(res_sub);
    sfree(res);
    res = tmp;
  }

  return res;
}

/**
 *
 * @param str
 * @return
 */
char* get_variables_ami_str_from_object(json_t* j_variables)
{
  const char* key;
  json_t* j_val;
  char* res_sub;
  char* res;
  char* tmp;

  if(j_variables == NULL) {
    return NULL;
  }

  res = NULL;
  json_object_foreach(j_variables, key, j_val) {
    asprintf(&res_sub, "Variable: %s=%s\r\n",
        key,
        json_string_value(j_val)? : ""
        );
    if(res == NULL) {
      asprintf(&tmp, "%s", res_sub);
    }
    else {
      asprintf(&tmp, "%s%s", res, res_sub);
    }

    sfree(res_sub);
    sfree(res);
    res = tmp;
  }

  return res;
}

/**
 * Decode uri.
 * @param str
 * @return
 */
char* uri_decode(const char* str)
{
  int i, j = 0, len;
  char* tmp;
  char hex[3];

  if(str == NULL) {
    return NULL;
  }

  len = strlen( str );
  hex[2] = 0;

  tmp = (char*)malloc(sizeof(char) * (len + 1));

  for(i = 0; i < len; i++, j++) {
    if(str[i] != '%') {
      tmp[j] = str[i];
    }
    else {

      if((IS_ALNUM(str[i + 1]) == 1) && (IS_ALNUM(str[i + 2]) == 1) && (i < (len - 2))) {
        hex[0] = str[i+1];
        hex[1] = str[i+2];
        tmp[j] = strtol( hex, NULL, 16 );

        i += 2;
      }
      else {
        tmp[j] = '%';
      }
    }
  }
  tmp[j] = 0;

  return tmp;
}

/**
 * Encode string.
 * @param str
 * @return
 */
char* uri_encode(const char* str)
{
  int i;
  int j;
  int len;
  char* tmp;

  if(str == NULL) {
    return NULL;
  }

  len = strlen( str );
  tmp = (char*) malloc( (sizeof(char) * 3 * len) +1 );

  j = 0;
  for(i = 0; i < len; i++ ){
    if(IS_ALNUM(str[i]) == 1) {
      tmp[j] = str[i];
    }
    else {
      snprintf(&tmp[j], 4, "%%%02X\n", (unsigned char)str[i]);
      j += 2;
    }
    j++;
  }
  tmp[j] = 0;

  return tmp;
}


/**
 *
 * @param time
 * @param format
 * @return
 */
int convert_time_string(const char* time, const char* format)
{
  struct tm tm;
  int sec;

  if((time == NULL) || (format == NULL)) {
    return -1;
  }

  memset(&tm, 0x00, sizeof(tm));
  strptime(time, format, &tm);

  sec = 0;
  sec += tm.tm_sec;
  sec += tm.tm_min * 60;
  sec += tm.tm_hour * 60 * 60;

  return sec;
}

/**
 @brief strip extension
 */
char* strip_ext(char *fname)
{
  char* filename;
  char* end;

  if(fname == NULL) {
    return NULL;
  }

  filename = strdup(fname);
  end = filename + strlen(filename);

  while ((end > filename) && (*end != '.') && (*end != '\\') && (*end != '/')) {
    --end;
  }

  if ((end > filename) && (*end == '.')) {
    *end = '\0';
  }

  return filename;
}

/**
 * Return true if given string is exist in the given file.
 * @param filename
 * @param str
 * @return
 */
bool is_exist_string_in_file(const char* filename, const char* str)
{
  FILE* fp;
  char* tmp;
  char tmp_str[1024];
  bool find;

  if((filename == NULL) || (str == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  fp = fopen(filename, "r");
  if(fp == NULL) {
    slog(LOG_ERR, "Could not open file. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return false;
  }

  find = false;
  while(1) {
    tmp = fgets(tmp_str, sizeof(tmp_str), fp);
    if(tmp == NULL) {
      break;
    }

    tmp = strstr(tmp_str, str);
    if(tmp != NULL) {
      find = true;
      break;
    }
  }
  fclose(fp);

  if(find == false) {
    return false;
  }

  return true;
}

/**
 * Append given string with new line at the EOF.
 * @param filename
 * @param str
 * @return
 */
bool append_string_to_file_end(const char* filename, const char* str)
{
  FILE* fp;
  int ret;
  char* tmp;

  if((filename == NULL) || (str == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  fp = fopen(filename, "a");
  if(fp == NULL) {
    slog(LOG_ERR, "Could not open file. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return false;
  }

  asprintf(&tmp, "%s\n", str);

  ret = fputs(tmp, fp);
  sfree(tmp);
  fclose(fp);
  if(ret < 0) {
    slog(LOG_ERR, "Could not write string. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return false;
  }

  return true;
}

bool create_empty_file(const char* filename)
{
  FILE* fp;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  fp = fopen(filename, "a");
  if(fp == NULL) {
    slog(LOG_ERR, "Could not create empty file. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return false;
  }

  fclose(fp);

  return true;
}

/**
 * Copy the given str and replace given org character to target character from str.
 * Return string should be freed after use it.
 * @param uuid
 * @return
 */
char* string_replace_char(const char* str, const char org, const char target)
{
  char* tmp;
  int len;
  int i;
  int j;

  if(str == NULL) {
    return NULL;
  }

  len = strlen(str);
  tmp = calloc(len + 1, sizeof(char));
  j = 0;
  for(i = 0; i < len; i++) {
    if(str[i] == org) {
      tmp[j] = target;
    }
    else {
      tmp[j] = str[i];
    }
    j++;
  }
  tmp[j] = '\0';
  return tmp;
}
