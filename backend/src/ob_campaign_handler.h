/*
 * campaign_handler.h
 *
 *  Created on: Nov 26, 2015
 *    Author: pchero
 */

#ifndef SRC_CAMPAIGN_HANDLER_H_
#define SRC_CAMPAIGN_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

typedef enum _E_CAMP_STATUS_T
{
  // static status
  E_CAMP_STOP   = 0,
  E_CAMP_START  = 1,
  E_CAMP_PAUSE  = 2,
//  E_CAMP_SCHEDULE_STOP = 3,

  // on going status
  E_CAMP_STOPPING = 10,
  E_CAMP_STARTING = 11,
  E_CAMP_PAUSING  = 12,
//  E_CAMP_SCHEDULE_STOPPING  = 13,

  // force status
  E_CAMP_STOPPING_FORCE = 30,
} E_CAMP_STATUS_T;

typedef enum _E_CAMP_SCHEDULE_MODE
{
  E_CAMP_SCHEDULE_OFF = 0,
  E_CAMP_SCHEDULE_ON  = 1,
} E_CAMP_SCHEDULE_MODE;

json_t* create_ob_campaign(json_t* j_camp);
json_t* delete_ob_campaign(const char* uuid);
json_t* update_ob_campaign(const json_t* j_camp);
bool update_ob_campaign_status(const char* uuid, E_CAMP_STATUS_T status);

json_t* get_ob_campaigns_all(void);
json_t* get_ob_campaigns_all_uuid(void);
json_t* get_campaigns_schedule_start(void);
json_t* get_campaigns_schedule_end(void);
json_t* get_ob_campaign(const char* uuid);
json_t* get_ob_campaigns_by_status(E_CAMP_STATUS_T status);
json_t* get_ob_campaign_for_dialing(void);
json_t* get_ob_campaign_stat(const char* uuid);
json_t* get_campaigns_stat_all(void);

bool is_referenced_destination_by_campaign(const char* uuid_dest);
bool is_referenced_dlma_by_campaign(const char* uuid_dlma);
bool is_referenced_plan_by_campaign(const char* uuid_plan);

bool clear_campaign_destination(const char* uuid_dest);
bool clear_campaign_dlma(const char* uuid_dlma);
bool clear_campaign_plan(const char* uuid_plan);

bool is_stoppable_campgain(json_t* j_camp);
bool is_startable_campgain(json_t* j_camp);
bool is_exist_ob_campaign(const char* uuid);
bool validate_ob_campaign(json_t* j_data);

#endif /* SRC_CAMPAIGN_HANDLER_H_ */
