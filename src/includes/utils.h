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

#define sfree(p) { if(p != NULL) free(p); p=NULL; }

void trim(char * s);

char* gen_uuid(void);
char* get_utc_timestamp(void);
int   get_utc_timestamp_day(void);
char* get_utc_timestamp_date(void);
char* get_utc_timestamp_time(void);
char* get_utc_timestamp_using_timespec(struct timespec timeptr);
time_t get_unixtime_from_utc_timestamp(const char* timestamp);

char* get_variables_info_ami_str_from_string(const char* str);
char* get_variables_ami_str_from_object(json_t* j_variables);

char* uri_decode(const char* uri);
char* uri_encode(const char* uri);
int convert_time_string(const char* time, const char* format);
char* strip_ext(char *fname);


bool is_exist_string_in_file(const char* filename, const char* str);
bool append_string_to_file_end(const char* filename, const char* str);
bool create_empty_file(const char* filename);

char* string_replace_char(const char* str, const char org, const char target);

#endif /* BACKEND_SRC_UTILS_H_ */
