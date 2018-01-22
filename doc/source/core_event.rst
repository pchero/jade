.. _core_event:

.. _core_agi:

core.agi
========
Notify core agi event.

.. _core_agi_create:

core.agi.create
---------------
Notify event for core agi create.

Topic
+++++
::

  /core/agis/<target>

* ``target``: URI encoded caller_id_name.

Event
+++++
::

  {
    "core.agi.create": {
      ...
    }
  }

* See detail at Asterisk's agi info.

Example
+++++++
::

  topic: /core/agis/pjagent-02
  
  {
    "core.agi.create": {
      "unique_id": "1516635793.1",
      "channel": "PJSIP/pjagent-02-00000001",
      "linked_id": "1516635793.1",
      "channel_state_desc": "Ring",
      "language": "en",
      "channel_state": 4,
      "caller_id_name": "pjagent-02",
      "caller_id_num": "pjagent-02",
      "priority": "2",
      "connected_line_num": "<unknown>",
      "connected_line_name": "<unknown>",
      "account_code": "",
      "context": "sample-agi-async",
      "env": {
        "agi_request": "async",
        "agi_uniqueid": "1516635793.1",
        "agi_enhanced": "0.0",
        "agi_callingpres": "0",
        "agi_version": "GIT-master-516ab38M",
        "agi_callington": "0",
        "agi_channel": "PJSIP/pjagent-02-00000001",
        "agi_language": "en",
        "agi_extension": "s",
        "agi_type": "PJSIP",
        "agi_callerid": "pjagent-02",
        "agi_callingani2": "0",
        "agi_priority": "2",
        "agi_calleridname": "pjagent-02",
        "agi_dnid": "4006",
        "agi_callingtns": "0",
        "agi_rdnis": "unknown",
        "agi_arg_1": "test argument 1",
        "agi_context": "sample-agi-async",
        "agi_accountcode": "",
        "agi_threadid": "1984414752",
        "agi_arg_2": "test argument 2"
        },
      "exten": "s",
      "cmd": {},
      "tm_update": "2018-01-22T15:43:13.166737961Z"
    }
  }


.. _core_agi_update:
  
core.agi.update
-------------------
Notify event for core agi update.

Topic
+++++
::

  /core/agis/<target>

* ``target``: URI encoded caller_id_name.

Event
+++++
::

  {
    "core.agi.update": {
      ...
    }
  }

* See detail at Asterisk's channel info.
  
Example
+++++++
::

  topic: /core/agi/pjagent-01

  {
    "core.agi.update": {
      "unique_id": "1516637665.7",
      "channel_state": 6,
      "caller_id_name": "pjagent-02",
      "tm_update": "2018-01-22T16:14:51.595209834Z",
      "cmd": {
        "77cdcf4c-feed-11e7-bd12-5b24ee4818c3": {
          "result": "200 result=0\n",
          "tm_update": "2018-01-22T16:14:51.595209834Z"
        }
      },
      "linked_id": "1516637665.7",
      "language": "en",
      "channel": "PJSIP/pjagent-02-00000006",
      "channel_state_desc": "Up",
      "caller_id_num": "pjagent-02",
      "connected_line_num": "<unknown>",
      "exten": "s",
      "connected_line_name": "<unknown>",
      "account_code": "",
      "context": "sample-agi-async",
      "priority": "2",
      "env": {
        "agi_request": "async",
        "agi_context": "sample-agi-async",
        "agi_channel": "PJSIP/pjagent-02-00000006",
        "agi_language": "en",
        "agi_type": "PJSIP",
        "agi_uniqueid": "1516637665.7",
        "agi_version": "GIT-master-516ab38M",
        "agi_callerid": "pjagent-02",
        "agi_callingani2": "0",
        "agi_callingtns": "0",
        "agi_calleridname": "pjagent-02",
        "agi_arg_1": "test argument 1",
        "agi_callingpres": "0",
        "agi_callington": "0",
        "agi_dnid": "4006",
        "agi_rdnis": "unknown",
        "agi_extension": "s",
        "agi_accountcode": "",
        "agi_priority": "2",
        "agi_enhanced": "0.0",
        "agi_threadid": "1954337824",
        "agi_arg_2": "test argument 2"
      }
    }
  }

.. _core_agi_delete:
  
core.agi.delete
---------------
Notify event for core agi delete.

Topic
+++++
::

  /core/agis/<target>

* ``target``: URI encoded caller_id_name.

Event
+++++
::

  {
    "core.agi.delete": {
      ...
    }
  }

* See detail at Asterisk's channel info.
  
Example
+++++++
::

  topic: /core/agi/pjagent-01

  {
    "core.agi.delete": {
      "unique_id": "1516637665.7",
      "channel_state": 6,
      "caller_id_name": "pjagent-02",
      "tm_update": "2018-01-22T16:24:23.1900430Z",
      "cmd": {
        "77cdcf4c-feed-11e7-bd12-5b24ee4818c3": {
          "result": "200 result=0\n",
          "tm_update": "2018-01-22T16:14:51.595209834Z"
        }
      },
      "linked_id": "1516637665.7",
      "language": "en",
      "channel": "PJSIP/pjagent-02-00000006",
      "channel_state_desc": "Up",
      "caller_id_num": "pjagent-02",
      "connected_line_num": "<unknown>",
      "exten": "s",
      "connected_line_name": "<unknown>",
      "account_code": "",
      "context": "sample-agi-async",
      "priority": "2",
      "env": {
        "agi_request": "async",
        "agi_context": "sample-agi-async",
        "agi_channel": "PJSIP/pjagent-02-00000006",
        "agi_language": "en",
        "agi_type": "PJSIP",
        "agi_uniqueid": "1516637665.7",
        "agi_version": "GIT-master-516ab38M",
        "agi_callerid": "pjagent-02",
        "agi_callingani2": "0",
        "agi_callingtns": "0",
        "agi_calleridname": "pjagent-02",
        "agi_arg_1": "test argument 1",
        "agi_callingpres": "0",
        "agi_callington": "0",
        "agi_dnid": "4006",
        "agi_rdnis": "unknown",
        "agi_extension": "s",
        "agi_accountcode": "",
        "agi_priority": "2",
        "agi_enhanced": "0.0",
        "agi_threadid": "1954337824",
        "agi_arg_2": "test argument 2"
      }
    }
  }

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