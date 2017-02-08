/*
 * dl_handler.h
 *
 *  Created on: Nov 29, 2015
 *    Author: pchero
 */

#ifndef SRC_DL_HANDLER_H_
#define SRC_DL_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

#include "ob_dialing_handler.h"

/**
 * dl_list.status
 */
typedef enum _E_DL_STATUS_T
{
  E_DL_IDLE     = 0,      //!< idle
  E_DL_DIALING  = 1,    //!< dialing
  E_DL_RESERVED   = 2,  //!< reserved for preview dialing
//  E_DL_QUEUEING   = 2,  ///< not in used.
} E_DL_STATUS_T;

typedef enum _E_DL_USE
{
  E_DL_USE_NO = 0,
  E_DL_USE_OK = 1,
} E_DL_USE;

bool create_dlma(const json_t* j_dlma);
bool update_dlma(const json_t* j_dlma);
bool delete_dlma(const char* uuid);

char* create_dl_list(json_t* j_dl);
bool update_dl_list(json_t* j_dl);
bool delete_dl_list(const char* uuid);

int get_current_dialing_dl_cnt(const char* camp_uuid, const char* dl_table);
int get_dial_num_point(json_t* j_dl_list, json_t* j_plan);
int get_dial_try_cnt(json_t* j_dl_list, int dial_num_point);

json_t* get_dlmas_all(void);
json_t* get_dlma(const char* uuid);
json_t* get_dl_list(const char* uuid);
json_t* get_dl_lists(const char* dlma_uuid, int count);
json_t* get_dl_lists_by_count(int count);
int get_dl_list_cnt_total(json_t* j_dlma);
int get_dl_list_cnt_finshed(json_t* j_dlma, json_t* j_plan);
int get_dl_list_cnt_available(json_t* j_dlma, json_t* j_plan);
int get_dl_list_cnt_dialing(json_t* j_dlma);
int get_dl_list_cnt_tried(json_t* j_dlma);


json_t* get_dl_available_predictive(json_t* j_dlma, json_t* j_plan);
bool is_endable_dl_list(json_t* j_dlma, json_t* j_plan);
void clear_dl_list_dialing(const char* uuid);

json_t* create_dial_info(json_t* j_plan, json_t* j_dl_list, json_t* j_dest);
json_t* create_json_for_dl_result(rb_dialing* dialing);

bool update_dl_list_after_create_dialing_info(rb_dialing* dialing);

#endif /* SRC_DL_HANDLER_H_ */
