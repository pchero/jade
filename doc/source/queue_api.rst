.. _queue_api:


.. _queue_entries:

/queue/entries
==============

Methods
-------
GET : Get list of all queue entries info.

.. _get_queue_entries:

Method: GET
-----------
Get list of all queue entries info.

Call
++++
::

   GET /queue/entries

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           "unique_id": "<string>",
           "queue_name": "<string>",
           "channel": "<string>",
    
           "caller_id_name": "<string>",
           "caller_id_num": "<string>",
           "connected_line_name": "<string>",
           "connected_line_num": "<string>",
    
           "position": <number>,
           "wait": <number>,
           
           "tm_update": "<timestamp>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of queue entries.
  * See detail at :ref:`get_queue_entries_detail`.

Example
+++++++
::

  $ curl -X GET localhost:8081/queue/entries
   
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "caller_id_name": "pjagent-01",
                "caller_id_num": "pjagent-01",
                "channel": "PJSIP/pjagent-01-00000002",
                "connected_line_name": "<unknown>",
                "connected_line_num": "<unknown>",
                "position": 1,
                "queue_name": "sales_1",
                "tm_update": "2017-12-18T00:23:39.821137155Z",
                "unique_id": "1513556618.4",
                "wait": null
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T00:23:40.94974824Z"
  }
   
.. _queue_entries_detail:   

/queue/entries/<detail>
=======================

Methods
-------
GET : Get queue entry detail info of given queue entry info.

.. _get_queue_entries_detail:

Method: GET
-----------
Get queue entry detail info of given queue entry info.

Call
++++
::

   GET /queue/entries/<detail>

Method parameters

* ``detail``: unique id

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "unique_id": "<string>",
       "queue_name": "<string>",
       "channel": "<string>",

       "caller_id_name": "<string>",
       "caller_id_num": "<string>",
       "connected_line_name": "<string>",
       "connected_line_num": "<string>",

       "position": <number>,
       "wait": <number>,
       
       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``unique_id``: Unique id of channel.
* ``caller_id_num``: The name of the queue.
* ``channel``: Channel name.

* ``caller_id_name``: Caller's name.
* ``caller_id_num``: Caller's number.
* ``connected_line_name``: Connected line's name.
* ``connected_line_num``: Connected line's number.

* ``position``: Position in the queue.
* ``wait``: If set when paused, the reason the queue member was paused.

Example
+++++++
::

  $ curl -X GET localhost:8081/queue/entries/1513557067.6
   
  {
    "api_ver": "0.1",
    "result": {
        "caller_id_name": "pjagent-01",
        "caller_id_num": "pjagent-01",
        "channel": "PJSIP/pjagent-01-00000004",
        "connected_line_name": "<unknown>",
        "connected_line_num": "<unknown>",
        "position": 1,
        "queue_name": "sales_1",
        "tm_update": "2017-12-18T00:31:08.754950500Z",
        "unique_id": "1513557067.6",
        "wait": null
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T00:31:18.894580134Z"
  }

  
/queue/members
==============

Methods
-------
GET : Get list of all queue members info.

.. _get_queue_members:

Method: GET
-----------
Get list of all queue members info.

Call
++++
::

   GET /queue/members

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
            "name": "<string>",
            "queue_name": "<string>",
            "status": <integer>,

            "membership": "<string>",
            "state_interface": "<string>",
            "location": "<string>",

            "paused": <integer>,
            "paused_reason": "<string>",
            "penalty": <integer>,

            "calls_taken": <integer>,
            "in_call": <integer>,

            "last_call": <integer>,
            "last_pause": <integer>,

            "ring_inuse": <integer>,

            "tm_update": "<timestamp>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of registry account.
  * See detail at :ref:`get_queue_members_detail`.

Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/queue_params
   
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "calls_taken": 0,
                "in_call": 0,
                "last_call": 0,
                "last_pause": 0,
                "location": "sip/agent-02",
                "membership": "dynamic",
                "name": "sip/agent-02",
                "paused": 0,
                "paused_reason": "",
                "penalty": 0,
                "queue_name": "sales_1",
                "ring_inuse": null,
                "state_interface": "sip/agent-02",
                "status": 4,
                "tm_update": "2017-12-18T00:31:04.175880809Z"
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T00:34:45.370734689Z"
  }


/queue/members/<detail>
=======================

Methods
-------
GET : Get queue member detail info of given queue member info.

.. _get_queue_members_detail:

Method: GET
-----------
Get queue member detail info of given queue member info.

Call
++++
::

   GET /queue/members/<detail>?queue_name=<string>

Method parameters

* ``detail``: member name.
* ``queue_name``: queue name.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "name": "<string>",
       "queue_name": "<string>",
       "status": <integer>,

       "membership": "<string>",
       "state_interface": "<string>",
       "location": "<string>",

       "paused": <integer>,
       "paused_reason": "<string>",
       "penalty": <integer>,

       "calls_taken": <integer>,
       "in_call": <integer>,

       "last_call": <integer>,
       "last_pause": <integer>,

       "ring_inuse": <integer>,

       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``name``: The name of the queue member.
* ``queue_name``: The name of the queue.
* ``status``: The numeric device state status of the queue member.

* ``membership``: Membership of queue member.
* ``state_interface``: Channel technology or location from which to read device state changes.
* ``location``: The queue member's channel technology or location.

* ``paused``: Paused.
* ``paused_reason``: If set when paused, the reason the queue member was paused.
* ``penalty``: The penalty associated with the queue member.

* ``calls_taken``: The number of calls this queue member has serviced.
* ``in_call``: Set to 1 if member is in call. Set to 0 after LastCall time is updated.

* ``last_call``: The time this member last took a call, expressed in seconds since 00:00, Jan 1, 1970 UTC.
* ``last_pause``: The time when started last paused the queue member.

* ``ring_inuse``: Ring in use option.

Example
+++++++
::

  $ curl -X GET localhost:8081/queue/members/Agent%2F10001\?queue_name=sales_1
   
  {
    "api_ver": "0.1",
    "result": {
        "calls_taken": 0,
        "in_call": 0,
        "last_call": 0,
        "last_pause": 0,
        "location": "Agent/10001",
        "membership": "static",
        "name": "Agent/10001",
        "paused": 0,
        "paused_reason": "",
        "penalty": 0,
        "queue_name": "sales_1",
        "ring_inuse": null,
        "state_interface": "Agent/10001",
        "status": 4,
        "tm_update": "2017-12-18T00:31:04.234368754Z"
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T00:38:27.704665757Z"
  }

  
.. _queue_queues:
  
/queue/queues
=============

Methods
-------
GET : Get list of all queues info.

POST : Create new queue info.

.. _get_queue_queues:

Method: GET
-----------
Get list of all queues info

Call
++++
::

   GET /queue/queues

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
            "name": "<string>",
            "strategy": "<string>",
            "max": <integer>,
            "weight": <integer>,

            "calls": <integer>,
            "completed": <integer>,
            "abandoned": <integer>,

            "hold_time": <integer>,
            "talk_time": <integer>,
            
            "service_level": <integer>,
            "service_level_perf": <integer>,

            "tm_update": "<timestamp>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of registry account.
  * See detail at :ref:`get_queue_queues_detail`.

Example
+++++++
::

  $ curl -X GET localhost:8081/queue/queues
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "abandoned": 2,
                "calls": 0,
                "completed": 0,
                "hold_time": 0,
                "max": 0,
                "name": "sales_1",
                "service_level": 5,
                "service_level_perf": 0.0,
                "strategy": "ringall",
                "talk_time": 0,
                "tm_update": "2017-12-18T00:31:04.142068111Z",
                "weight": 0
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T00:46:25.124236613Z"
  }


.. _post_queue_queues:

Method: POST
------------
Create new queue info.

Call
++++
::

  POST /queue/queues

  {
    ...
  }
  
Data parameters

* See detail at Asterisk's queue config.
   
Returns
+++++++
::

  {
    $defhdr
  }
  

Example
+++++++
::

  $ curl -X POST localhost:8081/queue/queues \
    -d '{"name": "test create queue", "context": "default"}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-21T02:02:49.663362846Z"
  } 
  

.. _queue_queues_detail:
  
/queue/queues/<detail>
======================

Methods
-------
GET : Get queue detail info of given queue info.

PUT : Update queue detail info of given queue info.

DELETE : Delete the given queue.

.. _get_queue_queues_detail:

Method: GET
-----------
Get queue detail info of given queue info.

Call
++++
::

  GET /queue/queues/<detail>

Method parameters

* ``detail``: queue name.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "name": "<string>",
       "strategy": "<string>",
       "max": <integer>,
       "weight": <integer>,

       "calls": <integer>,
       "completed": <integer>,
       "abandoned": <integer>,

       "hold_time": <integer>,
       "talk_time": <integer>,
       
       "service_level": <integer>,
       "service_level_perf": <integer>,

       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``name``: Queue name.
* ``strategy``: Call distribution.
* ``max``: Max waiting call count.
* ``weight``: Queue priority.

* ``calls``: Waiting call count.
* ``completed``: Completed call count.
* ``abandoned``: Abandoned call count.

* ``hold_time``: Average waiting time.
* ``talk_time``: Average talk time.

* ``service_level``: Service_level_perf interval time.
* ``service_leve_perf``: Service level performance.

Example
+++++++
::

  $ curl -X GET localhost:8081/queue/queues/sales_1
  
  {
    "api_ver": "0.1",
    "result": {
        "abandoned": 2,
        "calls": 0,
        "completed": 0,
        "hold_time": 0,
        "max": 0,
        "name": "sales_1",
        "service_level": 5,
        "service_level_perf": 0.0,
        "strategy": "ringall",
        "talk_time": 0,
        "tm_update": "2017-12-18T00:31:04.142068111Z",
        "weight": 0
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T00:43:30.189014882Z"
  }

.. _put_queue_queues_detail:

Method: PUT
-----------
Update queue detail info of given queue info.

Call
++++
::

  PUT /queue/queues/<detail>
  
  {
    ...
  }

Method parameters

* ``detail``: uri encoded queue name.

Data parameters

* See detail at Asterisk's queue setting.

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -X PUT localhost:8081/queue/queues/test%20create%20queue 
    -d '{"context": "default", "member":["> Agent/10001", "> Agent/10003"]}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-21T02:56:19.975276515Z"
  }

  
.. _delete_queue_queues_detail:

Method: DELETE
-----------
Delete the given queue.

Call
++++
::

  DELETE /queue/queues/<detail>  

Method parameters

* ``detail``: uri encoded queue name.

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -X DELETE localhost:8081/queue/queues/test%20create%20queue
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-21T02:58:36.537005271Z"
  }

  
  
.. _queue_statuses:
  
/queue/statuses
===============

Methods
-------
GET : Get list of all queues status.

.. _get_queue_statuses:

Method: GET
-----------
Get list of all queues status.

Call
++++
::

   GET /queue/statuses

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        {
          "queue": {...},
          "entries": [{...}, ...],
          "members": [{...}, ...]
        },
        ...
      ]
    }
  }
  
* ``list`` : array of info.
    * ``queue``: See detail at queue info.
    * ``entries``: See detail at queue entries.
    * ``members``: See detail at queue members.

Example
+++++++
::

  $ curl -X GET localhost:8081/queue/statuses
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "entries": [
                    {
                        "caller_id_name": "pjagent-01",
                        "caller_id_num": "pjagent-01",
                        "channel": "PJSIP/pjagent-01-00000000",
                        "connected_line_name": "<unknown>",
                        "connected_line_num": "<unknown>",
                        "position": 1,
                        "queue_name": "sales_1",
                        "tm_update": "2017-12-21T15:17:01.813584621Z",
                        "unique_id": "1513869420.0",
                        "wait": null
                    },
                    ...
                ],
                "members": [
                    {
                        "calls_taken": 0,
                        "in_call": 0,
                        "last_call": 0,
                        "last_pause": 0,
                        "location": "Agent/10001",
                        "membership": "static",
                        "name": "Agent/10001",
                        "paused": 0,
                        "paused_reason": "",
                        "penalty": 0,
                        "queue_name": "sales_1",
                        "ring_inuse": null,
                        "state_interface": "Agent/10001",
                        "status": 4,
                        "tm_update": "2017-12-21T15:16:24.593579797Z"
                    },
                    ...
                ],
                "queue": {
                    "abandoned": 0,
                    "calls": 0,
                    "completed": 0,
                    "hold_time": 0,
                    "max": 0,
                    "name": "sales_1",
                    "service_level": 5,
                    "service_level_perf": 0.0,
                    "strategy": "ringall",
                    "talk_time": 0,
                    "tm_update": "2017-12-21T15:16:24.490123142Z",
                    "weight": 0
                }
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-21T15:17:03.457906728Z"
  }

.. _queue_statuses_detail:
  
/queue/statuses/<detail>
========================

Methods
-------
GET : Get queue status detail info of given queue info.

.. _get_queue_statuses_detail:

Method: GET
-----------
Get queue status detail info of given queue info.

Call
++++
::

  GET /queue/statuses/<detail>

Method parameters

* ``detail``: queue name.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "queue": {...},
       "entries": [{...}, ...],
       "members": [{...}, ...]
     }
   }

* ``queue``: See detail at queue info.
* ``entries``: See detail at entry info.
* ``members``: See detail at member info.

Example
+++++++
::

  $ curl -X GET localhost:8081/queue/statuses/sales_1
  
  {
    "api_ver": "0.1",
    "result": {
        "entries": [
            {
                "caller_id_name": "pjagent-01",
                "caller_id_num": "pjagent-01",
                "channel": "PJSIP/pjagent-01-00000002",
                "connected_line_name": "<unknown>",
                "connected_line_num": "<unknown>",
                "position": 1,
                "queue_name": "sales_1",
                "tm_update": "2017-12-21T15:33:50.630820416Z",
                "unique_id": "1513870429.2",
                "wait": null
            }
        ],
        "members": [
            {
                "calls_taken": 0,
                "in_call": 0,
                "last_call": 0,
                "last_pause": 0,
                "location": "Agent/10001",
                "membership": "static",
                "name": "Agent/10001",
                "paused": 0,
                "paused_reason": "",
                "penalty": 0,
                "queue_name": "sales_1",
                "ring_inuse": null,
                "state_interface": "Agent/10001",
                "status": 4,
                "tm_update": "2017-12-21T15:33:37.613513551Z"
            },
            ...
        ],
        "queue": {
            "abandoned": 2,
            "calls": 0,
            "completed": 0,
            "hold_time": 0,
            "max": 0,
            "name": "sales_1",
            "service_level": 5,
            "service_level_perf": 0.0,
            "strategy": "ringall",
            "talk_time": 0,
            "tm_update": "2017-12-21T15:33:37.526617804Z",
            "weight": 0
        }
    },
    "statuscode": 200,
    "timestamp": "2017-12-21T15:33:51.732925780Z"
  }
