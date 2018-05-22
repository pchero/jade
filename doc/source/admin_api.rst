.. _admin_api:

.. _admin_core_channels:

/admin/core/channels
====================
Methods
-------
GET : Get list of all channels info.

.. _get_admin_core_channels:

Method: GET
-----------
Get list of all channels info.

Call
++++
::

  GET /admin/core/channels

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
  * See detail at :ref:`get_admin_core_channels_detail`.

Example
+++++++
::

  $ curl -k -X GET https://192.168.200.10:8081/v1/admin/core/channels
  
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

  
.. _admin_core_channels_detail:

/admin/core/channels/<detail>
=============================

Methods
-------
GET : Get channel's detail info of given channel info.

DELETE : Hangup the given channel.

.. _get_admin_core_channels_detail:

Method: GET
-----------
Get channel's detail info of given channel info.

Call
++++
::

   GET /admin/core/channels/<detail>
  
   
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

  $ curl -k -X GET https://localhost:8081/v1/admin/core/channels/1513946896.7
      
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

  
.. _delete_admin_core_channels_detail:

Method: DELETE
--------------
Hangup the given channel.

Call
++++
::

   DELETE /admin/core/channels/<detail>
  
   
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

  $ curl -k -X DELETE https://localhost:8081/v1/admin/core/channels/1513550354.0
      
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:11:49.996318852Z"
  }

.. _admin_core_modules:

/admin/core/modules
===================

Methods
-------
GET : Get list of all modules info.

.. _get_admin_core_modules:

Method: GET
-----------
Get list of all modules info.

Call
++++
::

   GET /admin/core/modules

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
  * See detail at :ref:`get_admin_core_modules_detail`.


Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/core/modules
  
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


.. _admin_core_modules_detail:
  
/admin/core/modules/<detail>
============================

Methods
-------
GET : Get detail info of given module

POST : Load given module.

PUT : Reload given module.

DELETE : Unload given module.

.. _get_admin_core_modules_detail:

Method: GET
-----------
Get list of all modules info.

Call
++++
::

   GET /admin/core/modules/<detail>
   
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

  $ curl -k -X GET https://localhost:8081/v1/admin/core/modules/app_voicemail
  
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

.. _post_admin_core_modules_detail:

Method: POST
------------
Load the given module.

Call
++++
::

   POST /admin/core/modules/<detail>
   
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

  $ curl -k -X POST https://localhost:8081/v1/admin/core/modules/app_voicemail
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:35:07.579784864Z"
  }

.. _put_admin_core_modules_detail:

Method: PUT
-----------
Reload the given module.

Call
++++
::

   PUT /admin/core/modules/<detail>
   
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

  $ curl -k -X PUT https://localhost:8081/v1/admin/core/modules/app_voicemail
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:36:24.508748449Z"
  }


.. _delete_admin_core_modules_detail:

Method: DELETE
--------------
Unload the given module.

Call
++++
::

  DELETE /admin/core/modules/<detail>
   
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

  $ curl -k -X DELETE https://localhost:8081/v1/admin/core/modules/app_voicemail
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-17T23:38:17.170752025Z"
  }

.. _admin_core_systems:

/admin/core/systems
===================

Methods
-------
GET : Get list of all systems info.

.. _get_admin_core_systems:

Method: GET
-----------
Get list of all modules info.

Call
++++
::

   GET /admin/core/systems

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
  * See detail at :ref:`get_admin_core_systems_detail`.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/core/systems
  
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

.. _admin_core_systems_detail:

/admin/core/systems/<detail>
============================

Methods
-------
GET : Get detail info of given system.

.. _get_admin_core_systems_detail:

Method: GET
-----------
Get detail info of given system.

Call
++++
::

   GET /admin/core/systems/<detail>

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

  $ curl -k -X GET https://localhost:8081/v1/admin/core/systems/1
  
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


.. _admin_dialplan_adps:


/admin/dialplan/adps
====================

Methods
-------
GET : Get list of all adp(async dialplan)s info.

POST : Create new adp(async dialplan) info.

.. _get_admin_dialplan_adps:

Method: GET
-----------
Get list of all adp(async dialplan)s info.

Call
++++
::

   GET /admin/dialplan/adps

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           ...
         },
         ...
       ]
     }
   }

Return parameters

* ``list`` : array of itmes.
  * See detail dialplans detail info.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/dialplan/adps
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "command": null,
                "detail": null,
                "dpma_uuid": "1d47735d-2265-463c-908a-d37dea085c21",
                "name": "test_dialplan_1",
                "sequence": 10,
                "tm_create": "2018-01-23T01:05:53.878811167Z",
                "tm_update": "2018-01-23T01:27:54.670312624Z",
                "uuid": "91bba56b-7ec5-4fe2-a724-9f39d527da8c"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-01-23T01:29:02.718231538Z"
  }

Method: POST
------------
Create new adp(async dialplan) info.

Call
++++
::

  POST /admin/dialplan/adps
  
  {
    ...
  }

Returns
+++++++
::

   {
     $defhdr
   }

Example
+++++++
::

  $ curl -k -X POST https://localhost:8081/v1/admin/dialplan/adps -d 
  '{"name": "test_dialplan_1", "dpma_uuid":"1d47735d-2265-463c-908a-d37dea085c21", "sequence": 1}'

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:38:38.709550080Z"
  }

.. _admin_dialplan_adps_detail:
  
/admin/dialplan/adps/<detail>
=============================

Methods
-------
GET : Get async dialplan detail info of given detail.

PUT : Update async dialplan detail info of given detail.

DELETE : Delete the given async dialplan info.

.. _get_admin_dialplan_adps_detail:

Method: GET
-----------
Get async dialplan detail info of given detail.

Call
++++
::

  GET /admin/dialplan/adps/<detail>


Method parameters

* ``detail``: dialplan uuid

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
        "uuid": "<string>",
        "dpma_uuid": "<string>",
        "sequence": <number>,
        
        "name": "<string>",
        "detail": "<string>",
        
        "command": "<string>",
        
        "tm_create": "<timestamp>",
        "tm_update": "<timestamp>"
    }
  }

Return parameters

* ``uuid``: dialplan uuid.
* ``dpma_uuid``: Dpma uuid.
* ``sequence``: Sequence.

* ``name``: Name.
* ``detail``: Detail info.

* ``command``: Command.

* ``tm_create``: timestamp.
* ``tm_update``: timestamp.

Example
+++++++
::

  $ curl -k https://localhost:8081/v1/admin/dialplan/adps/abde9ef2-7255-4e64-acbe-a6edb964c442
  
  {
    "api_ver": "0.1",
    "result": {
        "command": null,
        "detail": null,
        "dpma_uuid": "1d47735d-2265-463c-908a-d37dea085c21",
        "name": "test_dialplan_1",
        "sequence": 1,
        "tm_create": "2018-01-23T01:38:38.698487606Z",
        "tm_update": null,
        "uuid": "abde9ef2-7255-4e64-acbe-a6edb964c442"
    },
    "statuscode": 200,
    "timestamp": "2018-01-23T01:38:53.794937064Z"
  }

Method: PUT
-----------
Update async dialplan detail info of given detail.

Call
++++
::

  PUT /admin/dialplan/adps/<detail>
  
  {
    ...
  }


Method parameters

* ``detail``: dialplan uuid.

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -k -X PUT https://localhost:8081/v1/admin/dialplan/adps/91bba56b-7ec5-4fe2-a724-9f39d527da8c -d \
  '{"uuid": "91bba56b-7ec5-4fe2-a724-9f39d527da8c", \
  "dpma_uuid": "1d47735d-2265-463c-908a-d37dea085c21", "sequence": 10}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:27:54.683794074Z"
  }
  
Method: DELETE
--------------
DELETE : Delete the given async dialplan info.

Call
++++
::

  DELETE /admin/dialplan/adps/<detail>

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/dialplan/adps/91bba56b-7ec5-4fe2-a724-9f39d527da8c
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:29:34.538575375Z"
  }

.. _admin_dialplan_adpmas:

/admin/dialplan/adpmas
======================

Methods
-------
GET : Get list of all adpma info.

POST : Create new adpma info.

.. _get_admin_dialplan_adpmas:

Method: GET
-----------
Get list of all adpma info.

Call
++++
::

   GET /admin/dialplan/adpmas

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           ...
         },
         ...
       ]
     }
   }

Return parameters

* ``list`` : array of itmes.
  * See detail adpma detail info.

Example
+++++++
::

  $ curl -k https://localhost:8081/v1/admin/dialplan/adpmas
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "detail": "test dpma dpma",
                "name": "test dpma",
                "tm_create": "2018-01-22T23:40:55.513269352Z",
                "tm_update": null,
                "uuid": "1d47735d-2265-463c-908a-d37dea085c21"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-01-22T23:41:06.819146813Z"
  }

Method: POST
------------
Create new adpma info.

Call
++++
::

  POST /admin/dialplan/adpmas
  
  {
    ...
  }

Returns
+++++++
::

   {
     $defhdr
   }

Example
+++++++
::

  $ curl -k -X POST https://localhost:8081/v1/admin/dialplan/adpmas -d '{"name": "test dpma", "detail": "test dpma dpma"}'

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-22T23:40:55.524091997Z"
  }

.. _admin_dialplan_adpmas_detail:

/admin/dialplan/adpmas/<detail>
===============================

Methods
-------
GET : Get adpmas detail info of given detail.

PUT : Update adpmas detail info of given detail.

DELETE : Delete the given adpmas info.

.. _get_admin_dialplan_adpmas_detail:

Method: GET
-----------
Get adpma detail info of given detail.

Call
++++
::

  GET /admin/dialplan/adpmas/<detail>

Method parameters

* ``detail``: adpma uuid

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
        "uuid": "<string>",
        
        "name": "<string>",
        "detail": "<string>",
        
        "tm_create": "<string>",
        "tm_update": "<string>"
        
    }
  }

Return parameters

* ``uuid``: adpma uuid.

* ``name``: Name.
* ``detail``: Detail info.

* ``tm_create``: timestamp.
* ``tm_update``: timestamp.

Example
+++++++
::

  $ curl -k https://localhost:8081/v1/admin/dialplan/adpmas/1d47735d-2265-463c-908a-d37dea085c21
  
  {
    "api_ver": "0.1",
    "result": {
        "detail": "test dpma dpma",
        "name": "test dpma",
        "tm_create": "2018-01-22T23:40:55.513269352Z",
        "tm_update": null,
        "uuid": "1d47735d-2265-463c-908a-d37dea085c21"
    },
    "statuscode": 200,
    "timestamp": "2018-01-22T23:43:06.585017860Z"
  }

Method: PUT
-----------
Update adpmas detail info of given detail.

Call
++++
::

  PUT /admin/dialplan/adpmas/<detail>
  
  {
    ...
  }


Method parameters

* ``detail``: dpma uuid.

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -k -X PUT https://localhost:8081/v1/admin/dialplan/adpmas/1d47735d-2265-463c-908a-d37dea085c21 -d 
  '{"name": "test change name" }'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-22T23:46:38.238159711Z"
  }

Method: DELETE
--------------
DELETE : Delete the given adpma info.

Call
++++
::

  DELETE /admin/dialplan/adpmas/<detail>

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/dialplan/adpmas/1d47735d-2265-463c-908a-d37dea085c21
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:59:22.411717755Z"
  }

.. _admin_dialplan_configurations:
  
/admin/dialplan/configurations
==============================
Methods
-------
GET : Get all dialplan configurations info.

Method: GET
-----------
Get all dialplan configurations info.

Example
+++++++
::

  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
              "data": "...",
              "name": "extensions.conf"
            },
            {
              "data": "..."
              "name": "extensions.conf.2018-05-20T19:54:00.66126645Z"
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T13:24:14.979704248Z"
  }


.. _admin_dialplan_configurations_detail:

/admin/dialplan/configurations/<detail>
=======================================
Methods
-------
GET: Get given dialplan configuration detail info.

PUT: Update given dialplan configuration detail info.

DELETE: Delete given dialplan configuration detail info.

Method: GET
-----------
Get given dialplan configuration detail info.

Example
+++++++
::

  $ curl -k https://localhost:8081/v1/admin/dialplan/configurations/extensions.conf\?authtoken=86f7c25d-54db-4ffd-9bf8-8f691fbb4b97

  {
    "api_ver": "0.1",
    "result": {
      "data": "...",
      "name": "extensions.conf"
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T13:33:06.376506142Z"
  }


Method: PUT
-----------
Update given dialplan configuration detail info.

Method: DELETE
--------------
Delete given dialplan configuration detail info.

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/dialplan/configurations/extensions.conf.2018-05-21T08:49:27.783332067Z\?authtoken=86f7c25d-54db-4ffd-9bf8-8f691fbb4b97
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-22T13:35:18.521871974Z"
  }


.. _admin_dialplan_sdps:

/admin/dialplan/sdps
====================

Methods
-------
GET: Get all of dialplam's static dialplans info.

Method: GET
-----------

Example
+++++++
::

  $ curl -k https://localhost:8081/v1/admin/dialplan/sdps\?authtoken=86f7c25d-54db-4ffd-9bf8-8f691fbb4b97
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "data": [
                    {
                        "exten": "> _X.,1,NoOp(jade-queue)"
                    },
                    {
                        "same": "> n,Queue(jade-queue)"
                    }
                ],
                "name": "jade-queue"
            },
            {
                "data": [],
                "name": "jade_demo2"
            },
            {
                "data": [
                    {
                        "exten": "> 100,1,NoOp(sample_park)"
                    },
                    {
                        "same": "> n,park()"
                    },
                    {
                        "exten": "> _X.,1,BackGround(demo-congrats)"
                    },
                    {
                        "same": "> n,Hangup()"
                    }
                ],
                "name": "jade_demo"
            },
            {
                "data": [],
                "name": "jade_demo3"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T13:36:41.645574898Z"
}


.. _admin_dialplan_sdps_detail:

/admin/dialplan/sdps/<detail>
=============================
Methods
-------
Get: Get detail info of the given sdp.

PUT: Update detail info of the given sdp.

DELETE: Delete detail info of the given sdp.

Method: GET
-----------
Get detail info of the given sdp.

Example
+++++++
::

  $ curl -k https://localhost:8081/v1/admin/dialplan/sdps/jade_demo\?authtoken=86f7c25d-54db-4ffd-9bf8-8f691fbb4b97
  
  {
    "api_ver": "0.1",
    "result": {
        "data": [
            {
                "exten": "> 100,1,NoOp(sample_park)"
            },
            {
                "same": "> n,park()"
            },
            {
                "exten": "> _X.,1,BackGround(demo-congrats)"
            },
            {
                "same": "> n,Hangup()"
            }
        ],
        "name": "jade_demo"
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T13:39:39.664914968Z"
  }


Method: PUT
-----------
Update detail info of the given sdp.

Example
+++++++

Method: DELETE
--------------
Delete detail info of the given sdp.

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/dialplan/sdps/jade_demo3\?authtoken=86f7c25d-54db-4ffd-9bf8-8f691fbb4b97
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-22T13:42:00.607209696Z"
  }


/admin/info
===========
Methods
-------
GET: Get current user's info.

PUT: Update current user's info.

Method: GET
-----------
Get current user's info.

Example
+++++++
::

  $ curl -k https://localhost:8081/v1/admin/info\?authtoken=86f7c25d-54db-4ffd-9bf8-8f691fbb4b97
  
  {
    "api_ver": "0.1",
    "result": {
        "name": "Test Administrator",
        "tm_create": "2018-04-28T05:03:36.560694725Z",
        "tm_update": "2018-04-28T07:10:29.116108422Z",
        "username": "admin",
        "uuid": "ed629979-802b-40f0-9098-b30b2988f9b6"
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T13:42:29.17642767Z"
  }

Method: PUT
-----------
Update current user's info.

.. _admin_login:

/admin/login
============

Methods
-------
POST: Create new authtoken.

DELETE: Delete the given authtoken.

Method: POST
------------
Create new authtoken.

Example
+++++++
::

  $ curl -k -X POST https://localhost:8081/v1/admin/login -u admin:admin
  
  {
    "api_ver": "0.1",
    "result": {
        "authtoken": "7ac326bf-e146-493a-b9c7-bfb990264b59"
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T13:49:12.711802985Z"
  }
  
Method: DELETE
--------------
Delete the given authtoken

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/login\?authtoken=7ac326bf-e146-493a-b9c7-bfb990264b59
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-22T13:50:29.818196101Z"
  }


/admin/park/cfg_parkinglots
===========================

Methods
-------
GET: Get all of the config parkinglots info.

POST: Create new config parkinglot info.

Method: GET
-----------
Get all of the config parkinglots info.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/park/cfg_parkinglots\?authtoken=0e81eee1-f215-4397-ac99-e0de58b87b88
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "data": {
                    "context": "teset",
                    "parkpos": "851-900"
                },
                "name": "test_park1"
            },
            {
                "data": {},
                "name": "jade-tesetqueue2"
            },
            {
                "data": {
                    "comebackcontext": "edvinapark-timeout",
                    "comebackdialtime": "90",
                    "comebacktoorigin": "no",
                    "context": "edvina_park",
                    "findslot": "next",
                    "parkedmusicclass": "edvina",
                    "parkpos": "800-850"
                },
                "name": "edvina"
            },
            {
                "data": {},
                "name": "jade-testqueue2"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T14:09:47.561146235Z"
  }



Method: POST
------------
Create new config parkinglot info.

Example
+++++++


/admin/park/cfg_parkinglots/<detail>
=====================================
Methods
-------
GET: Get detail info of the given config parkinglot.

PUT: Update detail info of the given config parkinglot.

DELETE: Delete detail info of the given config parkinglot.

Method: GET
-----------
Get detail info of the given config parkinglot.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/park/cfg_parkinglots/edvina\?authtoken=0e81eee1-f215-4397-ac99-e0de58b87b88
  
  {
    "api_ver": "0.1",
    "result": {
        "data": {
            "comebackcontext": "edvinapark-timeout",
            "comebackdialtime": "90",
            "comebacktoorigin": "no",
            "context": "edvina_park",
            "findslot": "next",
            "parkedmusicclass": "edvina",
            "parkpos": "800-850"
        },
        "name": "edvina"
    },
    "statuscode": 200,
    "timestamp": "2018-05-22T14:14:53.789233359Z"
  }

Method: PUT
-----------
Update detail info of the given config parkinglot.

Method: DELETE
--------------
Delete detail info of the given config parkinglot.

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/park/cfg_parkinglots/jade-tesetqueue2\?authtoken=0e81eee1-f215-4397-ac99-e0de58b87b88
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-22T14:17:32.187887090Z"
  }

/admin/park/configurations
==========================

/admin/park/configurations/<detail>
===================================



/admin/park/parkedcalls
=======================

Methods
-------
GET : Get list of all parked calls info.

.. _get_admin_park_parkedcalls:

Method: GET
-----------
Get list of all parked calls info.

Call
++++
::

   GET /admin/park/parkedcalls

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           ...
         },
         ...
       ]
     }
   }

Return parameters

* ``list`` : array of itmes.
  * See detail parkedcall detail info.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/park/parkedcalls
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "parkee_account_code": "",
                "parkee_caller_id_name": "pjagent-01",
                "parkee_caller_id_num": "pjagent-01",
                "parkee_channel": "PJSIP/pjagent-01-00000044",
                "parkee_channel_state": "6",
                "parkee_channel_state_desc": "Up",
                "parkee_connected_line_name": "<unknown>",
                "parkee_connected_line_num": "<unknown>",
                "parkee_context": "sample_park",
                "parkee_exten": "s",
                "parkee_linked_id": "1514423807.111",
                "parkee_priority": "2",
                "parkee_unique_id": "1514423807.111",
                "parker_dial_string": "PJSIP/pjagent-01",
                "parking_duration": 0,
                "parking_lot": "default",
                "parking_space": "701",
                "parking_timeout": 45,
                "tm_update": "2017-12-28T01:16:48.343347741Z"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-28T01:17:10.187476889Z"
  }

.. _admin_park_parkedcalls_detail:

/admin/park/parkedcalls/<detail>
================================

Methods
-------
GET : Get parked_call's detail info of given info.

DELETE : Hangup the given parked call info.

.. _get_admin_park_parkedcalls_detail:

Method: GET
-----------
Get parked_call's detail info of given info.

Call
++++
::

   GET /admin/park/parkedcalls/<detail>

Method parameters

* ``detail``: Parkee's unique id.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "parkee_unique_id": "<string>",
       "parkee_linked_id": "<string>",

       "parkee_channel": "<string>",
       "parkee_channel_state": "<string>",
       "parkee_channel_state_desc": "<string>",

       "parkee_caller_id_name": "<string>",
       "parkee_caller_id_num": "<string>",

       "parkee_connected_line_name": "<string>",
       "parkee_connected_line_num": "<string>,

       "parkee_account_code": "<string>",

       "parkee_context": "<string>",
       "parkee_exten": "<string>",
       "parkee_priority": "<string>",

       "parker_dial_string": "<string>",
       "parking_duration": <integer>,
       "parking_lot": "<string>",
       "parking_space": "<string>",
       "parking_timeout": <integer>,
       
       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``parkee_unique_id``: Parkee's unique id.
* ``parkee_linked_id``: Parkee's linked id.

* ``parkee_channel``: Parkee's channel name.
* ``parkee_channel_state``: Parkee's channele state.
* ``parkee_channel_state_desc``: Parkee's channel state description.

* ``parkee_caller_id_name``: Parkee's caller id name.
* ``parkee_caller_id_num``: Parkee's caller id number.

* ``parkee_connected_line_name``: Parkee's connected line name.
* ``parkee_connected_line_num``: Parkee's connected line number.

* ``parkee_account_code``: Parkee's account code.

* ``parkee_context``: Parkee's current context.
* ``parkee_exten``: Parkee's current exten.
* ``parkee_priority``: Parkee's current priority.

* ``parker_dial_string``: Dial String that can be used to call back the parker on ParkingTimeout.
* ``parking_duration``: Time the parkee has been in the parking bridge since tm_update(in seconds).
* ``parking_lot``: Name of the parking lot that the parkee is parked in.
* ``parking_space``: Parking Space that the parkee is parked in.
* ``parking_timeout``: Time remaining until the parkee is forcefully removed from parking in seconds since tm_update.

Example
+++++++
::

  $ curl -X GET localhost:8081/park/parkedcalls/1514423984.115
  
  {
    "api_ver": "0.1",
    "result": {
        "parkee_account_code": "",
        "parkee_caller_id_name": "pjagent-01",
        "parkee_caller_id_num": "pjagent-01",
        "parkee_channel": "PJSIP/pjagent-01-00000046",
        "parkee_channel_state": "6",
        "parkee_channel_state_desc": "Up",
        "parkee_connected_line_name": "<unknown>",
        "parkee_connected_line_num": "<unknown>",
        "parkee_context": "sample_park",
        "parkee_exten": "s",
        "parkee_linked_id": "1514423984.115",
        "parkee_priority": "2",
        "parkee_unique_id": "1514423984.115",
        "parker_dial_string": "PJSIP/pjagent-01",
        "parking_duration": 0,
        "parking_lot": "default",
        "parking_space": "701",
        "parking_timeout": 45,
        "tm_update": "2017-12-28T01:19:44.271005802Z"
    },
    "statuscode": 200,
    "timestamp": "2017-12-28T01:19:53.629685348Z"
  }


.. _delete_admin_park_parkedcalls_detail:

Method: DELETE
--------------
Hangup the given parked call info.

Call
++++
::

   DELETE /admin/park/parkedcalls/<detail>

Method parameters

* ``detail``: Parkee's unique id.

Returns
+++++++
::

   {
     $defhdr
   }

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/park/parkedcalls/1515019232.8
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-03T22:40:50.55634319Z"
  }

.. _admin_park_parkinglots:

/admin/park/parkinglots
=======================

Methods
-------
GET : Get list of all parking lot info.

.. _get_admin_park_parkinglots:

Method: GET
-----------
Get list of all parking lots info.

Call
++++
::

   GET /admin/park/parkinglots

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           ...
         },
         ...
       ]
     }
   }

Return parameters

* ``list`` : array of itmes.
   * See detail at parking lot detail info.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/park/parkinglots
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "name": "default",
                "start_space": "701",
                "stop_spcae": "720",
                "timeout": 45,
                "tm_update": "2017-12-28T01:16:46.350523532Z"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-28T01:21:57.180448453Z"
  }

.. _admin_park_parkinglots_detail:

/admin/park/parkinglots/<detail>
================================

Methods
-------
GET : Get parking lot's detail info of given detail.

PUT : Update parking lot's detail info of given detail.

DELETE : Delete the given parking lot info.

.. _get_admin_park_parkinglots_detail:

Method: GET
-----------
Get parking lot's detail info of given info.

Call
++++
::

  GET /admin/park/parkinglots/<detail>


Method parameters

* ``detail``: url encoded parking lot's name.

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "name": "<string>",

      "start_space": "<string>",
      "stop_spcae": "<string>",

      "timeout": <integer>,
      
      "tm_update": "<timestamp>"
    }
  }

Return parameters

* ``name``: Parking lot's name.

* ``start_space``: Parking lot's start space.
* ``stop_spcae``: Parking lot's stop space.

* ``timeout``: Timeout second in the parking lot.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/park/parkinglots/default
  
  {
    "api_ver": "0.1",
    "result": {
        "name": "default",
        "start_space": "701",
        "stop_spcae": "720",
        "timeout": 45,
        "tm_update": "2017-12-28T01:16:46.350523532Z"
    },
    "statuscode": 200,
    "timestamp": "2017-12-28T01:25:47.123913131Z"
  }



.. _put_admin_park_parkinglots_detail:

Method: PUT
-----------
Update parking lot's detail info of given detail.

Call
++++
::

  PUT /admin/park/parkinglots/<detail>
  
  {
    ...
  }


Method parameters

* ``detail``: url encoded parking lot's name.

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -k -X PUT https://localhost:8081/v1/admin/park/parkinglots/test_parkinglot -d \
  '{"context": "> edvina_park", "parkpos": "> 800-900", "findslot": "> next", \
  "comebacktoorigin": "no", "comebackdialtime": "90", \
  "comebackcontext": "edvinapark-timeout", "parkedmusicclass": "edvina" }'
  
  {
    "api_ver": "0.1",
    "timestamp": "2018-01-03T22:17:22.76376966Z",
    "statuscode": 200
  }
  
.. _delete_admin_park_parkinglots_detail:

Method: DELETE
--------------
DELETE : Delete the given parking lot info.

Call
++++
::

  DELETE /admin/park/parkinglots/<detail>

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -k -X DELETE https://localhost:8081/v1/admin/park/parkinglots/test_parkinglot
  
  {
    "api_ver": "0.1",
    "timestamp": "2018-01-03T21:50:01.672074605Z",
    "statuscode": 200
  }


/admin/pjsip/aors
=================
Methods
-------
GET : Get all list of aors.

.. _get_admin_pjsip_aors:

Method: GET
-----------
GET : Get all list of aors.

Call
++++
::

  GET ^/admin/pjsip/aors
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          "object_name": "<string>",
          "object_type": "<string>",
     
          "endpoint_name": "<string>",
          "contacts": "<string>",
          "contacts_registered": <number>,
          "total_contacts": <number>,
          
          "authenticate_qualify": "<string>",
          
          "default_expiration": <number>,
          
          "mailboxes": "<string>",
     
          "max_contacts": <number>,
          "maximum_expiration": <number>,
          "minimum_expiration": <number>,
     
          "outbound_proxy": "<string>",
          
          "qualify_frequency": <number>,
          "qualify_timeout": <number>,
          
          "remove_existing": "<string>",
          "support_path": "<string>",
          
          "voicemail_extension": "<string>",
          
          "tm_update": "<timestamp>"
        },
        ...
      ]
    }
  }

   
Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/pjsip/aors
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "authenticate_qualify": "false",
                "contacts": "",
                "contacts_registered": 0,
                "default_expiration": 3600,
                "endpoint_name": "199",
                "mailboxes": "",
                "max_contacts": 1,
                "maximum_expiration": 7200,
                "minimum_expiration": 60,
                "object_name": "199",
                "object_type": "aor",
                "outbound_proxy": "",
                "qualify_frequency": 0,
                "qualify_timeout": 3,
                "remove_existing": "true",
                "support_path": "false",
                "tm_update": "2017-12-19T20:03:34.948197060Z",
                "total_contacts": 0,
                "voicemail_extension": ""
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:03:41.263715886Z"
  }

.. _admin_pjsip_aors_detail:

/admin/pjsip/aors/<detail>
==========================
Methods
-------
GET : Get detail aor info of given aor info.

.. _get_admi_pjsip_aors_detail:

Method: GET
-----------
GET : Get detail aor info of given aor info.


Call
++++
::

  GET /admin/pjsip/aors/<detail>
  
Method parameters

* ``detail``: aor name.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "object_name": "<string>",
      "object_type": "<string>",
      
      "endpoint_name": "<string>",
      "contacts": "<string>",
      "contacts_registered": <number>,
      "total_contacts": <number>,
      
      "authenticate_qualify": "<string>",
      
      "default_expiration": <number>,
      
      "mailboxes": "<string>",
      
      "max_contacts": <number>,
      "maximum_expiration": <number>,
      "minimum_expiration": <number>,
      
      "outbound_proxy": "<string>",
      
      "qualify_frequency": <number>,
      "qualify_timeout": <number>,
      
      "remove_existing": "<string>",
      "support_path": "<string>",
      
      "voicemail_extension": "<string>",
      
      "tm_update": "<timestamp>"
    }
  }

* ``object_name``: aor name.
* ``object_type``: type. Always would be "aor".

   
Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/pjsip/aors/pjagent-01
  
  {
    "api_ver": "0.1",
    "result": {
        "authenticate_qualify": "false",
        "contacts": "pjagent-01/sip:35740219@10.12.118.159:49027",
        "contacts_registered": 1,
        "default_expiration": 3600,
        "endpoint_name": "pjagent-01",
        "mailboxes": "",
        "max_contacts": 1,
        "maximum_expiration": 7200,
        "minimum_expiration": 60,
        "object_name": "pjagent-01",
        "object_type": "aor",
        "outbound_proxy": "",
        "qualify_frequency": 0,
        "qualify_timeout": 3,
        "remove_existing": "true",
        "support_path": "false",
        "tm_update": "2017-12-19T20:03:36.64487822Z",
        "total_contacts": 1,
        "voicemail_extension": ""
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:21:29.736727558Z"
  }

.. _admin_pjsip_auths:

/admin/pjsip/auth
=================
Methods
-------
GET : Get all list of auths.

.. _get_admin_pjsip_auths:

Method: GET
-----------
GET : Get all list of auths.

Call
++++
::

  GET /admin/pjsip/auths
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          "object_name": "<string>",
          "object_type": "<string>",

          "username": "<string>",
          "auth_type": "<string>",
          "password": "<string>",
          "md5_cred": "<string>",

          "realm": "<string>",
          "nonce_lifetime": <number>,
          
          "endpoint_name": "<string>",

          "tm_update": "<timestamp>"
        },
        ...
      ]
    }
  }

* ``list``
    * ``object_name``: auth name.
    * ``object_type``: type. Always will be "auth".
   
Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/pjsip/auths
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "auth_type": "userpass",
                "endpoint_name": "pjagent-01",
                "md5_cred": "",
                "nonce_lifetime": 32,
                "object_name": "pjagent-01",
                "object_type": "auth",
                "password": "pjagent-01",
                "realm": "",
                "tm_update": "2017-12-19T20:03:36.42243261Z",
                "username": "pjagent-01"
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:26:09.770460034Z"
  }


.. _admin_pjsip_auths_detail:

/admin/pjsip/auth/<detail>
==========================
Methods
-------
GET : Get detail auth info of given auth info.

.. _get_pjsip_auths_detail:

Method: GET
-----------
GET : Get detail auth info of given auth info.


Call
++++
::

  GET /admin/pjsip/auths/<detail>
  
Method parameters

* ``detail``: auth name.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "object_name": "<string>",
      "object_type": "<string>",

      "username": "<string>",
      "auth_type": "<string>",
      "password": "<string>",
      "md5_cred": "<string>",

      "realm": "<string>",
      "nonce_lifetime": <number>,
      
      "endpoint_name": "<string>",

      "tm_update": "<timestamp>"
    }
  }

* ``object_name``: aor name.
* ``object_type``: type. Always would be "aor".

   
Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/pjsip/auths/pjagent-01
  
  {
    "api_ver": "0.1",
    "result": {
        "auth_type": "userpass",
        "endpoint_name": "pjagent-01",
        "md5_cred": "",
        "nonce_lifetime": 32,
        "object_name": "pjagent-01",
        "object_type": "auth",
        "password": "pjagent-01",
        "realm": "",
        "tm_update": "2017-12-19T20:03:36.42243261Z",
        "username": "pjagent-01"
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:34:54.165290324Z"
  }

/admin/pjsip/configurations
===========================

/admin/pjsip/configurations/<detail>
====================================

/admin/pjsip/contacts
=====================
Methods
-------
GET : Get all list of contacts.

.. _get_admin_pjsip_contacts:

Method: GET
-----------
GET : Get all list of contacts.

Call
++++
::

  GET /admin/pjsip/contacts
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          "id": "<string>",
          "call_id": "<string>",
          
          "uri": "<string>",
          "aor": "<string>",
          "endpoint_name": "<string>",
          "status": "<string>",
          
          "qualify_frequency": 0,
          "qualify_timout": 3,
          
          "authentication_qualify": 0,
          
          
          "outbound_proxy": "<string>",
          "path": "<string>",
          
          "reg_expire": 1513714265,
          "round_trip_usec": "<string>",
          
          "user_agent": "<string>",
          "via_address": "<string>",
          
          "tm_update": "<string>"
        },
        ...
      ]
    }
  }

* ``list``
    * ``object_name``: auth name.
    * ``object_type``: type. Always will be "auth".
   
Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/pjsip/contacts
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "aor": "pjagent-01",
                "authentication_qualify": 0,
                "call_id": "c67ef58c-3b65-44cd-a9a8-784e2af542d3",
                "endpoint_name": "pjagent-01",
                "id": "pjagent-01;@116e48d167ff94856cbc9009cdc735b2",
                "outbound_proxy": "",
                "path": "",
                "qualify_frequency": 0,
                "qualify_timout": 3,
                "reg_expire": 1513714265,
                "round_trip_usec": "0",
                "status": "Unknown",
                "tm_update": "2017-12-19T20:29:36.751968068Z",
                "uri": "sip:35740219@10.12.118.159:49027",
                "user_agent": "Blink 3.0.3 (Linux)",
                "via_address": "10.12.118.159:49027"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:38:09.405901164Z"
  }
  
.. _admin_pjsip_contacts_detail:

/admin/pjsip/contacts/<detail>
==============================
Methods
-------
GET : Get detail contact info of given contact info.

.. _get_admin_pjsip_contacts_detail:

Method: GET
-----------
GET : Get detail contact info of given contact info.

Call
++++
::

  GET /admin/pjsip/contacts/<detail>
  
Method parameters

* ``detail``: uri encoded contact id.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "id": "<string>",
      "call_id": "<string>",
      
      "uri": "<string>",
      "aor": "<string>",
      "endpoint_name": "<string>",
      "status": "<string>",
      
      "qualify_frequency": 0,
      "qualify_timout": 3,
      
      "authentication_qualify": 0,
      
      
      "outbound_proxy": "<string>",
      "path": "<string>",
      
      "reg_expire": 1513714265,
      "round_trip_usec": "<string>",
      
      "user_agent": "<string>",
      "via_address": "<string>",
      
      "tm_update": "<string>"
    }
  }

   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/contacts/pjagent-01%3B%40116e48d167ff94856cbc9009cdc735b2
  
  {
    "api_ver": "0.1",
    "result": {
        "aor": "pjagent-01",
        "authentication_qualify": 0,
        "call_id": "c67ef58c-3b65-44cd-a9a8-784e2af542d3",
        "endpoint_name": "pjagent-01",
        "id": "pjagent-01;@116e48d167ff94856cbc9009cdc735b2",
        "outbound_proxy": "",
        "path": "",
        "qualify_frequency": 0,
        "qualify_timout": 3,
        "reg_expire": 1513716547,
        "round_trip_usec": "N/A",
        "status": "Unknown",
        "tm_update": "2017-12-19T20:45:42.698151858Z",
        "uri": "sip:35740219@10.12.118.159:49027",
        "user_agent": "Blink 3.0.3 (Linux)",
        "via_address": "10.12.118.159:49027"
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:51:43.977637876Z"
  }

.. _admin_pjsip_endpoints:
  
/admin/pjsip/endpoints
======================
Methods
-------
GET : Get all list of endpoints.

.. _get_admin_pjsip_endpoints:

Method: GET
-----------
GET : Get all list of endpoints.

Call
++++
::

  GET /admin/pjsip/endpoints
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          ...
        },
        ...
      ]
    }
  }

   
Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/pjsip/endpoints
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "account_code": "",
                "acl": "",
                "active_channels": "",
                "aggregate_mwi": "true",
                "allow": "(codec2|g723|ulaw|alaw|gsm|g726|g726aal2|adpcm|slin|slin|slin|slin|slin|slin|slin|slin|slin|lpc10|g729|speex|speex|speex|ilbc|g722|siren7|siren14|testlaw|g719|opus|jpeg|png|h261|h263|h263p|h264|mpeg4|vp8|vp9|red|t140|t38|silk|silk|silk|silk)",
                "allow_overlap": "true",
                "allow_subscribe": "true",
                "allow_transfer": "true",
                "aors": "pjagent-01",
                "asymmetric_rtp_codec": "false",
                "auth": "pjagent-01",
                "bind_rtp_to_media_address": "false",
                "bundle": "false",
                "call_group": "",
                "caller_id": "<unknown>",
                "caller_id_privacy": "allowed_not_screened",
                "caller_id_tag": "",
                "connected_line_method": "invite",
                "contact_acl": "",
                "context": "demo",
                "cos_audio": 0,
                "cos_video": 0,
                "device_state": "Not in use",
                "device_state_busy_at": 0,
                "direct_media": "true",
                "direct_media_glare_mitigation": "none",
                "direct_media_method": "invite",
                "disable_direct_media_on_nat": "false",
                "disallow": "",
                "dtls_ca_file": "",
                "dtls_ca_path": "",
                "dtls_cert_file": "",
                "dtls_cipher": "",
                "dtls_fingerprint": "SHA-256",
                "dtls_private_key": "",
                "dtls_rekey": 0,
                "dtls_setup": "active",
                "dtls_verify": "No",
                "dtmf_mode": "rfc4733",
                "fax_detect": "false",
                "fax_detect_time": 0,
                "force_avp": "false",
                "force_rport": "true",
                "from_domain": "",
                "from_user": "",
                "g_726_non_standard": "false",
                "ice_support": "false",
                "identify_by": "username,ip",
                "inband_progress": "false",
                "incoming_mwi_mailbox": "pjagent-01@vm-demo",
                "language": "",
                "mailboxes": "pjagent-01@vm-demo",
                "max_audio_streams": 1,
                "max_video_streams": 1,
                "media_address": "",
                "media_encryption": "no",
                "media_encryption_optimistic": "false",
                "media_use_received_transport": "false",
                "message_context": "",
                "moh_passthrough": "false",
                "moh_suggest": "default",
                "mwi_from_user": "",
                "mwi_subscribe_replaces_unsolicited": "false",
                "named_call_group": "",
                "named_pickup_group": "",
                "notify_early_inuse_ringing": "false",
                "object_name": "pjagent-01",
                "object_type": "endpoint",
                "one_touch_recording": "false",
                "outbound_auth": "",
                "outbound_proxy": "",
                "pickup_group": "",
                "preferred_codec_only": "false",
                "record_off_feature": "automixmon",
                "record_on_feature": "automixmon",
                "redirect_method": "",
                "refer_blind_progress": "true",
                "rel_100": "yes",
                "rewrite_contact": "false",
                "rpid_immediate": "false",
                "rtcp_mux": "false",
                "rtp_engine": "asterisk",
                "rtp_ipv6": "false",
                "rtp_keepalive": 0,
                "rtp_symmetric": "false",
                "rtp_timeout": 0,
                "rtp_timeout_hold": 0,
                "sdp_owner": "-",
                "sdp_session": "Asterisk",
                "send_diversion": "true",
                "send_pai": "false",
                "send_rpid": "false",
                "set_var": "",
                "srtp_tag32": "false",
                "sub_min_expiry": "0",
                "subscribe_context": "",
                "t38_udptl": "false",
                "t38_udptl_ec": "none",
                "t38_udptl_ipv6": "false",
                "t38_udptl_maxdatagram": 0,
                "t38_udptl_nat": "false",
                "timers": "yes",
                "timers_min_se": "90",
                "timers_sess_expires": 1800,
                "tm_update": "2017-12-19T20:45:42.632334496Z",
                "tone_zone": "",
                "tos_audio": 0,
                "tos_video": 0,
                "transport": "transport-udp",
                "trust_id_inbound": "false",
                "trust_id_outbound": "false",
                "use_avpf": "false",
                "use_ptime": "false",
                "user_eq_phone": "false",
                "voicemail_extension": "",
                "webrtc": "no"
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:55:13.546370914Z"
  }

.. _admin_pjsip_endpoints_detail:

/admin/pjsip/endpoints/<detail>
===============================
Methods
-------
GET : Get detail endpoint info of given endpoint info.

.. _get_admin_pjsip_endpoints_detail:

Method: GET
-----------
GET : Get detail endpoint info of given endpoint info.

Call
++++
::

  GET /admin/pjsip/endpoints/<detail>
  
Method parameters

* ``detail``: uri encoded endpoint name.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      ...
    }
  }


   
Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/pjsip/endpoints/pjagent-01
  
  {
    "api_ver": "0.1",
    "result": {
        "account_code": "",
        "acl": "",
        "active_channels": "",
        "aggregate_mwi": "true",
        "allow": "(codec2|g723|ulaw|alaw|gsm|g726|g726aal2|adpcm|slin|slin|slin|slin|slin|slin|slin|slin|slin|lpc10|g729|speex|speex|speex|ilbc|g722|siren7|siren14|testlaw|g719|opus|jpeg|png|h261|h263|h263p|h264|mpeg4|vp8|vp9|red|t140|t38|silk|silk|silk|silk)",
        "allow_overlap": "true",
        "allow_subscribe": "true",
        "allow_transfer": "true",
        "aors": "pjagent-01",
        "asymmetric_rtp_codec": "false",
        "auth": "pjagent-01",
        "bind_rtp_to_media_address": "false",
        "bundle": "false",
        "call_group": "",
        "caller_id": "<unknown>",
        "caller_id_privacy": "allowed_not_screened",
        "caller_id_tag": "",
        "connected_line_method": "invite",
        "contact_acl": "",
        "context": "demo",
        "cos_audio": 0,
        "cos_video": 0,
        "device_state": "Not in use",
        "device_state_busy_at": 0,
        "direct_media": "true",
        "direct_media_glare_mitigation": "none",
        "direct_media_method": "invite",
        "disable_direct_media_on_nat": "false",
        "disallow": "",
        "dtls_ca_file": "",
        "dtls_ca_path": "",
        "dtls_cert_file": "",
        "dtls_cipher": "",
        "dtls_fingerprint": "SHA-256",
        "dtls_private_key": "",
        "dtls_rekey": 0,
        "dtls_setup": "active",
        "dtls_verify": "No",
        "dtmf_mode": "rfc4733",
        "fax_detect": "false",
        "fax_detect_time": 0,
        "force_avp": "false",
        "force_rport": "true",
        "from_domain": "",
        "from_user": "",
        "g_726_non_standard": "false",
        "ice_support": "false",
        "identify_by": "username,ip",
        "inband_progress": "false",
        "incoming_mwi_mailbox": "pjagent-01@vm-demo",
        "language": "",
        "mailboxes": "pjagent-01@vm-demo",
        "max_audio_streams": 1,
        "max_video_streams": 1,
        "media_address": "",
        "media_encryption": "no",
        "media_encryption_optimistic": "false",
        "media_use_received_transport": "false",
        "message_context": "",
        "moh_passthrough": "false",
        "moh_suggest": "default",
        "mwi_from_user": "",
        "mwi_subscribe_replaces_unsolicited": "false",
        "named_call_group": "",
        "named_pickup_group": "",
        "notify_early_inuse_ringing": "false",
        "object_name": "pjagent-01",
        "object_type": "endpoint",
        "one_touch_recording": "false",
        "outbound_auth": "",
        "outbound_proxy": "",
        "pickup_group": "",
        "preferred_codec_only": "false",
        "record_off_feature": "automixmon",
        "record_on_feature": "automixmon",
        "redirect_method": "",
        "refer_blind_progress": "true",
        "rel_100": "yes",
        "rewrite_contact": "false",
        "rpid_immediate": "false",
        "rtcp_mux": "false",
        "rtp_engine": "asterisk",
        "rtp_ipv6": "false",
        "rtp_keepalive": 0,
        "rtp_symmetric": "false",
        "rtp_timeout": 0,
        "rtp_timeout_hold": 0,
        "sdp_owner": "-",
        "sdp_session": "Asterisk",
        "send_diversion": "true",
        "send_pai": "false",
        "send_rpid": "false",
        "set_var": "",
        "srtp_tag32": "false",
        "sub_min_expiry": "0",
        "subscribe_context": "",
        "t38_udptl": "false",
        "t38_udptl_ec": "none",
        "t38_udptl_ipv6": "false",
        "t38_udptl_maxdatagram": 0,
        "t38_udptl_nat": "false",
        "timers": "yes",
        "timers_min_se": "90",
        "timers_sess_expires": 1800,
        "tm_update": "2017-12-19T20:45:42.632334496Z",
        "tone_zone": "",
        "tos_audio": 0,
        "tos_video": 0,
        "transport": "transport-udp",
        "trust_id_inbound": "false",
        "trust_id_outbound": "false",
        "use_avpf": "false",
        "use_ptime": "false",
        "user_eq_phone": "false",
        "voicemail_extension": "",
        "webrtc": "no"
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T21:07:25.80668047Z"
  }

/admin/pjsip/registration_outbounds
===================================

/admin/pjsip/registration_outbounds/<detail>
============================================


/admin/queue/cfg_queues
=======================

/admin/queue/cfg_queues/<detail>
================================

/admin/queue/configurations
===========================

/admin/queue/configurations/<detail>
====================================

/admin/queue/entries
====================

Methods
-------
GET : Get list of all queue entries info.

.. _get_admin_queue_entries:

Method: GET
-----------
Get list of all queue entries info.

Call
++++
::

   GET /admin/queue/entries

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

  $ curl -k -X GET https://localhost:8081/v1/admin/queue/entries
   
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

/admin/queue/entries/<detail>
================================
Methods
-------
GET : Get queue entry detail info of given queue entry info.

.. _get_admin_queue_entries_detail:

Method: GET
-----------
Get queue entry detail info of given queue entry info.

Call
++++
::

   GET /admin/queue/entries/<detail>

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

  $ curl -k -X GET https://localhost:8081/v1/admin/queue/entries/1513557067.6
   
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


/admin/queue/members
====================
Methods
-------
GET : Get list of all queue members info.

.. _get_admin_queue_members:

Method: GET
-----------
Get list of all queue members info.

Call
++++
::

   GET /admin/queue/members

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
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
         },
         ...
       ]
     }
   }
  
* ``list`` : array of registry account.
  * See detail at :ref:`get_admin_queue_members_detail`.

Example
+++++++
::

  $ curl -k -X GET https://localhost:8081/v1/admin/queue/members
   
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "id": "sip/agent-02@sales_1",
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

.. _admin_queue_members_detail:

/admin/queue/members/<detail>
================================
Methods
-------
GET : Get queue member detail info of given queue member info.

.. _get_admin_queue_members_detail:

Method: GET
-----------
Get queue member detail info of given queue member info.

Call
++++
::

   GET /admin/queue/members/<detail>?queue_name=<string>

Method parameters

* ``detail``: member name.
* ``queue_name``: queue name.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "id":  "<string>",
       
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
* ``id``: member's id.

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

  $ curl -k -X GET https://localhost:8081/v1/admin/queue/members/Agent%2F10001\?queue_name=sales_1
   
  {
    "api_ver": "0.1",
    "result": {
        "id": "Agent/10001@sales_1",
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

.. _admin_queue_queues:

/admin/queue/queues
===================

Methods
-------
GET : Get list of all queues info.

.. _get_admin_queue_queues:

Method: GET
-----------
Get list of all queues info

Call
++++
::

   GET /admin/queue/queues

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

  $ curl -k -X GET https://localhost:8081/v1/admin/queue/queues
  
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

.. _admin_queue_queues_detail:

/admin/queue/queues/<detail>
============================
Methods
-------
GET : Get queue detail info of given queue info.

.. _get_admin_queue_queues_detail:

Method: GET
-----------
Get queue detail info of given queue info.

Call
++++
::

  GET /admin/queue/queues/<detail>

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

  $ curl -k -X GET https://localhost:8081/v1/admin/queue/queues/sales_1
  
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

.. _admin_user_users:

/admin/user/users
=================

Methods
-------
GET : Get the all users info.

POST : Create user info

Method: GET
-----------
Get the all users info.

Example
+++++++
::

  $ curl -k https://192.168.200.10:8081/v1/admin/user/users\?authtoken=986d5648-3905-4def-8479-009b33939273
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "name": "Test Administrator",
                "password": "admin",
                "tm_create": "2018-04-28T05:03:36.560694725Z",
                "tm_update": "2018-04-28T07:10:29.116108422Z",
                "username": "admin",
                "uuid": "ed629979-802b-40f0-9098-b30b2988f9b6"
            },
            {
                "name": "test1",
                "password": "test1",
                "tm_create": "2018-04-29T07:19:00.56688782Z",
                "tm_update": "2018-05-02T16:18:36.730339477Z",
                "username": "test1",
                "uuid": "65abf0b4-9cd5-4bff-8ec9-c03c1aea22d4"
            },
            {
                "name": "test2",
                "password": "test2",
                "tm_create": "2018-05-02T13:52:58.303710170Z",
                "tm_update": "2018-05-03T03:55:08.886606228Z",
                "username": "test2",
                "uuid": "16d1e8ea-1331-4ba7-a371-2c6261e1ce33"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-05-03T08:46:52.150892217Z"
  }

  
Method: POST
------------
Create user info

Example
+++++++
::

  $ curl -k -X POST https://192.168.200.10:8081/v1/admin/user/users\?authtoken=986d5648-3905-4def-8479-009b33939273 -d '{"name": "test3", "password": "test3", "username": "test3"}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-03T08:48:05.464359109Z"
  }

/admin/user/users/<detail>
==========================

Methods
-------
GET : Get the given detail user info.

PUT : Update the given detail user info.

DELETE : Delete the given detail user info.

.. _get_manager_users_detail:

Method: GET
-----------
Get the given detail user info.

Example
+++++++
::
  
  $ curl -k https://192.168.200.10:8081/v1/admin/user/users/16d1e8ea-1331-4ba7-a371-2c6261e1ce33\?authtoken=986d5648-3905-4def-8479-009b33939273
  
  {
    "api_ver": "0.1",
    "result": {
        "name": "test2",
        "password": "test2",
        "tm_create": "2018-05-02T13:52:58.303710170Z",
        "tm_update": "2018-05-03T03:55:08.886606228Z",
        "username": "test2",
        "uuid": "16d1e8ea-1331-4ba7-a371-2c6261e1ce33"
    },
    "statuscode": 200,
    "timestamp": "2018-05-03T08:47:13.766737242Z"
  }
  
Method: PUT
-----------
Update the given detail user info.

Example
+++++++
::

  $ curl -k -X PUT https://192.168.200.10:8081/v1/admin/user/users/4364a7cc-982b-448c-ba18-8a9b0fb3040d\?authtoken=986d5648-3905-4def-8479-009b33939273 -d '{"name": "test3 change"}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-03T08:53:07.924367640Z"
  }
  
Method: DELETE
--------------
Delete the given detail user info.

Example
+++++++
::

  $ curl -k -X DELETE https://192.168.200.10:8081/v1/admin/user/users/4364a7cc-982b-448c-ba18-8a9b0fb3040d\?authtoken=986d5648-3905-4def-8479-009b33939273

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-03T08:53:56.840482109Z"
  }

  
  
/admin/user/contacts
====================


/admin/user/contacts/<detail>
=============================


/admin/user/permissions
=======================


/admin/user/permissions/<detail>
=================================






  
  