/*
 * utils.h
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_UTILS_H_
#define BACKEND_SRC_UTILS_H_

#define sfree(p) { if(p != NULL) free(p); p=NULL; }

void trim(char * s);

char* gen_uuid(void);
char* get_utc_timestamp(void);
int   get_utc_timestamp_day(void);
char* get_utc_timestamp_date(void);
char* get_utc_timestamp_time(void);
char* get_utc_timestamp_using_timespec(struct timespec timeptr);
char* get_variables_info_ami_str_from_string(const char* str);
//char* get_variables_info_ami_str(struct ast_json* j_obj, const char* name);
//struct ast_json* get_variables_info_json_object(struct ast_json* j_obj, const char* name);
//char* get_variables_info_ami_str_from_json_array(struct ast_json* j_arr);
//char* get_variables_info_ami_str_from_string(const char* str);
//const char* message_get_header(const struct message *m, char *var);


#endif /* BACKEND_SRC_UTILS_H_ */
