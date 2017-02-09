/*
 * db_sql_create.h
 *
 *  Created on: Oct 13, 2016
 *      Author: pchero
 */

#ifndef SRC_DB_SQL_CREATE_H_
#define SRC_DB_SQL_CREATE_H_

// peer
static const char* g_sql_drop_peer =
"drop table if exists peer;";

static const char* g_sql_create_peer =
"create table peer("

"   channel_type varchar(255)    not null,"   // channel type(SIP, ...)
"   object_name  varchar(255)    not null,"   // name(test-1, test-2, ...)

"   chan_object_type varchar(255),"   // (peer, ...)
"   ip_address       varchar(255),"   // address
"   ip_port          varchar(255),"   // port

"   dynamic          varchar(255),"   // dynamic(yes, ...)
"   auto_force_port  varchar(255),"   //
"   force_port       varchar(255),"   //
"   auto_comedia     varchar(255),"   //
"   comedia          varchar(255),"   //
"   video_support    varchar(255),"   //
"   text_support     varchar(255),"   //

"   acl              varchar(255),"   //
"   status           varchar(255),"   //
"   realtime_device  varchar(255),"   //
"   description      varchar(255),"   //

// timestamp. UTC."
"   tm_update        datetime(6),"   // update time."


"   primary key(channel_type, object_name)"

");";


// queue param
static const char* g_sql_drop_queue_param =
"drop table if exists queue_param;";
static const char* g_sql_create_queue_param =
"create table queue_param("

// identity
"   name             varchar(255),"    // queue name.

// status
"   max              int,"             // max available calls in the queue.
"   strategy         varchar(255),"    // queue strategy.
"   calls            int,"             // waiting call count.
"   hold_time        int,"             // average waiting time.
"   talk_time        int,"             // average talk time.
"   completed        int,"             // distributed call count.
"   abandoned        int,"             // could not distributed call count.

// performance
"   service_level       int,"    // service level interval time sec.
"   service_level_perf  real,"   // service level performance(%). completed_call / (abandoned_call + completed_call) * 100

// etc
"   weight          int,"    // queue priority.

// timestamp. UTC."
"   tm_update       datetime(6),"   // update time."

"   primary key(name)"

");";


// queue member
static const char* g_sql_drop_queue_member = "drop table if exists queue_member;";
static const char* g_sql_create_queue_member =
"create table queue_member("

// identity
"   queue_name     varchar(255),"   // queue name
"   name           varchar(255),"   // name

"   location          varchar(255),"          // location
"   state_interface   varchar(255),"          // state interface
"   membership        varchar(255),"  // membership
"   penalty           int,"           // penalty
"   calls_taken       int,"           // call taken count.
"   last_call         int,"
"   last_pause        int,"
"   in_call           int,"
"   status            int,"
"   paused            int,"
"   paused_reason     text,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(queue_name, name)"

");";


// queue entry
static const char* g_sql_drop_queue_entry = "drop table if exists queue_entry;";
static const char* g_sql_create_queue_entry =
"create table queue_entry("

// identity
"   queue_name        varchar(255),"
"   position          int,"

// info
"   channel           varchar(255),"
"   unique_id         varchar(255),"
"   caller_id_num     varchar(255),"
"   caller_id_name    varchar(255),"
"   connected_line_num  varchar(255),"
"   connected_line_name varchar(255),"
"   wait    int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(queue_name, position)"

");";

#endif /* SRC_DB_SQL_CREATE_H_ */
