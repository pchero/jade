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

#include "utils.h"

#define DEF_UUID_STR_LEN 37


static int uri_decode(const char *s, char *dec);
static int ishex(int x);



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

static int ishex(int x)
{
  return  (x >= '0' && x <= '9')  ||
    (x >= 'a' && x <= 'f')  ||
    (x >= 'A' && x <= 'F');
}

static int uri_decode(const char *s, char *dec)
{
  char *o;
  const char *end;
  int c;

  end = s + strlen(s);
  for (o = dec; s <= end; o++) {
    c = *s++;
    if (c == '+') {
      c = ' ';
    }
    else if ((c == '%') && ( !ishex(*s++) || !ishex(*s++) || !sscanf(s - 2, "%2x", &c))) {
      return -1;
    }

    if (dec) {
      *o = c;
    }
  }

  return o - dec;
}

char* uri_parse(const char* uri)
{
  char tmp[2048];
  char* res;
  int ret;

  if(uri == NULL) {
    return NULL;
  }

  ret = uri_decode(uri, tmp);
  if(ret < 0) {
    return NULL;
  }

  res = strdup(tmp);

  return res;
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
