.. _core_event:

.. _core_channel:

core.channel
============
Notify core channel event.

.. _core_channel_create:

core.channel.create
-------------------
Notify event for core channel create.

Topic
+++++
::

  /core/channels/<target>
  
* ``target``: URI encoded caller_id_name.

Event
+++++
::

  {
    "core.channel.create": {
      ...
    }
  }

* See detail at Asterisk's channel info.
  
Example
+++++++
::

  topic: /core/channels/pjagent-01

  {
    "core.channel.create": {
      "unique_id": "1513946313.6",
      "duration": null,
      "linked_id": "1513946313.6",
      "context": "demo",
      "connected_line_num": "<unknown>",
      "account_code": "",
      "channel_state_desc": "Ring",
      "connected_line_name": "<unknown>",
      "caller_id_name": "pjagent-01",
      "channel": "PJSIP/pjagent-01-00000006",
      "channel_state": 4,
      "caller_id_num": "pjagent-01",
      "language": "en",
      "exten": "4002",
      "variables": {},
      "hangup_cause": null,
      "priority": "1",
      "application": null,
      "application_data": null,
      "tm_update": "2017-12-22T12:38:33.971448254Z",
      "bridge_id": null,
      "hangup_cause_desc": null
    }
  }

.. _core_channel_update:
  
core.channel.update
-------------------
Notify event for core channel update.

Topic
+++++
::

  /core/channels/<target>
  
* ``target``: URI encoded caller_id_name.

Event
+++++
::

  {
    "core.channel.update": {
      ...
    }
  }

* See detail at Asterisk's channel info.
  
Example
+++++++
::

  topic: /core/channels/pjagent-01

  {
    "core.channel.update": {
      "unique_id": "1513946313.6",
      "duration": null,
      "linked_id": "1513946313.6",
      "context": "queue_sales",
      "connected_line_num": "<unknown>",
      "account_code": "",
      "channel_state_desc": "Ring",
      "connected_line_name": "<unknown>",
      "caller_id_name": "pjagent-01",
      "channel": "PJSIP/pjagent-01-00000006",
      "channel_state": 4,
      "caller_id_num": "pjagent-01",
      "language": "en",
      "exten": "s",
      "variables": {
        "ABANDONED": "TRUE"
      },
      "hangup_cause": null,
      "priority": "2",
      "application": "Queue",
      "application_data": "",
      "tm_update": "2017-12-22T12:38:37.542000617Z",
      "bridge_id": null,
      "hangup_cause_desc": null
    }
  }

  
.. _core_channel_delete:
  
core.channel.delete
-------------------
Notify event for core channel delete.

Topic
+++++
::

  /core/channels/<target>
  
* ``target``: URI encoded caller_id_name.

Event
+++++
::

  {
    "core.channel.delete": {
      ...
    }
  }

* See detail at Asterisk's channel info.
  
Example
+++++++
::

  topic: /core/channels/pjagent-01

  {
    "core.channel.delete": {
      "unique_id": "1513946313.6",
      "duration": null,
      "linked_id": "1513946313.6",
      "context": "queue_sales",
      "connected_line_num": "<unknown>",
      "account_code": "",
      "channel_state_desc": "Ring",
      "connected_line_name": "<unknown>",
      "caller_id_name": "pjagent-01",
      "channel": "PJSIP/pjagent-01-00000006",
      "channel_state": 4,
      "caller_id_num": "pjagent-01",
      "language": "en",
      "exten": "s",
      "variables": {
        "ABANDONED": "TRUE",
        "QUEUEPOSITION": "1"
      },
      "hangup_cause": 127,
      "priority": "2",
      "application": "Queue",
      "application_data": "",
      "tm_update": "2017-12-22T12:38:37.623937167Z",
      "bridge_id": null,
      "hangup_cause_desc": "Interworking, unspecified"
    }
  }