/*
 * db_sql_create.h
 *
 *  Created on: Oct 13, 2016
 *      Author: pchero
 */

#ifndef SRC_DB_SQL_CREATE_H_
#define SRC_DB_SQL_CREATE_H_

// plan
static const char* g_db_sql_plan =
"create table plan( "

// identity
"    uuid        varchar(255) unique not null,"
"    name        varchar(255)    default null,"			// plan name"
"    detail      varchar(1023)   default null," 		// description
"    in_use      int default 1,"										// 0:not in use, 1:in use"

// resource
"    variables   text,"															// string of json object. "{"var1":"value1", "var2":"value2", ... }"

// strategy"
"    dial_mode       int default 1,"              	// dial mode(desktop, power, predictive, email, fax, sms)"
"    dial_timeout    int default 30000,"          	// no answer hangup timeout(30000 ms = 30 second)"
"    caller_id       varchar(255) default null,"		// caller name(from)"
"    dl_end_handle   int default 1,"              	// stratery when it running out dial list(keep_running, stop)"
"    retry_delay     int default 60,"          			// retry delaytime(sec)"
"    trunk_name      varchar(255) default null,"  	// trunk name"
"    tech_name       varchar(255) default null,"  		// tech name"
"    service_level   int unsigned default 0,"     // service level. determine how many calls can going out campare to available agents."
"    early_media     varchar(255) default null,"
"    codecs          varchar(255) default null,"

// retry count"
"    max_retry_cnt_1     int default 5,"  // max retry count for dial number 1"
"    max_retry_cnt_2     int default 5,"  // max retry count for dial number 2"
"    max_retry_cnt_3     int default 5,"  // max retry count for dial number 3"
"    max_retry_cnt_4     int default 5,"  // max retry count for dial number 4"
"    max_retry_cnt_5     int default 5,"  // max retry count for dial number 5"
"    max_retry_cnt_6     int default 5,"  // max retry count for dial number 6"
"    max_retry_cnt_7     int default 5,"  // max retry count for dial number 7"
"    max_retry_cnt_8     int default 5,"  // max retry count for dial number 8"

// timestamp. UTC."
"    tm_create           datetime(6),"    // create time."
"    tm_delete           datetime(6),"    // delete time."
"    tm_update           datetime(6),"    // last update time."

"    primary key(uuid)"
");";

// queue
static const char* g_db_sql_queue =
"create table queue("

// identity"
"  uuid      varchar(255)    unique,"         // queue uuid"
"  name      varchar(255)    not null,"       // queue name"
"  detail    text            default null,"
"  in_use    int             default 1,"      // 0:not in use, 1:in use"

"  tm_create           datetime(6),"    // create time."
"  tm_delete           datetime(6)"     // delete time."

");";

// dial_list_original
// original dial list info table"
// all of other dial lists are copy of this table."
static const char* g_db_sql_dial_list =
"create table dl_list("

// identity"
"    uuid        varchar(255)    unique,"     // dl uuid"
"    dlma_uuid   varchar(255)    not null,"   // dl_list_ma uuid"
"    in_use      int default 1,"              // 0:not in use, 1:in use"

// information"
"    name            varchar(255),"   // customer name"
"    detail          varchar(255),"   // customer detail info"
"    status          int default 0,"  // dial list status. (0:idle, 1:dialing, 2:reserved)"

// desktop dialing"
"    resv_target     varchar(255),"   // reserved target address"

// custom define data"
"    ukey            text,"   // user define key"
"    variables       text,"		// string of json object. "{"var1":"value1", "var2":"value2", ... }"


// current dialing"
"    dialing_uuid            varchar(255),"       // dialing channel unique id."
"    dialing_camp_uuid       varchar(255),"       // dialing campaign uuid."
"    dialing_plan_uuid       varchar(255),"       // dialing plan uuid."

// numbers."
"    number_1    varchar(255),"       // tel number 1"
"    number_2    varchar(255),"       // tel number 2"
"    number_3    varchar(255),"       // tel number 3"
"    number_4    varchar(255),"       // tel number 4"
"    number_5    varchar(255),"       // tel number 5"
"    number_6    varchar(255),"       // tel number 6"
"    number_7    varchar(255),"       // tel number 7"
"    number_8    varchar(255),"       // tel number 8"

// other address."
"    email       text,"

// try counts."
"    trycnt_1    int default 0,"      // try count for tel number 1"
"    trycnt_2    int default 0,"      // try count for tel number 2"
"    trycnt_3    int default 0,"      // try count for tel number 3"
"    trycnt_4    int default 0,"      // try count for tel number 4"
"    trycnt_5    int default 0,"      // try count for tel number 5"
"    trycnt_6    int default 0,"      // try count for tel number 6"
"    trycnt_7    int default 0,"      // try count for tel number 7"
"    trycnt_8    int default 0,"      // try count for tel number 8"

// result info"
"    res_dial            int default 0 not null,"   // last dial result.(no answer, answer, busy, ...)"
"    res_dial_detail     text,"
"    res_hangup          int default 0 not null,"   // last route result after answer.(routed, agent busy, no route place, ...)"
"    res_hangup_detail   text,"

// timestamp. UTC."
"    tm_create       datetime(6),"   // create time"
"    tm_delete       datetime(6),"   // delete time"
"    tm_update       datetime(6),"   // last update time"
"    tm_last_dial    datetime(6),"   // last dial time"
"    tm_last_hangup  datetime(6),"   // last hangup time"

"    primary key(uuid)"
");";

// dl_list_ma
// dial list"
// manage all of dial list tables"
static const char* g_db_sql_dl_list_ma =
"create table dl_list_ma("

// row identity"
"    uuid        varchar(255)    unique not null,"    // dial_list_#### reference uuid."

// information"
"    name        varchar(255),"                       // dial list name"
"    detail      text,"                               // description of dialist"
"    dl_table    varchar(255),"                       // dial list table name.(view)"
"    variables   text,"		                            // string of json object. "{"var1":"value1", "var2":"value2", ... }"

"    in_use      int default 1,"

// timestamp. UTC."
"    tm_create           datetime(6),"    // create time."
"    tm_delete           datetime(6),"    // delete time."
"    tm_update           datetime(6),"    // last update time."

"    primary key(uuid)"
");";



// destination
static const char* g_db_sql_destination =
"create table destination("

// identity
" uuid          varchar(255)    unique,"
" name          varchar(255),"
" detail        varchar(1023),"
" in_use        int     default 1,"

// information
" type          int default 0,"         // 0:extension, 1:application

// extension
" exten         varchar(255),"
" context       varchar(255),"
" priority      varchar(255),"
" variables     text,"   								// string of json object. "{"var1":"value1", "var2":"value2", ... }"


// application
" application   varchar(255),"  // application name
" data          text,"  // application arguments

// timestamp. UTC
" tm_create     datetime(6),"
" tm_delete     datetime(6),"
" tm_update     datetime(6)"
");";


// campaign
static const char* g_db_sql_campaign =
"create table campaign("

// identity"
"    uuid        varchar(255)    unique,"

// information"
"    detail            varchar(1023),"                 // description"
"    name              varchar(255),"                  // campaign name"
"    status            int              default 0,"    // status code(stop(0), start(1), pause(2), stopping(10), starting(11), pausing(12)"
"    in_use            int              default 1,"    // in use:1, not in use:0"

"    next_campaign     varchar(255),"                  // next campaign uuid"

"    plan    varchar(255),"                       // plan uuid"
"    dlma    varchar(255),"                       // dial_list_ma uuid"
"    dest    varchar(255),"                       // destination uuid

// schedule
"    sc_mode              int      default 0,"	// scheduling mode. 0:off, 1:on
"    sc_date_start        date,"		// start date(YYYY-MM-DD)
"    sc_date_end          date,"		// end date(YYYY-MM-DD)
"    sc_date_list         text,"		// "YYYY-MM-DD, YYYY-MM-DD, ..."
"    sc_date_list_except  text,"		// "YYYY-MM-DD, YYYY-MM-DD, ..."
"    sc_time_start        time,"		// "HH:MM:SS"
"    sc_time_end          time,"		// "HH:MM:SS"
"    sc_day_list          text,"		// "0, 1, ..., 6" 0=Sunday, 1=Monday, ... 6=Saturday

// timestamp. UTC."
"    tm_create           datetime(6),"   // create time."
"    tm_delete           datetime(6),"   // delete time."
"    tm_update           datetime(6),"   // update time."

"    foreign key(plan)   references plan(uuid)       on delete set null on update cascade,"
"    foreign key(dlma)   references dl_list_ma(uuid) on delete set null on update cascade,"

"    primary key(uuid)"
");";

// dl_result
// campaign dial result table."
static const char* g_sql_dl_result =
"create table dl_result("

// identity
"    dialing_uuid        varchar(255)    not null,"   // dialing uuid(channel unique id)."
"    camp_uuid           varchar(255)    not null,"   // campaign uuid."
"    plan_uuid           varchar(255)    not null,"   // plan uuid."
"    dlma_uuid           varchar(255)    not null,"   // dial_list_ma uuid."
"    dl_list_uuid        varchar(255)    not null,"   // dl_list uuid"

// dial_info"
"    info_camp   text    not null,"   // campaign info. json format."
"    info_plan   text    not null,"   // plan info. json format."
"    info_dlma	 text    not null,"   // dlma info. json format."
"    info_dl     text    not null,"   // dl info. json format."
"    info_chan   text    not null,"   // channel info. json format."
"    info_queues text    not null,"   // queue info. json format"
"    info_agents text    not null,"   // agents info. json format"

// timestamp(UTC)"
"    tm_dial_begin       datetime(6),"   // timestamp for dialing requested."
"    tm_dial_end         datetime(6),"   // timestamp for dialing ended."

"    tm_pickup           datetime(6),"   // timestamp for call tried."
"    tm_redirect         datetime(6),"   // timestamp for call redirected."
"    tm_bridge           datetime(6),"   // timestamp for call bridged."
"    tm_hangup           datetime(6),"   // timestamp for call hungup."

"    tm_tr_dial_begin    datetime(6),"   // timestamp for dialing to agent."
"    tm_tr_dial_end      datetime(6),"   // timestamp for transfer to agent."
"    tm_tr_hangup        datetime(6),"   // timestamp for agent hangup."

// dial info"
"    dial_index          int,"            // dialing number index."
"    dial_addr           varchar(255),"   // dialing address(number)."
"    dial_channel        varchar(255),"   // dialing channel"
"    dial_trycnt         int,"            // dialing try count."
"    dial_timeout        int,"            // dialing timeout."
"    dial_type           int,"            // dialing type."
"    dial_exten          varchar(255),"
"    dial_context        varchar(255),"
"    dial_application    varchar(255),"
"    dial_data           varchar(255),"

// channel info"
"    channel_name        varchar(255),"   // channel name"

// transfer info"
"    tr_trycnt          int,"             // transfer try count."
"    tr_agent_uuid      varchar(255),"    // transfered agent."
"    tr_chan_unique_id  varchar(255),"    // trying transfer chan unique id."

// dial result"
"    res_dial                int default 0 not null,"     // dial result(answer, no_answer, ...)"
"    res_hangup              int default 0 not null,"     // hangup code."
"    res_hangup_detail       varchar(255),"               // hangup detail."
"    res_tr_dial             varchar(255),"               // transferred dial result(answer, no_answer, ...)"
"    res_tr_hangup           varchar(255),"               // hangup code."
"    res_tr_hangup_detail    varchar(255),"               // hangup detail."

"    primary key(dialing_uuid)"

");";


#endif /* SRC_DB_SQL_CREATE_H_ */
