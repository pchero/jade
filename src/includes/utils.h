/*
 * utils.h
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_UTILS_H_
#define BACKEND_SRC_UTILS_H_

#include <time.h>
#include <stdbool.h>
#include <jansson.h>

#define sfree(p) { if(p != NULL) free(p); p=NULL; }

#define DEF_UTILS_MAX_CALLBACKS 256


enum EN_RESOURCE_UPDATE_TYPES {
  EN_RESOURCE_CREATE   = 1,
  EN_RESOURCE_UPDATE   = 2,
  EN_RESOURCE_DELETE   = 3,
};

struct st_callback {
  bool (*callback[DEF_UTILS_MAX_CALLBACKS])(enum EN_RESOURCE_UPDATE_TYPES, const json_t*);
  int count;
};


void utils_trim(char * s);

char* utils_gen_uuid(void);
char* utils_get_utc_timestamp(void);
int   utils_get_utc_timestamp_day(void);
char* utils_get_utc_timestamp_date(void);
char* utils_get_utc_timestamp_time(void);
char* utils_get_utc_timestamp_using_timespec(struct timespec timeptr);
time_t utils_get_unixtime_from_utc_timestamp(const char* timestamp);

char* utils_get_variables_info_ami_str_from_string(const char* str);
char* utils_get_variables_ami_str_from_object(json_t* j_variables);

char* utils_uri_decode(const char* uri);
char* utils_uri_encode(const char* uri);
int utils_convert_time_string(const char* time, const char* format);
char* utils_strip_ext(char *fname);


bool utils_is_string_exist_in_file(const char* filename, const char* str);
bool utils_append_string_to_file_end(const char* filename, const char* str);
bool utils_create_empty_file(const char* filename);

char* utils_string_replace_char(const char* str, const char org, const char target);

bool utils_register_callback(struct st_callback* callback, bool (*func)(enum EN_RESOURCE_UPDATE_TYPES, const json_t*));
struct st_callback* utils_create_callback(void);
void utils_terminiate_callback(struct st_callback* callback);

void utils_execute_callbacks(struct st_callback* callbacks, enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);

#endif /* BACKEND_SRC_UTILS_H_ */
