.. _queue_event:

.. _queue_entry:

queue.entry
===========
Queue enty event.

.. _queue.entry.delete:

queue.entry.delete
------------------
Event for queue entry delete.

Topic
+++++
::

  /queue/statuses/<target>
  
* ``target``: URI encoded queue name.

Event
+++++
::

  {
    "queue.entry.delete": {
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

* See detail at Asterisk's queue info.
  
Example
+++++++
::

  topic: /queue/statuses/sales%5F1

  {
    "queue.entry.delete": {
      "unique_id": "1513886574.11",
      "connected_line_num": "<unknown>",
      "queue_name": "sales_1",
      "wait": null,
      "position": 1,
      "channel": "PJSIP/pjagent-01-0000000b",
      "caller_id_num": "pjagent-01",
      "caller_id_name": "pjagent-01",
      "connected_line_name": "<unknown>",
      "tm_update": "2017-12-21T20:02:55.766020378Z"
    }
  }

.. _queue_entry_update:
  
queue.entry.update
------------------
Event for queue entry insert/update.

Topic
+++++
::

  /queue/statuses/<target>
  
* ``target``: URI encoded queue name.

Event
+++++
::

  {
    "queue.entry.update": {
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

* See detail at Asterisk's queue info.
  
Example
+++++++
::

  topic: /queue/statuses/sales%5F1

  {
    "queue.entry.update": {
      "unique_id": "1513887042.12",
      "caller_id_name": "pjagent-01",
      "connected_line_num": "<unknown>",
      "position": 1,
      "tm_update": "2017-12-21T20:10:43.841799160Z",
      "queue_name": "sales_1",
      "channel": "PJSIP/pjagent-01-0000000c",
      "caller_id_num": "pjagent-01",
      "connected_line_name": "<unknown>",
      "wait": null
    }
  }


  
queue.member
============
Queue member event.
  
.. _queue.member.delete:

queue.member.delete
-------------------
Event for queue member delete.

Topic
+++++
::

  /queue/statuses/<target>
  
* ``target``: URI encoded queue name.

Event
+++++
::

  {
    "queue.entry.delete": {
       "id": "<string>",
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

* See detail at Asterisk's queue info.
  
Example
+++++++
::

  topic: /queue/statuses/sales%5F1

  {
    "queue.member.delete": {
      "location": "sip/agent-01",
      "penalty": 0,
      "ring_inuse": 1,
      "id": "sip/agent-01@sales_1",
      "state_interface": "sip/agent-01",
      "queue_name": "sales_1",
      "name": "sip/agent-01",
      "membership": "dynamic",
      "calls_taken": 0,
      "last_call": 0,
      "last_pause": 0,
      "in_call": 0,
      "status": 4,
      "paused": 0,
      "paused_reason": "",
      "tm_update": "2017-12-21T21:35:04.605327430Z"
    }
  }
  
.. _queue_member_update:
  
queue.member.update
-------------------
Event for queue member insert/update.

Topic
+++++
::

  /queue/statuses/<target>
  
* ``target``: URI encoded queue name.

Event
+++++
::

  {
    "queue.member.update": {
       "id": "<string>",
       
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


* See detail at Asterisk's queue info.
  
Example
+++++++
::

  topic: /queue/statuses/sales%5F1

  {
    "queue.member.update": {
      "location": "sip/agent-01",
      "penalty": 0,
      "ring_inuse": 1,
      "id": "sip/agent-01@sales_1",
      "state_interface": "sip/agent-01",
      "queue_name": "sales_1",
      "name": "sip/agent-01",
      "membership": "dynamic",
      "calls_taken": 0,
      "last_call": 0,
      "last_pause": 0,
      "in_call": 0,
      "status": 4,
      "paused": 0,
      "paused_reason": "",
      "tm_update": "2017-12-21T21:35:04.605327430Z"
    }
  }

  
queue.queue
===========
  
.. _queue.queue.update:

queue.queue.update
------------------

Topic
+++++
::

  /queue/statuses/<target>
  
* ``target``: URI encoded queue name.

Event
+++++
::

  {
    "queue.queue.update": {
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

* See detail at Asterisk's queue info.
  
Example
+++++++
::

  topic: /queue/statuses/sales%5F1

  {
    "queue.queue.update": {
      "weight": 0,
      "name": "sales_1",
      "service_level": 5,
      "abandoned": 6,
      "service_level_perf": 0.0,
      "max": 0,
      "hold_time": 0,
      "strategy": "ringall",
      "calls": 0,
      "tm_update": "2017-12-21T18:35:17.131303352Z",
      "talk_time": 0,
      "completed": 0
    }
  }
