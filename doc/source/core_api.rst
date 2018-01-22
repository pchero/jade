.. _core_api:


.. _core_agis:

/core/agis
==========

Methods
-------
GET : Get list of all agis info.

.. _get_core_agis:

Method: GET
-----------
Get list of all agis info.

Call
++++
::

   GET /core/agis

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
            "unique_id": "<string>",
            "linked_id": "<string>",
                
            "channel": "<string>",
            "channel_state": <integer>,
            "channel_state_desc": "<string>",

            "context": "<string>",
            "exten": "<string>",
            "priority": "<string>",

            "caller_id_name": "<string>",
            "caller_id_num": "<string>",

            "connected_line_name": "<string>",
            "connected_line_num": "<string>",

            "account_code": "<string>",
            "language": "<string>",
            
            "env": {...},
            "cmd": {...},

            "tm_update": "<timestamp>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of agis.
    * See detail at :ref:`get_core_agis_detail`.


Example
+++++++
::

  $ curl -X GET localhost:8081/core/agis
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "account_code": "",
                "caller_id_name": "pjagent-02",
                "caller_id_num": "pjagent-02",
                "channel": "PJSIP/pjagent-02-00000007",
                "channel_state": 4,
                "channel_state_desc": "Ring",
                "cmd": {},
                "connected_line_name": "<unknown>",
                "connected_line_num": "<unknown>",
                "context": "sample-agi-async",
                "env": {
                    "agi_accountcode": "",
                    "agi_arg_1": "test argument 1",
                    "agi_arg_2": "test argument 2",
                    "agi_callerid": "pjagent-02",
                    "agi_calleridname": "pjagent-02",
                    "agi_callingani2": "0",
                    "agi_callingpres": "0",
                    "agi_callingtns": "0",
                    "agi_callington": "0",
                    "agi_channel": "PJSIP/pjagent-02-00000007",
                    "agi_context": "sample-agi-async",
                    "agi_dnid": "4006",
                    "agi_enhanced": "0.0",
                    "agi_extension": "s",
                    "agi_language": "en",
                    "agi_priority": "2",
                    "agi_rdnis": "unknown",
                    "agi_request": "async",
                    "agi_threadid": "1953600544",
                    "agi_type": "PJSIP",
                    "agi_uniqueid": "1516639454.9",
                    "agi_version": "GIT-master-516ab38M"
                },
                "exten": "s",
                "language": "en",
                "linked_id": "1516639454.9",
                "priority": "2",
                "tm_update": "2018-01-22T16:44:14.739567617Z",
                "unique_id": "1516639454.9"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-01-22T16:44:31.678361591Z"
  }


/core/agis/<detail>
=======================

Methods
-------
GET : Get agi's detail info of given agi info.

DELETE : Hangup the given agi.


.. _get_core_agis_detail:

Method: GET
-----------
Get agi's detail info of given agi info.

Call
++++
::

   GET /core/agis/<detail>
  
   
Method parameters

* ``detail``: unique id.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "unique_id": "<string>",
       "linked_id": "<string>",
              
       "channel": "<string>",
       "channel_state": <integer>,
       "channel_state_desc": "<string>",

       "context": "<string>",
       "exten": "<string>",
       "priority": "<string>",

       "caller_id_name": "<string>",
       "caller_id_num": "<string>",
       
       "connected_line_name": "<string>",
       "connected_line_num": "<string>",

       "account_code": "<string>",
       "language": "<string>",
        
       "env": {
         "<key>": "<string>",
         ...
       },
       "cmd": {
         "<command-id>": {
           "command": "<string>",
           "result": "<string>",
           "tm_create": "<timestamp>"
           "tm_update": "<timestamp>"
       },
              
       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``unique_id``: Channel's unique id.
* ``linked_id``: Channel's linked id.

* ``channel``: Channel's name.
* ``channel_state``: Channel's state.
* ``channel_state_desc``: Channel's state description.

* ``context``: Context.
* ``exten``: Extension.
* ``priority``: Priority

* ``caller_id_name``: Caller's id name.
* ``caller_id_num``: Caller's id number.

* ``connected_line_name``: Caller's line name.
* ``connected_line_num``: Caller's line number.

* ``account_code``: Account code.
* ``language``: Language.

* ``env``: agi channel environment key-values.
* ``cmd``: Command list
    * ``command-id``: Command id.
        * ``command``: Command.
        * ``result``: Command result.
        * ``tm_create``: Command created timestamp.
        * ``tm_update``: Command updated timestamp.

Example
+++++++
::

  $ curl localhost:8081/core/agis/1516639454.9
  
  {
    "api_ver": "0.1",
    "result": {
        "account_code": "",
        "caller_id_name": "pjagent-02",
        "caller_id_num": "pjagent-02",
        "channel": "PJSIP/pjagent-02-00000007",
        "channel_state": 4,
        "channel_state_desc": "Ring",
        "cmd": {},
        "connected_line_name": "<unknown>",
        "connected_line_num": "<unknown>",
        "context": "sample-agi-async",
        "env": {
            "agi_accountcode": "",
            "agi_arg_1": "test argument 1",
            "agi_arg_2": "test argument 2",
            "agi_callerid": "pjagent-02",
            "agi_calleridname": "pjagent-02",
            "agi_callingani2": "0",
            "agi_callingpres": "0",
            "agi_callingtns": "0",
            "agi_callington": "0",
            "agi_channel": "PJSIP/pjagent-02-00000007",
            "agi_context": "sample-agi-async",
            "agi_dnid": "4006",
            "agi_enhanced": "0.0",
            "agi_extension": "s",
            "agi_language": "en",
            "agi_priority": "2",
            "agi_rdnis": "unknown",
            "agi_request": "async",
            "agi_threadid": "1953600544",
            "agi_type": "PJSIP",
            "agi_uniqueid": "1516639454.9",
            "agi_version": "GIT-master-516ab38M"
        },
        "exten": "s",
        "language": "en",
        "linked_id": "1516639454.9",
        "priority": "2",
        "tm_update": "2018-01-22T16:44:14.739567617Z",
        "unique_id": "1516639454.9"
    },
    "statuscode": 200,
    "timestamp": "2018-01-22T16:44:49.638007048Z"
  }

.. _delete_core_channels_detail:

Method: DELETE
--------------
Hangup the given agi.

Call
++++
::

   DELETE /core/agis/<detail>
  
   
Method parameters

* ``detail``: unique id.

Returns
+++++++
::

   {
     $defhdr
   }

Example
+++++++
::

  $ curl -X DELETE localhost:8081/core/agis/1516639454.9
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-22T16:45:26.945414749Z"
  }

.. _core_channels:

/core/channels
==============

Methods
-------
GET : Get list of all channels info.

.. _get_core_channels:

Method: GET
-----------
Get list of all channels info.

Call
++++
::

   GET /core/channels

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
            "unique_id": "<string>",
            "linked_id": "<string>",
                
            "channel": "<string>",
            "channel_state": <integer>,
            "channel_state_desc": "<string>",

            "context": "<string>",
            "exten": "<string>",
            "priority": "<string>",

            "caller_id_name": "<string>",
            "caller_id_num": "<string>",

            "connected_line_name": "<string>",
            "connected_line_num": "<string>",

            "account_code": "<string>",
            "language": "<string>",
            
            "hangup_cause": <number>,
            "hangup_cause_desc": "<string>",

            "variables": {},

            "tm_update": "<timestamp>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of channels.
    * See detail at :ref:`get_core_channels_detail`.


Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/core/channels
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "account_code": "",
                "application": "Queue",
                "application_data": "",
                "bridge_id": null,
                "caller_id_name": "pjagent-01",
                "caller_id_num": "pjagent-01",
                "channel": "PJSIP/pjagent-01-00000007",
                "channel_state": 4,
                "channel_state_desc": "Ring",
                "connected_line_name": "<unknown>",
                "connected_line_num": "<unknown>",
                "context": "queue_sales",
                "duration": null,
                "exten": "s",
                "hangup_cause": null,
                "hangup_cause_desc": null,
                "language": "en",
                "linked_id": "1513946896.7",
                "priority": "2",
                "tm_update": "2017-12-22T12:48:17.211205731Z",
                "unique_id": "1513946896.7",
                "variables": {}
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-22T12:48:26.824442473Z"
  }


/core/channels/<detail>
=======================

Methods
-------
GET : Get channel's detail info of given channel info.

DELETE : Hangup the given channel.


.. _get_core_channels_detail:

Method: GET
-----------
Get channel's detail info of given channel info.

Call
++++
::

   GET /core/channels/<detail>
  
   
Method parameters

* ``detail``: unique id.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "unique_id": "<string>",
       "linked_id": "<string>",
              
       "channel": "<string>",
       "channel_state": <integer>,
       "channel_state_desc": "<string>",

       "context": "<string>",
       "exten": "<string>",
       "priority": "<string>",

       "caller_id_name": "<string>",
       "caller_id_num": "<string>",
       
       "connected_line_name": "<string>",
       "connected_line_num": "<string>",

       "account_code": "<string>",
       "language": "<string>",
       
       "application": "<string>",
       "application_data": "<string>",
       
       "hangup_cause": <number>,
       "hangup_cause_desc": "<string>",
       
       "variables": {},
       
       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``unique_id``: Channel's unique id.
* ``linked_id``: Channel's linked id.

* ``channel``: Channel's name.
* ``channel_state``: Channel's state.
* ``channel_state_desc``: Channel's state description.

* ``context``: Context.
* ``exten``: Extension.
* ``priority``: Priority

* ``caller_id_name``: Caller's id name.
* ``caller_id_num``: Caller's id number.

* ``connected_line_name``: Caller's line name.
* ``connected_line_num``: Caller's line number.

* ``account_code``: Account code.
* ``language``: Language.

* ``application``: Application name.
* ``application_data``: Application data.

* ``hangup_cause``: Hangup reason.
* ``hangup_cause_desc``: Hangup reason description.

* ``variables``: List of channel's variables.

Example
+++++++
::

  $ curl -X GET localhost:8081/core/channels/1513946896.7
      
  {
    "api_ver": "0.1",
    "result": {
        "account_code": "",
        "application": "Queue",
        "application_data": "",
        "bridge_id": null,
        "caller_id_name": "pjagent-01",
        "caller_id_num": "pjagent-01",
        "channel": "PJSIP/pjagent-01-00000007",
        "channel_state": 4,
        "channel_state_desc": "Ring",
        "connected_line_name": "<unknown>",
        "connected_line_num": "<unknown>",
        "context": "queue_sales",
        "duration": null,
        "exten": "s",
        "hangup_cause": null,
        "hangup_cause_desc": null,
        "language": "en",
        "linked_id": "1513946896.7",
        "priority": "2",
        "tm_update": "2017-12-22T12:48:17.211205731Z",
        "unique_id": "1513946896.7",
        "variables": {}
    },
    "statuscode": 200,
    "timestamp": "2017-12-22T12:49:31.608415721Z"
  }

.. _delete_core_channels_detail:

Method: DELETE
--------------
Hangup the given channel.

Call
++++
::

   DELETE /core/channels/<detail>
  
   
Method parameters

* ``detail``: unique id.

Returns
+++++++
::

   {
     $defhdr
   }

Example
+++++++
::

  $ curl -X DELETE localhost:8081/core/channels/1513550354.0
      
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:11:49.996318852Z"
  }

   
/core/modules
=============

Methods
-------
GET : Get list of all modules info.

.. _get_core_modules:

Method: GET
-----------
Get list of all modules info.

Call
++++
::

   GET /core/modules

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           "load": "<string>",
           "name": "<string>",
           "size": number,
           "tm_update": "<timestamp>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of channels.
    * See detail at :ref:`get_core_modules_detail`.


Example
+++++++
::

  $ curl -X GET localhost:8081/core/modules
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "load": "unknown",
                "name": "app_voicemail",
                "size": 1083280,
                "tm_update": "2017-12-17T22:11:40.375546224Z"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-17T23:14:08.548863974Z"
  }

  
/core/modules/<detail>
======================

Methods
-------
GET : Get detail info of given module

POST : Load given module.

PUT : Reload given module.

DELETE : Unload given module.


.. _get_core_modules_detail:

Method: GET
-----------
Get list of all modules info.

Call
++++
::

   GET /core/modules/<detail>
   
Method parameters

* ``detail``: module name

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "load": "<string>",
       "name": "<string>",
       "size": number,
       "tm_update": "<timestamp>"
     }
   }
  
* ``load``: load status.
* ``name``: module name.
* ``size``: module size.

Example
+++++++
::

  $ curl -X GET localhost:8081/core/modules/app_voicemail
  
  {
    "api_ver": "0.1",
    "result": {
      "load": "unknown",
      "name": "app_voicemail",
      "size": 1083280,
      "tm_update": "2017-12-17T23:28:02.30024695Z"
    },
    "statuscode": 200,
    "timestamp": "2017-12-17T23:28:07.214355673Z"
  }

  
.. _post_core_modules_detail:

Method: POST
------------
Load the given module.

Call
++++
::

   POST /core/modules/<detail>
   
Method parameters

* ``detail``: module name

Returns
+++++++
::

   {
     $defhdr
   }

Example
+++++++
::

  $ curl -X POST localhost:8081/core/modules/app_voicemail
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:35:07.579784864Z"
  }


.. _put_core_modules_detail:

Method: PUT
-----------
Reload the given module.

Call
++++
::

   PUT /core/modules/<detail>
   
Method parameters

* ``detail``: module name

Returns
+++++++
::

   {
     $defhdr
   }

Example
+++++++
::

  $ curl -X PUT localhost:8081/core/modules/app_voicemail
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:36:24.508748449Z"
  }


.. _delete_core_modules_detail:

Method: DELETE
--------------
Unload the given module.

Call
++++
::

  DELETE /core/modules/<detail>
   
Method parameters

* ``detail``: module name

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -X DELETE localhost:8081/core/modules/app_voicemail
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:38:17.170752025Z"
  }

  
/core/systems
=============

Methods
-------
GET : Get list of all systems info.

.. _get_core_systems:

Method: GET
-----------
Get list of all modules info.

Call
++++
::

   GET /core/systems

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
            "id": "<string>",
            "system_name": "<string>",
            
            "ami_version": "<string>",
            "ast_version": "<string>",
            
            "cdr_enabled": "<string>",
            "http_enabled": "<string>",
            
            "current_calls": <number>,
            "max_calls": <number>,
            "max_file_handles": <number>,
            "max_load_avg": <real>,
            
            "real_time_enabled": "<string>",
            
            "reload_date": "<string>",
            "reload_time": "<string>",
            
            "run_group": "<string>",
            "run_user": "<string>",
            
            "startup_date": "<string>",
            "startup_time": "<string>",
            
            "tm_update": "<string>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of channels.
    * See detail at :ref:`get_core_systems_detail`.


Example
+++++++
::

  $ curl -X GET localhost:8081/core/systems
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "ami_version": "4.0.0",
                "ast_version": "GIT-master-e97e415M",
                "cdr_enabled": "Yes",
                "current_calls": 0,
                "http_enabled": "No",
                "id": "1",
                "max_calls": 0,
                "max_file_handles": 0,
                "max_load_avg": 0.0,
                "real_time_enabled": "No",
                "reload_date": "2017-12-17",
                "reload_time": "23:36:24",
                "run_group": "",
                "run_user": "",
                "startup_date": "2017-12-17",
                "startup_time": "10:17:20",
                "system_name": "",
                "tm_update": "2017-12-17T23:38:54.940300313Z"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-17T23:38:55.914253052Z"
  }


/core/systems/<detail>
======================

Methods
-------
GET : Get detail info of given system.

.. _get_core_systems_detail:

Method: GET
-----------
Get detail info of given system.

Call
++++
::

   GET /core/systems/<detail>

Method parameters

* ``detail``: system id.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
        "id": "<string>",
        "system_name": "<string>",
        
        "ami_version": "<string>",
        "ast_version": "<string>",
        
        "cdr_enabled": "<string>",
        "http_enabled": "<string>",
        "real_time_enabled": "<string>",
        
        "current_calls": <number>,
        "max_calls": <number>,
        "max_file_handles": <number>,
        "max_load_avg": <real>,
        
        "reload_date": "<string>",
        "reload_time": "<string>",
        
        "run_group": "<string>",
        "run_user": "<string>",
        
        "startup_date": "<string>",
        "startup_time": "<string>",
        
        "tm_update": "<string>"
     }
   }
  

* ``id``: system id.
* ``system_name``: system name.

* ``ami_version``: AMI version.
* ``ast_version``: Asterisk version.

* ``cdr_enabled``: Cdr enanbled or not. If enabled "Yes".
* ``http_enabled``: http enabled or not. If enabled "Yes".
* ``real_time_enabled``: real time enabled or not. If enabled "Yes".

* ``current_calls``: Current call count.
* ``max_calls``: Maximum call count.
* ``max_file_handles``: Maximum file handle count.
* ``max_load_avg``: Max load average.
        
* ``reload_date``: Reloaded date.
* ``reload_time``: Reloaded time.
        
* ``run_group``: Process running group.
* ``run_user``: Process running user.
        
* ``startup_date``: Startup date.
* ``startup_time``: Startup time.

Example
+++++++
::

  $ curl -X GET localhost:8081/core/systems/1
  
  {
    "api_ver": "0.1",
    "result": {
        "ami_version": "4.0.0",
        "ast_version": "GIT-master-e97e415M",
        "cdr_enabled": "Yes",
        "current_calls": 0,
        "http_enabled": "No",
        "id": "1",
        "max_calls": 0,
        "max_file_handles": 0,
        "max_load_avg": 0.0,
        "real_time_enabled": "No",
        "reload_date": "2017-12-17",
        "reload_time": "23:36:24",
        "run_group": "",
        "run_user": "",
        "startup_date": "2017-12-17",
        "startup_time": "10:17:20",
        "system_name": "",
        "tm_update": "2017-12-17T23:42:16.933212413Z"
    },
    "statuscode": 200,
    "timestamp": "2017-12-17T23:42:17.210353468Z"
  }

