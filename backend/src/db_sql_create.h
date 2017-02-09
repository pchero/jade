/*
 * db_sql_create.h
 *
 *  Created on: Oct 13, 2016
 *      Author: pchero
 */

#ifndef SRC_DB_SQL_CREATE_H_
#define SRC_DB_SQL_CREATE_H_

// queue
static const char* g_sql_queue =
"create table queue("

// identity"
"  uuid      varchar(255)    unique,"         // queue uuid"
"  name      varchar(255)    not null,"       // queue name"
"  detail    text            default null,"
"  in_use    int             default 1,"      // 0:not in use, 1:in use"

"  tm_create           datetime(6),"    // create time."
"  tm_delete           datetime(6)"     // delete time."

");";


// peer
// campaign dial result table."
static const char* g_sql_peer =
"create table peer("

"    channel_type varchar(255)    not null,"   // channel type(SIP, ...)
"    object_name  varchar(255)    not null,"   // name(test-1, test-2, ...)

"    chan_object_type varchar(255),"   // (peer, ...)
"    ip_address       varchar(255),"   // address
"    ip_port          varchar(255),"   // port

"    dynamic          varchar(255),"   // dynamic(yes, ...)
"    auto_force_port  varchar(255),"   //
"    force_port       varchar(255),"   //
"    auto_comedia     varchar(255),"   //
"    comedia          varchar(255),"   //
"    video_support    varchar(255),"   //
"    text_support     varchar(255),"   //

"    acl              varchar(255),"   //
"    status           varchar(255),"   //
"    realtime_device  varchar(255),"   //
"    description      varchar(255),"   //

"    primary key(channel_type, object_name)"

");";

#endif /* SRC_DB_SQL_CREATE_H_ */
