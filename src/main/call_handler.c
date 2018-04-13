/*
 * call_handler.c
 *
 *  Created on: Apr 13, 2018
 *      Author: pchero
 */


#include <stdbool.h>


#include "slog.h"

#include "call_handler.h"


static bool init_databases(void);
static bool init_database_channel(void);


#define DEF_DB_TABLE_CALL_CHANNEL "channel"


bool call_init_handler(void)
{
  int ret;

  ret = init_databases();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database.");
    return false;
  }

  return true;
}

bool call_term_handler(void)
{
  return true;
}

bool call_reload_handler(void)
{
  int ret;

  ret = call_term_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not terminate call_handler.");
    return false;
  }

  ret = call_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate call_handler.");
    return false;
  }

  return true;
}

static bool init_databases(void)
{
  int ret;

  slog(LOG_DEBUG, "Fired init_databases.");

  ret = init_database_channel();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate channel database.");
    return false;
  }

  return true;
}

static bool init_database_channel(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_CALL_CHANNEL ";";
  create_table =
      "create table " DEF_DB_TABLE_CALL_CHANNEL "("

      // identity
      "   unique_id   varchar(255) not null,"
      "   linked_id   varchar(255),"

      // channel info
      "   channel             varchar(255),"    ///< channel name
      "   channel_state       int,"
      "   channel_state_desc  varchar(255),"

      // dial info
      "   caller_id_num       varchar(255),"
      "   caller_id_name      varchar(255),"

      "   connected_line_num  varchar(255),"
      "   connected_line_name varchar(255),"

      "   language            varchar(255),"
      "   account_code        varchar(255),"

      // dialplan
      "   context     varchar(255),"
      "   exten       varchar(255),"
      "   priority    varchar(255),"

      "   application       varchar(255),"
      "   application_data  varchar(1023),"
      "   bridge_id         varchar(255),"

      // hangup
      "   hangup_cause        int,"
      "   hangup_cause_desc   varchar(255),"

      // variables
      "   variables   text,"

      // other
      "   duration  int,"

      // timestamp. UTC."
      "   tm_update     datetime(6),"

      "   primary key(unique_id)"
      ");";


  // execute
  ret = resource_exec_mem_sql(drop_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_CALL_CHANNEL);
    return false;
  }

  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_CALL_CHANNEL);
    return false;
  }

  return true;
}
