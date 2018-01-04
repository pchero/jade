.. _park_event:

.. _park_parkedcall:

park.parkedcall
===============
Notify park parkedcall event.

.. _park_pakedcall_create:

core.channel.create
-------------------
Notify event for core channel create.

Topic
+++++
::

  /park/parkinglots/<target>
  
* ``target``: URI encoded parking lot.

Event
+++++
::

  {
    "park.parkedcall.create": {
      ...
    }
  }

* See detail at parkedcall info.
  
Example
+++++++
::

  topic: /park/parkinglots/default

  {
    "park.parkedcall.create": {
      "parkee_unique_id": "1514424631.119",
      "parking_lot": "default",
      "parkee_caller_id_num": "pjagent-01",
      "parkee_exten": "s",
      "parkee_linked_id": "1514424631.119",
      "parkee_connected_line_num": "<unknown>",
      "parkee_channel": "PJSIP/pjagent-01-00000048",
      "parkee_account_code": "",
      "parkee_caller_id_name": "pjagent-01",
      "parkee_channel_state_desc": "Up",
      "tm_update": "2017-12-28T01:30:31.382834514Z",
      "parkee_channel_state": "6",
      "parking_timeout": 45,
      "parkee_connected_line_name": "<unknown>",
      "parkee_context": "sample_park",
      "parker_dial_string": "PJSIP/pjagent-01",
      "parking_duration": 0,
      "parkee_priority": "2",
      "parking_space": "701"
    }
  }


.. _park_parkedcall_update:
  
park.parkedcall.update
----------------------
Notify event for park parkedcall update.

Topic
+++++
::

  /park/parkinglots/<target>
  
* ``target``: URI encoded parking lot.

Event
+++++
::

  {
    "park.parkedcall.update": {
      ...
    }
  }

* See detail at parkedcall info.
  
Example
+++++++
::

  ...

  
.. _park_parkedcall_delete:
  
park.parkedcall.delete
----------------------
Notify event for park parkedcall delete.

Topic
+++++
::

  /park/parkinglots/<target>
  
* ``target``: URI encoded parking lot.

Event
+++++
::

  {
    "park.parkedcall.delete": {
      ...
    }
  }

* See detail at parkedcall info.
  
Example
+++++++
::

  topic: /park/parkinglots/default

  {
    "park.parkedcall.delete": {
      "parkee_unique_id": "1514425145.127",
      "parking_lot": "default",
      "parkee_caller_id_num": "pjagent-01",
      "parkee_exten": "s",
      "parkee_linked_id": "1514425145.127",
      "parkee_connected_line_num": "<unknown>",
      "parkee_channel": "PJSIP/pjagent-01-0000004c",
      "parkee_account_code": "",
      "parkee_caller_id_name": "pjagent-01",
      "parkee_channel_state_desc": "Up",
      "tm_update": "2017-12-28T01:39:05.298799696Z",
      "parkee_channel_state": "6",
      "parking_timeout": 45,
      "parkee_connected_line_name": "<unknown>",
      "parkee_context": "sample_park",
      "parker_dial_string": "PJSIP/pjagent-01",
      "parking_duration": 0,
      "parkee_priority": "2",
      "parking_space": "701"
    }
  }

