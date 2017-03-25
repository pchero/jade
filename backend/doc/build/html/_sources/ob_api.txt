.. _ob_api:

*************
Outbound APIs
*************

/ob/plans
=========

Methods
-------
POST : Create new plan.

GET : Get list of all plans.

Method: POST
------------
Create new plan.

Call
++++
::

  POST /ob/plans
  
  {
    "name": "<string>",
    "detail": "<string>",
    
    "tech_name": "<string>",
    "trunk_name": "<string>",

    "dial_mode": <integer>,
    "dial_timeout": <integer>,
    "dl_end_handle": <integer>,
    "retry_delay": <integer>,

    "caller_id": "<string>",
    "service_level": <integer>,
    "early_media": "<string>",
    "codecs": "<string>",
    "variables": {"<string>": "<string>", ...},
    
    "max_retry_cnt_1": <integer>,
    "max_retry_cnt_2": <integer>,
    "max_retry_cnt_3": <integer>,
    "max_retry_cnt_4": <integer>,
    "max_retry_cnt_5": <integer>,
    "max_retry_cnt_6": <integer>,
    "max_retry_cnt_7": <integer>,
    "max_retry_cnt_8": <integer>
  }

* ``name``: Plan name.
* ``detail``: Detail info.

* ``tech_name``: Tech name for outbound dialing. See detail :ref:`ob_tech_name`.
* ``trunk_name``: Trunkname for outbound dialing. See detail :ref:`trunk_name`.

* ``dial_mode``: Dialling mode. See detail :ref:`ob_dial_mode`.
* ``dial_timeout``: Ringing timeout(ms).
* ``dl_end_handle``: Determine behavior of when the dial list end. See detail :ref:`ob_dial_list_end_handling`.
* ``retry_delay``: Delay time for next try(sec).

* ``caller_id``: Caller's id.
* ``service_level``: Determine service level.
* ``early_media``: Set to true to force call bridge on early media. 
* ``codecs``: Codecs. Comma-separated list of codecs to use for this call.
* ``variables``: variables info json object.

* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "uuid": "<string>",
      "name": "<string>",
      "detail": "<string>",
       
      "tech_name": "<string>",
      "trunk_name": "<string>",
   
      "dial_mode": <integer>,
      "dial_timeout": <integer>,
      "dl_end_handle": <integer>,
      "retry_delay": <integer>,
   
      "caller_id": "<string>",
      "service_level": <integer>,
      "early_media": "<string>",
      "codecs": "<string>",
      "variables": {"<string>": "<string>", ...},
       
      "max_retry_cnt_1": <integer>,
      "max_retry_cnt_2": <integer>,
      "max_retry_cnt_3": <integer>,
      "max_retry_cnt_4": <integer>,
      "max_retry_cnt_5": <integer>,
      "max_retry_cnt_6": <integer>,
      "max_retry_cnt_7": <integer>,
      "max_retry_cnt_8": <integer>,

      "in_use": <integer>,
      "tm_create": "<timestamp>",
      "tm_update": "<timestamp>",
      "tm_delete": "<timestamp>"
    }
  }

* ``uuid`` : Created plan uuid.
* ``name``: Plan name.
* ``detail``: Detail info.

* ``tech_name``: Tech name for outbound dialing. See detail :ref:`ob_tech_name`.
* ``trunk_name``: Trunkname for outbound dialing. See detail :ref:`trunk_name`.

* ``dial_mode``: Dialling mode. See detail :ref:`ob_dial_mode`.
* ``dial_timeout``: Ringing timeout(ms).
* ``dl_end_handle``: Determine behavior of when the dial list end. See detail :ref:`ob_dial_list_end_handling`.
* ``retry_delay``: Delay time for next try(sec).

* ``caller_id``: Caller's id.
* ``service_level``: Determine service level.
* ``early_media``: Set to true to force call bridge on early media. 
* ``codecs``: Codecs. Comma-separated list of codecs to use for this call.
* ``variables``: variables info json object.

* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.

Example
+++++++
::

   $ curl -X POST 192.168.200.10:8081/ob/plans -d \
   '{"name": "Test plan create", "variables": {"plan_key_1": "plean_val_1"}}'

   {
     "api_ver": "0.1",
     "result": {
       "caller_id": null,
       "codecs": null,
       "detail": null,
       "dial_mode": 0,
       "dial_timeout": 30000,
       "dl_end_handle": 1,
       "early_media": null,
       "in_use": 1,
       "max_retry_cnt_1": 5,
       "max_retry_cnt_2": 5,
       "max_retry_cnt_3": 5,
       "max_retry_cnt_4": 5,
       "max_retry_cnt_5": 5,
       "max_retry_cnt_6": 5,
       "max_retry_cnt_7": 5,
       "max_retry_cnt_8": 5,
       "name": "Test plan create",
       "retry_delay": 60,
       "service_level": 0,
       "tech_name": "SIP",
       "tm_create": "2017-03-11T12:29:48.54314463Z",
       "tm_delete": null,
       "tm_update": null,
       "trunk_name": null,
       "uuid": "48c93d14-31f8-4445-970d-b865c8a4f432",
       "variables": {
         "plan_key_1": "plean_val_1"
       }
     },
     "statuscode": 200,
     "timestamp": "2017-03-11T12:29:48.70986205Z"
   }


Method: GET
-----------
Get list of all plans.

Call
++++
::

  GET /ob/plans
  
Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {"uuid": "<string>"},
         ...
       ]
     }
   }

Return parameters

* ``list`` : array of items.
   * ``uuid``: plan uuid.

Example
+++++++
::

   curl -X GET 192.168.200.10:8081/ob/plans
  
   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {
           "uuid": "e55e4655-6065-4e9a-bdcf-50363aa6f6d4"
         },
         {
           "uuid": "05540cd8-be5d-4a1a-936c-4f2e439b9126"
          }
        ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-12T15:51:10.303203612Z"
   }
  
/ob/plans/<uuid>
================

Methods
-------
GET : Get specified plan detail info.

PUT : Update specified plan detail info.

DELETE : Delete specified plan

Method: GET
-----------
Get plan detail info.

Call
++++
::

   GET /ob/plans/<plan-uuid>

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
      "uuid": "<string>",
      "name": "<string>",
      "detail": "<string>",
       
      "tech_name": "<string>",
      "trunk_name": "<string>",
   
      "dial_mode": <integer>,
      "dial_timeout": <integer>,
      "dl_end_handle": <integer>,
      "retry_delay": <integer>,
   
      "caller_id": "<string>",
      "service_level": <integer>,
      "early_media": "<string>",
      "codecs": "<string>",
      "variables": {"<string>": "<string>", ...},
       
      "max_retry_cnt_1": <integer>,
      "max_retry_cnt_2": <integer>,
      "max_retry_cnt_3": <integer>,
      "max_retry_cnt_4": <integer>,
      "max_retry_cnt_5": <integer>,
      "max_retry_cnt_6": <integer>,
      "max_retry_cnt_7": <integer>,
      "max_retry_cnt_8": <integer>,

      "in_use": <integer>,
      "tm_create": "<timestamp>",
      "tm_update": "<timestamp>",
      "tm_delete": "<timestamp>"
     }
   }

* ``uuid`` : Plan uuid.
* ``name``: Plan name.
* ``detail``: Detail info.

* ``tech_name``: Tech name for outbound dialing. See detail :ref:`ob_tech_name`.
* ``trunk_name``: Trunkname for outbound dialing. See detail :ref:`trunk_name`.

* ``dial_mode``: Dialling mode. See detail :ref:`ob_dial_mode`.
* ``dial_timeout``: Ringing timeout(ms).
* ``dl_end_handle``: Determine behavior of when the dial list end. See detail :ref:`ob_dial_list_end_handling`.
* ``retry_delay``: Delay time for next try(sec).

* ``caller_id``: Caller's id.
* ``service_level``: Determine service level.
* ``early_media``: Set to true to force call bridge on early media. 
* ``codecs``: Codecs. Comma-separated list of codecs to use for this call.
* ``variables``: variables info json object.

* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/plans/48c93d14-31f8-4445-970d-b865c8a4f432
   
   {
     "api_ver": "0.1",
     "result": {
       "caller_id": null,
       "codecs": null,
       "detail": null,
       "dial_mode": 0,
       "dial_timeout": 30000,
       "dl_end_handle": 1,
       "early_media": null,
       "in_use": 1,
       "max_retry_cnt_1": 5,
       "max_retry_cnt_2": 5,
       "max_retry_cnt_3": 5,
       "max_retry_cnt_4": 5,
       "max_retry_cnt_5": 5,
       "max_retry_cnt_6": 5,
       "max_retry_cnt_7": 5,
       "max_retry_cnt_8": 5,
       "name": "Test plan create",
       "retry_delay": 60,
       "service_level": 0,
       "tech_name": "SIP",
       "tm_create": "2017-03-11T12:29:48.54314463Z",
       "tm_delete": null,
       "tm_update": null,
       "trunk_name": null,
       "uuid": "48c93d14-31f8-4445-970d-b865c8a4f432",
       "variables": {
         "plan_key_1": "plean_val_1"
       }
     },
     "statuscode": 200,
     "timestamp": "2017-03-11T12:34:09.643603005Z"
   }
  
Method: PUT
-----------
Get plan detail info.

Call
++++
Update specified plan info.

::

   PUT /ob/plans/<plan-uuid>
   
   {
     "name": "<string>",
     "detail": "<string>",
    
     "tech_name": "<string>",
     "trunk_name": "<string>",

     "dial_mode": <integer>,
     "dial_timeout": <integer>,
     "dl_end_handle": <integer>,
     "retry_delay": <integer>,

     "caller_id": "<string>",
     "service_level": <integer>,
     "early_media": "<string>",
     "codecs": "<string>",
     "variables": {"<string>": "<string>", ...},
    
     "max_retry_cnt_1": <integer>,
     "max_retry_cnt_2": <integer>,
     "max_retry_cnt_3": <integer>,
     "max_retry_cnt_4": <integer>,
     "max_retry_cnt_5": <integer>,
     "max_retry_cnt_6": <integer>,
     "max_retry_cnt_7": <integer>,
     "max_retry_cnt_8": <integer>  
   }

* ``uuid`` : Plan uuid.
* ``name``: Plan name.
* ``detail``: Detail info.

* ``tech_name``: Tech name for outbound dialing. See detail :ref:`ob_tech_name`.
* ``trunk_name``: Trunkname for outbound dialing. See detail :ref:`trunk_name`.

* ``dial_mode``: Dialling mode. See detail :ref:`ob_dial_mode`.
* ``dial_timeout``: Ringing timeout(ms).
* ``dl_end_handle``: Determine behavior of when the dial list end. See detail :ref:`ob_dial_list_end_handling`.
* ``retry_delay``: Delay time for next try(sec).

* ``caller_id``: Caller's id.
* ``service_level``: Determine service level.
* ``early_media``: Set to true to force call bridge on early media. 
* ``codecs``: Codecs. Comma-separated list of codecs to use for this call.
* ``variables``: variables info json object.

* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.

Returns
+++++++
Updated plan info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "tech_name": "<string>",
       "trunk_name": "<string>",
   
       "dial_mode": <integer>,
       "dial_timeout": <integer>,
       "dl_end_handle": <integer>,
       "retry_delay": <integer>,
   
       "caller_id": "<string>",
       "service_level": <integer>,
       "early_media": "<string>",
       "codecs": "<string>",
       "variables": {"<string>": "<string>", ...},
       
       "max_retry_cnt_1": <integer>,
       "max_retry_cnt_2": <integer>,
       "max_retry_cnt_3": <integer>,
       "max_retry_cnt_4": <integer>,
       "max_retry_cnt_5": <integer>,
       "max_retry_cnt_6": <integer>,
       "max_retry_cnt_7": <integer>,
       "max_retry_cnt_8": <integer>,

       "in_use": <integer>,
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

* ``uuid`` : Plan uuid.
* ``name``: Plan name.
* ``detail``: Detail info.

* ``tech_name``: Tech name for outbound dialing. See detail :ref:`ob_tech_name`.
* ``trunk_name``: Trunkname for outbound dialing. See detail :ref:`trunk_name`.

* ``dial_mode``: Dialling mode. See detail :ref:`ob_dial_mode`.
* ``dial_timeout``: Ringing timeout(ms).
* ``dl_end_handle``: Determine behavior of when the dial list end. See detail :ref:`ob_dial_list_end_handling`.
* ``retry_delay``: Delay time for next try(sec).

* ``caller_id``: Caller's id.
* ``service_level``: Determine service level.
* ``early_media``: Set to true to force call bridge on early media. 
* ``codecs``: Codecs. Comma-separated list of codecs to use for this call.
* ``variables``: variables info json object.

* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.

Example
+++++++
::

   $ curl -X PUT 192.168.200.10:8081/ob/plans/48c93d14-31f8-4445-970d-b865c8a4f432 -d \
   '{"name": "update plan name"}'
   
   {
     "api_ver": "0.1",
     "result": {
       "caller_id": null,
       "codecs": null,
       "detail": null,
       "dial_mode": 0,
       "dial_timeout": 30000,
       "dl_end_handle": 1,
       "early_media": null,
       "in_use": 1,
       "max_retry_cnt_1": 5,
       "max_retry_cnt_2": 5,
       "max_retry_cnt_3": 5,
       "max_retry_cnt_4": 5,
       "max_retry_cnt_5": 5,
       "max_retry_cnt_6": 5,
       "max_retry_cnt_7": 5,
       "max_retry_cnt_8": 5,
       "name": "update plan name",
       "retry_delay": 60,
       "service_level": 0,
       "tech_name": "SIP",
       "tm_create": "2017-03-11T12:29:48.54314463Z",
       "tm_delete": null,
       "tm_update": "2017-03-11T12:39:35.385525099Z",
       "trunk_name": null,
       "uuid": "48c93d14-31f8-4445-970d-b865c8a4f432",
       "variables": {
         "plan_key_1": "plean_val_1"
       }
     },
     "statuscode": 200,
     "timestamp": "2017-03-11T12:39:35.404466531Z"
   }

Method: DELETE
--------------
Delete specified plan info.

Call
++++
Delete specified plan info.

::

   DELETE /ob/plans/<plan-uuid>

Returns
+++++++
Deleted plan info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "tech_name": "<string>",
       "trunk_name": "<string>",
   
       "dial_mode": <integer>,
       "dial_timeout": <integer>,
       "dl_end_handle": <integer>,
       "retry_delay": <integer>,
   
       "caller_id": "<string>",
       "service_level": <integer>,
       "early_media": "<string>",
       "codecs": "<string>",
       "variables": {"<string>": "<string>", ...},
       
       "max_retry_cnt_1": <integer>,
       "max_retry_cnt_2": <integer>,
       "max_retry_cnt_3": <integer>,
       "max_retry_cnt_4": <integer>,
       "max_retry_cnt_5": <integer>,
       "max_retry_cnt_6": <integer>,
       "max_retry_cnt_7": <integer>,
       "max_retry_cnt_8": <integer>,

       "in_use": <integer>,
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

   
Example
+++++++
::

   curl -X DELETE 192.168.200.10:8081/ob/plans/c8b521ab-8114-4a62-b4e9-488d770d4ee8
   
   {
     "api_ver": "0.1",
     "result": {
       "caller_id": null,
       "codecs": null,
       "detail": null,
       "dial_mode": 0,
       "dial_timeout": 30000,
       "dl_end_handle": 1,
       "early_media": null,
       "in_use": 0,
       "max_retry_cnt_1": 5,
       "max_retry_cnt_2": 5,
       "max_retry_cnt_3": 5,
       "max_retry_cnt_4": 5,
       "max_retry_cnt_5": 5,
       "max_retry_cnt_6": 5,
       "max_retry_cnt_7": 5,
       "max_retry_cnt_8": 5,
       "name": "update plan name",
       "retry_delay": 60,
       "service_level": 0,
       "tech_name": "SIP",
       "tm_create": "2017-03-11T12:29:48.54314463Z",
       "tm_delete": "2017-03-11T12:45:37.751824868Z",
       "tm_update": "2017-03-11T12:39:35.385525099Z",
       "trunk_name": null,
       "uuid": "48c93d14-31f8-4445-970d-b865c8a4f432",
       "variables": {
         "plan_key_1": "plean_val_1"
       }
     },
     "statuscode": 200,
     "timestamp": "2017-03-11T12:45:37.805729832Z"
   }

/ob/destinations
================

Methods
-------
POST : Create new destination.

GET : Get list of all destinations.

Method: POST
------------
Create new destination.

Call
++++
::

   POST /ob/destinations
   
   {
     "name": "<string>",
     "detail": "<string>",
     
     "type": <integer>,
     
     "application": "<string>",
     "data": "<string>",
     
     "context": "<string>",
     "exten": "<string>",
     "priority": "<string>",
     
     "variables": {"<string>": "<string>", ...}
   }

Data parameters

* ``name``: Destination name.
* ``detail``: Detail info.

* ``type``: Destination type. See detail :ref:`ob_destination_type`.

* ``application``: Application name. Type: 1(application) only
* ``data``: Application data. Type: 1(application) only

* ``context``: Conetxt. Type: 0(exten) only
* ``exten``: Extension. Type: 0(exten) only
* ``priority``: Priority. Type: 0(exten) only

* ``variables``: variables info json object.

Returns
+++++++
Created destination info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "type": <integer>,
       
       "application": "<string>",
       "data": "<string>",
       
       "context": "<string>",
       "exten": "<string>",
       "priority": "<string>",
       
       "variables": "<string>"
       
       "in_use": 1,
       "tm_create": "<timestamp>",
       "tm_delete": "<timestamp>",
       "tm_update": "<timestamp>"
     }
   }

Result parameters

* ``uuid``: Created destination uuid.
* ``name``: Destination name.
* ``detail``: Detail info.

* ``type``: Destination type. See detail :ref:`ob_destination_type`.

* ``application``: Application name. Type: 1(application) only
* ``data``: Application data. Type: 1(application) only

* ``context``: Conetxt. Type: 0(exten) only
* ``exten``: Extension. Type: 0(exten) only
* ``priority``: Priority. Type: 0(exten) only

* ``variables``: variables info json object.

Example
+++++++
::

   $ curl -X POST 192.168.200.10:8081/ob/destinations -d'{}'
 
   {
     "api_ver": "0.1",
     "statuscode": 200,
     "timestamp": "2017-03-05T08:48:43.789657952Z",
     "result": {
       "application": null,
       "context": null,
       "data": null,
       "detail": null,
       "exten": null,
       "in_use": 1,
       "name": null,
       "priority": null,
       "tm_create": "2017-03-05T08:48:43.745658496Z",
       "tm_delete": null,
       "tm_update": null,
       "type": 0,
       "uuid": "5ff9611c-136b-47d7-b8d4-0bd6f0d5b037",
       "variables": null
     }
   }

Method: GET
-----------
Get list of all destinations.

Call
++++
::

  GET /ob/destinations
  
Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {"uuid": "<string>"},
         ...
       ]
     }
   }

Return parameters

* ``list`` : array of items.
   * ``uuid``" destination uuid.

Example
+++++++
::

   curl -X GET 192.168.200.10:8081/ob/destinations
  
   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {
           "uuid": "78c8cfb5-2ca3-471f-a843-2a287842fef4"
         }
       ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-12T15:52:21.86883360Z"
   }

/ob/destinations/<uuid>
=======================

Methods
-------
GET : Get specified destination detail info.

PUT : Update specified destination detail info.

DELETE : Delete specified destination

Method: GET
-----------
Get specified destination detail info.

Call
++++
::

  GET /ob/destinations/<uuid>

Returns
+++++++
Specified destination detail info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "type": <integer>,
       
       "application": "<string>",
       "data": "<string>",
       
       "context": "<string>",
       "exten": "<string>",
       "priority": "<string>",
       
       "variables": "<string>"
       
       "in_use": 1,
       "tm_create": "<timestamp>",
       "tm_delete": "<timestamp>",
       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: Destination uuid.
* ``name``: Destination name.
* ``detail``: Detail info.

* ``type``: Destination type. See detail :ref:`ob_destination_type`.

* ``application``: Application name. Type: 1(application) only
* ``data``: Application data. Type: 1(application) only

* ``context``: Conetxt. Type: 0(exten) only
* ``exten``: Extension. Type: 0(exten) only
* ``priority``: Priority. Type: 0(exten) only

* ``variables``: variables info json object.

Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/ob/destinations/5ff9611c-136b-47d7-b8d4-0bd6f0d5b037

  {
    "api_ver": "0.1",
    "result": {
      "application": null,
      "context": null,
      "data": null,
      "detail": null,
      "exten": null,
      "in_use": 1,
      "name": null,
      "priority": null,
      "tm_create": "2017-03-05T08:48:43.745658496Z",
      "tm_delete": null,
      "tm_update": null,
      "type": 0,
      "uuid": "5ff9611c-136b-47d7-b8d4-0bd6f0d5b037",
      "variables": null
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T09:04:23.954636454Z"
  }

Method: PUT
-----------
Update specified destination detail info.

Call
++++
::
   
   PUT /ob/destinations/<uuid>
   
   {
     "name": "<string>",
     "detail": "<string>",
     
     "type": <integer>,
     
     "application": "<string>",
     "data": "<string>",
     
     "context": "<string>",
     "exten": "<string>",
     "priority": "<string>",
     
     "variables": "<string>"
   }

Method parameters

* ``uuid``: Destination uuid.

Data parameters

* ``name``: Destination name.
* ``detail``: Detail info.

* ``type``: Destination type. See detail :ref:`ob_destination_type`.

* ``application``: Application name. Type: 1(application) only
* ``data``: Application data. Type: 1(application) only

* ``context``: Conetxt. Type: 0(exten) only
* ``exten``: Extension. Type: 0(exten) only
* ``priority``: Priority. Type: 0(exten) only

* ``variables``: variables info json object.

Returns
+++++++
Return the updated destination detail info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "type": <integer>,
       
       "application": "<string>",
       "data": "<string>",
       
       "context": "<string>",
       "exten": "<string>",
       "priority": "<string>",
       
       "variables": {}
       
       "in_use": 1,
       "tm_create": "<timestamp>",
       "tm_delete": "<timestamp>",
       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: Destination uuid.
* ``name``: Destination name.
* ``detail``: Detail info.

* ``type``: Destination type. See detail :ref:`ob_destination_type`.

* ``application``: Application name. Type: 1(application) only
* ``data``: Application data. Type: 1(application) only

* ``context``: Conetxt. Type: 0(exten) only
* ``exten``: Extension. Type: 0(exten) only
* ``priority``: Priority. Type: 0(exten) only

* ``variables``: variables info json object.


Example
+++++++
::
  
   $ curl -X PUT 192.168.200.10:8081/ob/destinations/5ff9611c-136b-47d7-b8d4-0bd6f0d5b037 \
     -d '{"name":"new updated name"}'

   {
     "api_ver": "0.1",
     "result": {
       "application": null,
       "context": null,
       "data": null,
       "detail": null,
       "exten": null,
       "in_use": 1,
       "name": "new updated name",
       "priority": null,
       "tm_create": "2017-03-05T08:48:43.745658496Z",
       "tm_delete": null,
       "tm_update": "2017-03-05T09:31:57.663960574Z",
       "type": 0,
       "uuid": "5ff9611c-136b-47d7-b8d4-0bd6f0d5b037",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-05T09:31:57.884592113Z"
   }

Method: DELETE
--------------
Delete specified destination info.

Call
++++
::

  DELETE /ob/destinations/<uuid>  

Method parameters

* ``uuid``: Destination uuid.

Returns
+++++++
Return the deleted destination detail info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "type": <integer>,
       
       "application": "<string>",
       "data": "<string>",
       
       "context": "<string>",
       "exten": "<string>",
       "priority": "<string>",
       
       "variables": {}
       
       "in_use": 0,
       "tm_create": "<timestamp>",
       "tm_delete": "<timestamp>",
       "tm_update": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: Destination uuid.
* ``name``: Destination name.
* ``detail``: Detail info.

* ``type``: Destination type. See detail :ref:`ob_destination_type`.

* ``application``: Application name. Type: 1(application) only
* ``data``: Application data. Type: 1(application) only

* ``context``: Conetxt. Type: 0(exten) only
* ``exten``: Extension. Type: 0(exten) only
* ``priority``: Priority. Type: 0(exten) only

* ``variables``: variables info json object.

Example
+++++++
::

  $ curl -X DELETE 192.168.200.10:8081/ob/destinations/38dd5d48-4758-4194-9b7d-24acada05e08

  {
    "api_ver": "0.1",
    "result": {
      "application": null,
      "context": null,
      "data": null,
      "detail": null,
      "exten": null,
      "in_use": 0,
      "name": null,
      "priority": null,
      "tm_create": "2017-03-05T09:40:44.52708509Z",
      "tm_delete": "2017-03-05T09:40:57.912346237Z",
      "tm_update": null,
      "type": 0,
      "uuid": "38dd5d48-4758-4194-9b7d-24acada05e08",
      "variables": null
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T09:40:57.954045594Z"
  }

/ob/dlmas
=========

Methods
-------
POST : Create new dlma.

GET : Get list of all dlmas.

Method: POST
------------
Create new dlma

Call
++++
::

   POST /ob/dlmas
   
   {
     "name": "<string>",
     "detail": "<string>",
     "variables": {}
   }

Data parameters
   
* ``name``: Dlma name.
* ``detail``: Detail dlma info.
* ``variables``: variables info json object.

Returns
+++++++
Return the created campaign info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "dl_table": "<string>",
       
       "variables": {},
       
       "in_use": 1,
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: Created dlma uuid.
* ``name``: dlma name.
* ``detail``: dlma detail info.

* ``dl_table``: dlma reference table.

* ``variables``: variables info json object.

Example
+++++++
::

   $ curl -X POST 192.168.200.10:8081/ob/dlmas -d '{"name": "test dlma"}'
   
   {
     "api_ver": "0.1",
     "timestamp": "2017-03-08T06:02:06.328854094Z",
     "statuscode": 200,
     "result": {
       "tm_delete": null,
       "uuid": "cd60aff5-3355-488c-bf6a-d42f8b67a006",
       "name": "test dlma",
       "tm_update": null,
       "detail": null,
       "dl_table": "cd60aff5_3355_488c_bf6a_d42f8b67a006",
       "variables": null,
       "tm_create": "2017-03-08T06:02:06.313020031Z",
       "in_use": 1
     }
   }

Method: GET
-----------
Get list of all dlmas.

Call
++++
::

   GET /ob/dlmas
   
Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {"uuid": "<string>"},
         ...
       ]
     }
   }

Return parameters
  
* ``list`` : array of items.
   * ``uuid``: dlma uuid.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/dlmas

   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {
           "uuid": "26f91049-469d-4437-8a42-0e00c7f4eaa3"
         }
       ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-12T15:55:00.283719620Z"
   }

/ob/dlmas/<uuid>
================

Methods
-------
GET : Get specified dlma detail info.

PUT : Update specified dlma detail info.

DELETE : Delete specified dlma.

Method: GET
-----------
Get specified dlma detail info.

Call
++++
::

   GET /ob/dlmas/<uuid>

Method parameters

* ``uuid`` : dlma uuid.
   
Returns
+++++++
Get specified campaign detail info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "dl_table": "<string>",
       
       "variables": {},
       
       "in_use": 1,
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: dlma uuid.
* ``name``: dlma name.
* ``detail``: dlma detail info.

* ``dl_table``: dlma reference table.

* ``variables``: variables info json object.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/dlmas/6526c474-280e-4652-a1bf-731089c981a2

   {
     "api_ver": "0.1",
     "result": {
       "detail": null,
       "dl_table": "6526c474_280e_4652_a1bf_731089c981a2",
       "in_use": 1,
       "name": null,
       "tm_create": "2017-02-08T00:56:35.486586398Z",
       "tm_delete": null,
       "tm_update": null,
       "uuid": "6526c474-280e-4652-a1bf-731089c981a2",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-08T07:29:53.471067319Z"
   }
   
Method: PUT
-----------
Update specified dlma detail info.

Call
++++
::

   PUT /ob/dlmas/<uuid>
   
   {
     "name": "<string>",
     "detail": "<string>",
     "variables": "<string>"
   }

Method parameters

* ``uuid`` : dlma uuid.

Data parameters

* ``name``: Update dlma name
* ``detail``: Update dlma detail info.
* ``variables``: variables info json object.

Returns
+++++++
Updated dlma info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "dl_table": "<string>",
       
       "variables": {},
       
       "in_use": 1,
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

Return parameters
   
* ``uuid``: dlma uuid.
* ``name``: dlma name.
* ``detail``: dlma detail info.

* ``dl_table``: dlma reference table.

* ``variables``: variables info json object.

Example
+++++++
::

   $ curl -X PUT 192.168.200.10:8081/ob/dlmas/6526c474-280e-4652-a1bf-731089c981a2 -d 
   '{"detail": "update dlma detail info."}'
   
   {
     "api_ver": "0.1",
     "result": {
       "detail": "update dlma detail info.",
       "dl_table": "6526c474_280e_4652_a1bf_731089c981a2",
       "in_use": 1,
       "name": null,
       "tm_create": "2017-02-08T00:56:35.486586398Z",
       "tm_delete": null,
       "tm_update": "2017-03-08T07:38:20.478261875Z",
       "uuid": "6526c474-280e-4652-a1bf-731089c981a2",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-08T07:38:20.586105308Z"
   }
   
Method: DELETE
--------------
Delete specified dlma.

If there's available dl(dial list), error responsed.

Call
++++
::

   DELETE /ob/dlmas/<uuid>?force=<force_option>

Method parameters

* ``uuid`` : dlma uuid.
* ``force``: <optional> If sets to 1, delete dlma and all the dl(dial list) related with given dlma. default=0

Returns
+++++++
Deleted dlma info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       
       "dl_table": "<string>",
       
       "variables": {},
       
       "in_use": 0,
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: dlma uuid.
* ``name``: dlma name.
* ``detail``: dlma detail info.
* ``dl_table``: dlma reference table.
* ``variables``: variables info json object.

Example
+++++++
::

   $ curl -X DELETE 192.168.200.10:8081/ob/dlmas/6526c474-280e-4652-a1bf-731089c981a2
   
   {
     "api_ver": "0.1",
     "result": {
       "detail": "update dlma detail info.",
       "dl_table": "6526c474_280e_4652_a1bf_731089c981a2",
       "in_use": 0,
       "name": null,
       "tm_create": "2017-02-08T00:56:35.486586398Z",
       "tm_delete": "2017-03-08T07:43:25.381266416Z",
       "tm_update": "2017-03-08T07:38:20.478261875Z",
       "uuid": "6526c474-280e-4652-a1bf-731089c981a2",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-08T07:43:25.398930595Z"
   }

/ob/dls
=======

Methods
-------
POST : Create new dial list.

GET : Get list of dial list.

Method: POST
------------
Create new dial list.

Call
++++
::

   POST /ob/dls
   
   {
     "name": "<string>",
     "detail": "<string>",
     "status": <integer>,
     "dlma_uuid": "<string>",

     "email": "<string>",

     "number_1": "<string>",
     "number_2": "<string>",
     "number_3": "<string>",
     "number_4": "<string>",
     "number_5": "<string>",
     "number_6": "<string>",
     "number_7": "<string>",
     "number_8": "<string>",
     
     "resv_target": "<string>",
     "ukey": "<string>",
     "variables": {}
   }

Data details

* ``name``: dial list name.
* ``detail``: dial list detail.
* ``status``: Dial list status. See detail :ref:`ob_dial_list_status`.
* ``dlma_uuid``: dlma uuid.

* ``email``: Email address

* ``number_1``: The contact number 1
* ``number_2``: The contact number 2
* ``number_3``: The contact number 3
* ``number_4``: The contact number 4
* ``number_5``: The contact number 5
* ``number_6``: The contact number 6
* ``number_7``: The contact number 7
* ``number_8``: The contact number 8

* ``resv_target``: Reserved target. Used for reserved dial.
* ``ukey``: User defined key.
* ``variables``: variables info json object.

Returns
+++++++
Returns created dial list info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
       "dlma_uuid": "<string>",
       
       "email": "<string>",

       "number_1": "<string>",
       "number_2": "<string>",
       "number_3": "<string>",
       "number_4": "<string>",
       "number_5": "<string>",
       "number_6": "<string>",
       "number_7": "<string>",
       "number_8": "<string>",

       "trycnt_1": <integer>,
       "trycnt_2": <integer>,
       "trycnt_3": <integer>,
       "trycnt_4": <integer>,
       "trycnt_5": <integer>,
       "trycnt_6": <integer>,
       "trycnt_7": <integer>,
       "trycnt_8": <integer>,

       "dialing_camp_uuid": "<string>",
       "dialing_plan_uuid": "<string>",
       "dialing_uuid": "<string>",
       
       "res_dial": <integer>,
       "res_dial_detail":"<string>",
       "res_hangup": <integer>,
       "res_hangup_detail": "<string>",

       "tm_last_dial": "<timestamp>",
       "tm_last_hangup": "<timestamp>",

       "resv_target": "<string>",
       
       "ukey": "<string>",
       "variables": {},

       "in_use": 1,

       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

* ``uuid``: dial list uuid.
* ``name``: dial list name.
* ``detail``: dial list detail.
* ``status``: Dial list status. See detail :ref:`ob_dial_list_status`.
* ``dlma_uuid``: <required> The dlma uuid info if this dial list is dialing.

* ``email``: Email address

* ``number_1``: The contact number 1
* ``number_2``: The contact number 2
* ``number_3``: The contact number 3
* ``number_4``: The contact number 4
* ``number_5``: The contact number 5
* ``number_6``: The contact number 6
* ``number_7``: The contact number 7
* ``number_8``: The contact number 8

* ``trycnt_1``: The dialing try count of number 1
* ``trycnt_2``: The dialing try count of number 2
* ``trycnt_3``: The dialing try count of number 3
* ``trycnt_4``: The dialing try count of number 4
* ``trycnt_5``: The dialing try count of number 5
* ``trycnt_6``: The dialing try count of number 6
* ``trycnt_7``: The dialing try count of number 7
* ``trycnt_8``: The dialing try count of number 8

* ``dialing_camp_uuid``: The campaign uuid info if this dial list is dialing.
* ``dialing_plan_uuid``: The plan uuid info if this dial list is dialing.
* ``dialing_uuid``: The dialing uuid info if this dial list is dialing.

* ``res_dial``: The last dialing dial result. See detail :ref:`ob_dial_result`.
* ``res_dial_detail``: The last dialing dial result detail.
* ``res_hangup``: The last dialing hangup result. See detail :ref:`ob_hangup_result`.
* ``res_hangup_detail``: The last dialing hangup result detail.

* ``tm_last_dial``: Timestamp of the last dialing.
* ``tm_last_hangup``: Timestamp of the last hangup.

* ``in_use``: Use flag. See detail :ref:`ob_use_flag`.
* ``resv_target``: Reserved target. Used for reserved dial.
* ``ukey``: 
* ``variables``: variables info json object.

* ``tm_create``: 
* ``tm_delete``: 
* ``tm_update``: 

Example
+++++++
::

   $ curl -X POST 192.168.200.10:8081/ob/dls -d 
   '{"dlma_uuid":"42b72a18-a6c5-43bf-b9aa-6043ff32128d", "name": "sample dial list", \
   "detail": "dial list sample detail", "number_1": "300"}'
   
   {
     "api_ver": "0.1",
     "result": {
       "detail": "dial list sample detail",
       "dialing_camp_uuid": null,
       "dialing_plan_uuid": null,
       "dialing_uuid": null,
       "dlma_uuid": "42b72a18-a6c5-43bf-b9aa-6043ff32128d",
       "email": null,
       "in_use": 1,
       "name": "sample dial list",
       "number_1": "300",
       "number_2": null,
       "number_3": null,
       "number_4": null,
       "number_5": null,
       "number_6": null,
       "number_7": null,
       "number_8": null,
       "res_dial": 0,
       "res_dial_detail": null,
       "res_hangup": 0,
       "res_hangup_detail": null,
       "resv_target": null,
       "status": 0,
       "tm_create": "2017-03-08T07:50:59.525293176Z",
       "tm_delete": null,
       "tm_last_dial": null,
       "tm_last_hangup": null,
       "tm_update": null,
       "trycnt_1": 0,
       "trycnt_2": 0,
       "trycnt_3": 0,
       "trycnt_4": 0,
       "trycnt_5": 0,
       "trycnt_6": 0,
       "trycnt_7": 0,
       "trycnt_8": 0,
       "ukey": null,
       "uuid": "ef3281f6-b18c-4bf5-9e9d-072bb6036717",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-08T07:50:59.604472784Z"
   }
   
Method: GET
-----------
Get list of dial list.

Call
++++
::

   GET /ob/dls?dlma_uuid=<dlam-uuid>&count=<request list count>

Parameter details

* ``dlma_uuid`` : dial list master uuid.
* ``count`` : Request list count. Default 100.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {"uuid": "<string>"},
         ...
       ]
     }
   }

Return parameters

* ``list``: Array of items.
   * ``uuid``: dl uuid.   


Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/dls?dlma_uuid=42b72a18-a6c5-43bf-b9aa-6043ff32128d
   
   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {
           "uuid": "26f91049-469d-4437-8a42-0e00c7f4eaa3"
          }
        ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-12T15:55:00.283719620Z"
   }

/ob/dls/<uuid>
==============

Methods
-------
GET : Get specified dial list detail info.

PUT : Update specified dial list detail info.

DELETE : Delete specified dial list.

Method: GET
-----------
Get specified dial list detail info.

Call
++++
::

   GET /ob/dls/<dl-uuid>
   
Returns
+++++++
Returns specified dial list detail info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
       "dlma_uuid": "<string>",
       
       "email": "<string>",

       "number_1": "<string>",
       "number_2": "<string>",
       "number_3": "<string>",
       "number_4": "<string>",
       "number_5": "<string>",
       "number_6": "<string>",
       "number_7": "<string>",
       "number_8": "<string>",

       "trycnt_1": <integer>,
       "trycnt_2": <integer>,
       "trycnt_3": <integer>,
       "trycnt_4": <integer>,
       "trycnt_5": <integer>,
       "trycnt_6": <integer>,
       "trycnt_7": <integer>,
       "trycnt_8": <integer>,

       "dialing_camp_uuid": "<string>",
       "dialing_plan_uuid": "<string>",
       "dialing_uuid": "<string>",
       
       "res_dial": <integer>,
       "res_dial_detail":"<string>",
       "res_hangup": <integer>,
       "res_hangup_detail": "<string>",

       "tm_last_dial": "<timestamp>",
       "tm_last_hangup": "<timestamp>",

       "resv_target": "<string>",
       
       "ukey": "<string>",
       "variables": {},

       "in_use": 1,

       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

* ``uuid``: dial list uuid.
* ``name``: dial list name.
* ``detail``: dial list detail.
* ``status``: Dial list status. See detail :ref:`ob_dial_list_status`.
* ``dlma_uuid``: The dlma uuid info if this dial list is dialing.

* ``email``: Email address

* ``number_1``: The contact number 1
* ``number_2``: The contact number 2
* ``number_3``: The contact number 3
* ``number_4``: The contact number 4
* ``number_5``: The contact number 5
* ``number_6``: The contact number 6
* ``number_7``: The contact number 7
* ``number_8``: The contact number 8

* ``trycnt_1``: The dialing try count of number 1
* ``trycnt_2``: The dialing try count of number 2
* ``trycnt_3``: The dialing try count of number 3
* ``trycnt_4``: The dialing try count of number 4
* ``trycnt_5``: The dialing try count of number 5
* ``trycnt_6``: The dialing try count of number 6
* ``trycnt_7``: The dialing try count of number 7
* ``trycnt_8``: The dialing try count of number 8

* ``dialing_camp_uuid``: The campaign uuid info if this dial list is dialing.
* ``dialing_plan_uuid``: The plan uuid info if this dial list is dialing.
* ``dialing_uuid``: The dialing uuid info if this dial list is dialing.

* ``res_dial``: The last dialing dial result. See detail :ref:`ob_dial_result`.
* ``res_dial_detail``: The last dialing dial result detail.
* ``res_hangup``: The last dialing hangup result. See detail :ref:`ob_hangup_result`.
* ``res_hangup_detail``: The last dialing hangup result detail.

* ``tm_last_dial``: Timestamp of the last dialing.
* ``tm_last_hangup``: Timestamp of the last hangup.

* ``in_use``: Use flag. See detail :ref:`ob_use_flag`.
* ``resv_target``: Reserved target. Used for reserved dial.
* ``ukey``: 
* ``variables``: variables info json object.

* ``tm_create``: 
* ``tm_delete``: 
* ``tm_update``: 

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/dls/7d760dd1-8ba5-48d9-989d-415d610ffe57
   
   {
     "api_ver": "0.1",
     "result": {
       "detail": "test customer for basic tutorial",
       "dialing_camp_uuid": null,
       "dialing_plan_uuid": null,
       "dialing_uuid": null,
       "dlma_uuid": "42b72a18-a6c5-43bf-b9aa-6043ff32128d",
       "email": null,
       "in_use": 1,
       "name": "basic tutorial test customer 1",
       "number_1": "300",
       "number_2": null,
       "number_3": null,
       "number_4": null,
       "number_5": null,
       "number_6": null,
       "number_7": null,
       "number_8": null,
       "res_dial": 4,
       "res_dial_detail": "Remote end has answered",
       "res_hangup": 16,
       "res_hangup_detail": "Normal Clearing",
       "resv_target": null,
       "status": 0,
       "tm_create": "2017-03-08T01:55:07.911271576Z",
       "tm_delete": null,
       "tm_last_dial": "2017-03-08T02:02:19.561972329Z",
       "tm_last_hangup": "2017-03-08T02:02:38.294239075Z",
       "tm_update": null,
       "trycnt_1": 1,
       "trycnt_2": 0,
       "trycnt_3": 0,
       "trycnt_4": 0,
       "trycnt_5": 0,
       "trycnt_6": 0,
       "trycnt_7": 0,
       "trycnt_8": 0,
       "ukey": null,
       "uuid": "7d760dd1-8ba5-48d9-989d-415d610ffe57",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-08T18:46:54.423073902Z"
   }

Method: PUT
-----------
Update specified dial list detail info.


Call
++++
::

   PUT /ob/dls/<dl-uuid>
   
   {
     "name": "<string>",
     "detail": "<string>",
     "status": <integer>,
     "dlma_uuid": "<string>",
     
     "email": "<string>",

     "number_1": "<string>",
     "number_2": "<string>",
     "number_3": "<string>",
     "number_4": "<string>",
     "number_5": "<string>",
     "number_6": "<string>",
     "number_7": "<string>",
     "number_8": "<string>",

     "trycnt_1": <integer>,
     "trycnt_2": <integer>,
     "trycnt_3": <integer>,
     "trycnt_4": <integer>,
     "trycnt_5": <integer>,
     "trycnt_6": <integer>,
     "trycnt_7": <integer>,
     "trycnt_8": <integer>,
     
     "res_dial": <integer>,
     "res_dial_detail":"<string>",
     "res_hangup": <integer>,
     "res_hangup_detail": "<string>",

     "tm_last_dial": "<timestamp>",
     "tm_last_hangup": "<timestamp>",

     "resv_target": "<string>",
     
     "ukey": "<string>",
     "variables": "<string>"
   }

Method parameters

* ``dl-uuid``: dl uuid.

Data parameters

* ``name``: dial list name.
* ``detail``: dial list detail.
* ``status``: Dial list status. See detail :ref:`ob_dial_list_status`.
* ``dlma_uuid``: The dlma uuid info if this dial list is dialing.

* ``email``: Email address

* ``number_1``: The contact number 1
* ``number_2``: The contact number 2
* ``number_3``: The contact number 3
* ``number_4``: The contact number 4
* ``number_5``: The contact number 5
* ``number_6``: The contact number 6
* ``number_7``: The contact number 7
* ``number_8``: The contact number 8

* ``trycnt_1``: The dialing try count of number 1
* ``trycnt_2``: The dialing try count of number 2
* ``trycnt_3``: The dialing try count of number 3
* ``trycnt_4``: The dialing try count of number 4
* ``trycnt_5``: The dialing try count of number 5
* ``trycnt_6``: The dialing try count of number 6
* ``trycnt_7``: The dialing try count of number 7
* ``trycnt_8``: The dialing try count of number 8

* ``dialing_camp_uuid``: The campaign uuid info if this dial list is dialing.
* ``dialing_plan_uuid``: The plan uuid info if this dial list is dialing.
* ``dialing_uuid``: The dialing uuid info if this dial list is dialing.

* ``res_dial``: The last dialing dial result. See detail :ref:`ob_dial_result`.
* ``res_dial_detail``: The last dialing dial result detail.
* ``res_hangup``: The last dialing hangup result. See detail :ref:`ob_hangup_result`.
* ``res_hangup_detail``: The last dialing hangup result detail.

* ``tm_last_dial``: Timestamp of the last dialing.
* ``tm_last_hangup``: Timestamp of the last hangup.

* ``in_use``: Use flag. See detail :ref:`ob_use_flag`.
* ``resv_target``: Reserved target. Used for reserved dial.
* ``ukey``: 
* ``variables``: variables info json object.

Returns
+++++++
Returns updated dial list detail info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
       "dlma_uuid": "<string>",
       
       "email": "<string>",

       "number_1": "<string>",
       "number_2": "<string>",
       "number_3": "<string>",
       "number_4": "<string>",
       "number_5": "<string>",
       "number_6": "<string>",
       "number_7": "<string>",
       "number_8": "<string>",

       "trycnt_1": <integer>,
       "trycnt_2": <integer>,
       "trycnt_3": <integer>,
       "trycnt_4": <integer>,
       "trycnt_5": <integer>,
       "trycnt_6": <integer>,
       "trycnt_7": <integer>,
       "trycnt_8": <integer>,

       "dialing_camp_uuid": "<string>",
       "dialing_plan_uuid": "<string>",
       "dialing_uuid": "<string>",
       
       "res_dial": <integer>,
       "res_dial_detail":"<string>",
       "res_hangup": <integer>,
       "res_hangup_detail": "<string>",

       "tm_last_dial": "<timestamp>",
       "tm_last_hangup": "<timestamp>",

       "resv_target": "<string>",
       
       "ukey": "<string>",
       "variables": {},

       "in_use": 1,

       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: dial list uuid.
* ``name``: dial list name.
* ``detail``: dial list detail.
* ``status``: Dial list status. See detail :ref:`ob_dial_list_status`.
* ``dlma_uuid``: The dlma uuid info if this dial list is dialing.

* ``email``: Email address

* ``number_1``: The contact number 1
* ``number_2``: The contact number 2
* ``number_3``: The contact number 3
* ``number_4``: The contact number 4
* ``number_5``: The contact number 5
* ``number_6``: The contact number 6
* ``number_7``: The contact number 7
* ``number_8``: The contact number 8

* ``trycnt_1``: The dialing try count of number 1
* ``trycnt_2``: The dialing try count of number 2
* ``trycnt_3``: The dialing try count of number 3
* ``trycnt_4``: The dialing try count of number 4
* ``trycnt_5``: The dialing try count of number 5
* ``trycnt_6``: The dialing try count of number 6
* ``trycnt_7``: The dialing try count of number 7
* ``trycnt_8``: The dialing try count of number 8

* ``dialing_camp_uuid``: The campaign uuid info if this dial list is dialing.
* ``dialing_plan_uuid``: The plan uuid info if this dial list is dialing.
* ``dialing_uuid``: The dialing uuid info if this dial list is dialing.

* ``res_dial``: The last dialing dial result. See detail :ref:`ob_dial_result`.
* ``res_dial_detail``: The last dialing dial result detail.
* ``res_hangup``: The last dialing hangup result. See detail :ref:`ob_hangup_result`.
* ``res_hangup_detail``: The last dialing hangup result detail.

* ``tm_last_dial``: Timestamp of the last dialing.
* ``tm_last_hangup``: Timestamp of the last hangup.

* ``in_use``: Use flag. See detail :ref:`ob_use_flag`.
* ``resv_target``: Reserved target. Used for reserved dial.
* ``ukey``: 
* ``variables``: variables info json object.

* ``tm_create``: 
* ``tm_delete``: 
* ``tm_update``: 

Example
+++++++

::

   $ curl -X PUT 192.168.200.10:8081/ob/dls/7d760dd1-8ba5-48d9-989d-415d610ffe57 -d 
   '{"detail": "Updated test customer detail"}'
   
   {
     "api_ver": "0.1",
     "result": {
       "detail": "Updated test customer detail",
       "dialing_camp_uuid": null,
       "dialing_plan_uuid": null,
       "dialing_uuid": null,
       "dlma_uuid": "42b72a18-a6c5-43bf-b9aa-6043ff32128d",
       "email": null,
       "in_use": 1,
       "name": "basic tutorial test customer 1",
       "number_1": "300",
       "number_2": null,
       "number_3": null,
       "number_4": null,
       "number_5": null,
       "number_6": null,
       "number_7": null,
       "number_8": null,
       "res_dial": 4,
       "res_dial_detail": "Remote end has answered",
       "res_hangup": 16,
       "res_hangup_detail": "Normal Clearing",
       "resv_target": null,
       "status": 0,
       "tm_create": "2017-03-08T01:55:07.911271576Z",
       "tm_delete": null,
       "tm_last_dial": "2017-03-08T02:02:19.561972329Z",
       "tm_last_hangup": "2017-03-08T02:02:38.294239075Z",
       "tm_update": null,
       "trycnt_1": 1,
       "trycnt_2": 0,
       "trycnt_3": 0,
       "trycnt_4": 0,
       "trycnt_5": 0,
       "trycnt_6": 0,
       "trycnt_7": 0,
       "trycnt_8": 0,
       "ukey": null,
       "uuid": "7d760dd1-8ba5-48d9-989d-415d610ffe57",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-08T19:18:30.669572995Z"
   }
   
Method: DELETE
--------------
Delete specified dial list.

Call
++++
::

   DELETE /ob/dls/<dl-uuid>

Method parameters

* ``uuid`` : dl uuid.

Returns
+++++++
Returns deleted dial list info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
       "dlma_uuid": "<string>",
       
       "email": "<string>",

       "number_1": "<string>",
       "number_2": "<string>",
       "number_3": "<string>",
       "number_4": "<string>",
       "number_5": "<string>",
       "number_6": "<string>",
       "number_7": "<string>",
       "number_8": "<string>",

       "trycnt_1": <integer>,
       "trycnt_2": <integer>,
       "trycnt_3": <integer>,
       "trycnt_4": <integer>,
       "trycnt_5": <integer>,
       "trycnt_6": <integer>,
       "trycnt_7": <integer>,
       "trycnt_8": <integer>,

       "dialing_camp_uuid": "<string>",
       "dialing_plan_uuid": "<string>",
       "dialing_uuid": "<string>",
       
       "res_dial": <integer>,
       "res_dial_detail":"<string>",
       "res_hangup": <integer>,
       "res_hangup_detail": "<string>",

       "tm_last_dial": "<timestamp>",
       "tm_last_hangup": "<timestamp>",

       "resv_target": "<string>",
       
       "ukey": "<string>",
       "variables": {},

       "in_use": 1,

       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

Return parameters

* ``uuid``: dial list uuid.
* ``name``: dial list name.
* ``detail``: dial list detail.
* ``status``: Dial list status. See detail :ref:`ob_dial_list_status`.
* ``dlma_uuid``: The dlma uuid info if this dial list is dialing.

* ``email``: Email address

* ``number_1``: The contact number 1
* ``number_2``: The contact number 2
* ``number_3``: The contact number 3
* ``number_4``: The contact number 4
* ``number_5``: The contact number 5
* ``number_6``: The contact number 6
* ``number_7``: The contact number 7
* ``number_8``: The contact number 8

* ``trycnt_1``: The dialing try count of number 1
* ``trycnt_2``: The dialing try count of number 2
* ``trycnt_3``: The dialing try count of number 3
* ``trycnt_4``: The dialing try count of number 4
* ``trycnt_5``: The dialing try count of number 5
* ``trycnt_6``: The dialing try count of number 6
* ``trycnt_7``: The dialing try count of number 7
* ``trycnt_8``: The dialing try count of number 8

* ``dialing_camp_uuid``: The campaign uuid info if this dial list is dialing.
* ``dialing_plan_uuid``: The plan uuid info if this dial list is dialing.
* ``dialing_uuid``: The dialing uuid info if this dial list is dialing.

* ``res_dial``: The last dialing dial result. See detail :ref:`ob_dial_result`.
* ``res_dial_detail``: The last dialing dial result detail.
* ``res_hangup``: The last dialing hangup result. See detail :ref:`ob_hangup_result`.
* ``res_hangup_detail``: The last dialing hangup result detail.

* ``tm_last_dial``: Timestamp of the last dialing.
* ``tm_last_hangup``: Timestamp of the last hangup.

* ``in_use``: Use flag. See detail :ref:`ob_use_flag`.
* ``resv_target``: Reserved target. Used for reserved dial.
* ``ukey``: 
* ``variables``: variables info json object.

* ``tm_create``: 
* ``tm_delete``: 
* ``tm_update``: 

Example
+++++++

::

   $ curl -X DELETE 192.168.200.10:8081/ob/dls/7d760dd1-8ba5-48d9-989d-415d610ffe57
   
   {
     "api_ver": "0.1",
     "result": {
       "detail": "Updated test customer detail",
       "dialing_camp_uuid": null,
       "dialing_plan_uuid": null,
       "dialing_uuid": null,
       "dlma_uuid": "42b72a18-a6c5-43bf-b9aa-6043ff32128d",
       "email": null,
       "in_use": 0,
       "name": "basic tutorial test customer 1",
       "number_1": "300",
       "number_2": null,
       "number_3": null,
       "number_4": null,
       "number_5": null,
       "number_6": null,
       "number_7": null,
       "number_8": null,
       "res_dial": 4,
       "res_dial_detail": "Remote end has answered",
       "res_hangup": 16,
       "res_hangup_detail": "Normal Clearing",
       "resv_target": null,
       "status": 0,
       "tm_create": "2017-03-08T01:55:07.911271576Z",
       "tm_delete": "2017-03-08T20:46:09.359647726Z",
       "tm_last_dial": "2017-03-08T02:02:19.561972329Z",
       "tm_last_hangup": "2017-03-08T02:02:38.294239075Z",
       "tm_update": null,
       "trycnt_1": 1,
       "trycnt_2": 0,
       "trycnt_3": 0,
       "trycnt_4": 0,
       "trycnt_5": 0,
       "trycnt_6": 0,
       "trycnt_7": 0,
       "trycnt_8": 0,
       "ukey": null,
       "uuid": "7d760dd1-8ba5-48d9-989d-415d610ffe57",
       "variables": null
     },
     "statuscode": 200,
     "timestamp": "2017-03-08T20:46:09.380969992Z"
   }

/ob/dialings
============

Methods
-------
GET : Get list of all dialings.

Method: GET
-----------
Get list of all dialings.

Call
++++
::

   GET /ob/dialings

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {"uuid": "<string>"},
         ...
       ]
     }
   }

Return parameters

* ``list`` : Array of items.
   * ``uuid``: dialing uuid.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/dialings
   
   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {
           "uuid": "21663592-8171-4fcb-9390-bac51a3280fd"
         }
       ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-12T16:02:24.445429913Z"
   }


/ob/dialings/<uuid>
===================

Methods
-------
GET : Get specified dialing detail info.

DELETE : Send hangup request of specified dialing.

Method: GET
-----------
Get specified dialing detail info.

Call
++++
::

   GET /ob/dialings/<uuid>

Method parameters

* ``uuid``: Dialing uuid.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "action_id": "<string>",
       "channel": "<string>",
       "status": <integer>,

       "res_dial": <integer>,
       "res_dial_detail": "<string>",
       "res_hangup": <integer>,
       "res_hangup_detail": "<string>",
       
       "dial_channel": "<string>",
       "dial_addr": "<string>",
       
       "dial_application": "<string>",
       "dial_data": "<string>",
       
       "dial_context": "<string>",
       "dial_exten": "<string>",
       "dial_priority": "<string>",
       
       "dial_type": <integer>,
       "dial_timeout": <integer>,
       "dial_index": <integer>,
       "dial_trycnt": <integer>,
       "dial_variables": {}

       "uuid_camp": "<string>",
       "uuid_dest": "<string>",
       "uuid_dl_list": "<string>",
       "uuid_dlma": "<string>",
       "uuid_plan": "<string>",
       
       "info_camp": "<string>",
       "info_dest": "<string>",
       "info_dial": "<string>",
       "info_dl_list": "<string>",
       "info_dlma": "<string>",
       "info_plan": "<string>",
       
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

* ``uuid`` : Dialing uuid.
* ``action_id``: Requested originate action id.
* ``channel``: Channel name.
* ``status``: Dialing status. See detail :ref:`ob_dialing_status`.

* ``res_dial``: Result of dial. See detail :ref:`ob_dial_result`.
* ``res_dial_detail``: Detail string of res_dial.
* ``res_hangup``: Result of hangup. See detail :ref:`ob_hangup_result`.
* ``res_hangup_detail``: Detail string of res_hangup.

* ``dial_channel``: Dialed channel. Combination of tech name and dialed number.
* ``dial_addr``: Dialed address(number).
* ``dial_application``: Destination application name.
* ``dial_data``: Destination application data.

* ``dial_context``: Destination context name.
* ``dial_exten``: Destination context extension. 
* ``dial_priority``: Destination context extension's prioirty.

* ``dial_type``: Destination type, See detail :ref:`ob_destination_type`.
* ``dial_timeout``: Dialing timeout.
* ``dial_index``: The index number of customer numbers.
* ``dial_trycnt``: Try count number of current dialing.
* ``dial_variables``: The combination of variables(campaign/plan/destination/dlma/dl)

* ``uuid_camp``: Dialing campign uuid.
* ``uuid_dest``: Dialing destination uuid.
* ``uuid_dl_list``: Dialing dial list uuid.
* ``uuid_dlma``: Dialing dlma uuid.
* ``uuid_plan``: Dialing plan uuid.

* ``info_camp``: The json string dump of campaign info when the dialing has created.
* ``info_dest``: The json string dump of destination info when the dialing has created.
* ``info_dial``: The json string dump of dial info when the dialing has created.
* ``info_dl_list``: The json string dump of dial list info when the dialing has created.
* ``info_dlma``: The json string dump of dlma info when the dialing has created.
* ``info_plan``: The json string dump of plan info when the dialing has created.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/dialings/4d78b20b-99e5-4449-977f-261204e11a26
   
   {
     "api_ver": "0.1",
     "result": {
       "action_id": "0ab20783-50e5-40fc-8a22-c075f901722c",
       "channel": "SIP/300-0000002e",
       "dial_addr": "300",
       "dial_application": "queue",
       "dial_channel": "SIP/300",
       "dial_context": null,
       "dial_data": "sales_1",
       "dial_exten": null,
       "dial_index": 1,
       "dial_priority": null,
       "dial_timeout": 30000,
       "dial_trycnt": 2,
       "dial_type": 1,
       "dial_variables": {
         "campaign key 1": "campaign value 1",
         "destination key 1": "destination value 1",
         "dl key1": "dl val1",
         "dlma_key1": "dlma_val1",
         "plan key 1": "plan value 1"
       },
       "info_camp": {
         "dest": "78c8cfb5-2ca3-471f-a843-2a287842fef4",
         "detail": null,
         "dlma": "26f91049-469d-4437-8a42-0e00c7f4eaa3",
         "in_use": 1,
         "name": "Test campaign 1",
         "next_campaign": null,
         "plan": "e55e4655-6065-4e9a-bdcf-50363aa6f6d4",
         "sc_date_end": null,
         "sc_date_list": null,
         "sc_date_list_except": null,
         "sc_date_start": null,
         "sc_day_list": null,
         "sc_mode": 0,
         "sc_time_end": null,
         "sc_time_start": null,
         "status": 1,
         "tm_create": "2017-03-11T05:24:26.976688716Z",
         "tm_delete": null,
         "tm_update": "2017-03-11T05:52:24.817439382Z",
         "uuid": "ab02b7ef-9c12-4d24-b944-9e853c9ce0f9",
         "variables": {
           "campaign key 1": "campaign value 1"
         }
       },
       "info_dest": {
         "application": "queue",
         "context": null,
         "data": "sales_1",
         "detail": null,
         "exten": null,
         "in_use": 1,
         "name": "Test destination 1",
         "priority": null,
         "tm_create": "2017-03-11T05:28:20.20643710Z",
         "tm_delete": null,
         "tm_update": "2017-03-11T05:45:29.804143894Z",
         "type": 1,
         "uuid": "78c8cfb5-2ca3-471f-a843-2a287842fef4",
         "variables": {
           "destination key 1": "destination value 1"
         }
       },
       "info_dial": {
         "channelid": "4d78b20b-99e5-4449-977f-261204e11a26",
         "dest_variables": "",
         "dial_addr": "300",
         "dial_application": "queue",
         "dial_channel": "SIP/300",
         "dial_data": "sales_1",
         "dial_index": 1,
         "dial_timeout": 30000,
         "dial_trycnt": 2,
         "dial_type": 1,
         "dl_variables": "",
         "otherchannelid": "f0b6b955-6978-4c75-b51f-b231623bd351",
         "plan_variables": "",
         "uuid": "7dc1b243-f6ea-4f45-bda5-dd478dac2fd5",
         "variables": "{}"
       },
       "info_dl_list": {
         "detail": "test customer for normal call distribute",
         "dialing_camp_uuid": null,
         "dialing_plan_uuid": null,
         "dialing_uuid": null,
         "dlma_uuid": "26f91049-469d-4437-8a42-0e00c7f4eaa3",
         "email": null,
         "in_use": 1,
         "name": "normal call distribute test customer 3",
         "number_1": "300",
         "number_2": null,
         "number_3": null,
         "number_4": null,
         "number_5": null,
         "number_6": null,
         "number_7": null,
         "number_8": null,
         "res_dial": 0,
         "res_dial_detail": "",
         "res_hangup": 0,
         "res_hangup_detail": "",
         "resv_target": null,
         "status": 0,
         "tm_create": "2017-03-11T10:09:05.424657580Z",
         "tm_delete": null,
         "tm_last_dial": "2017-03-11T11:38:08.568583019Z",
         "tm_last_hangup": "2017-03-11T11:38:29.151276033Z",
         "tm_update": null,
         "trycnt_1": 1,
         "trycnt_2": 0,
         "trycnt_3": 0,
         "trycnt_4": 0,
         "trycnt_5": 0,
         "trycnt_6": 0,
         "trycnt_7": 0,
         "trycnt_8": 0,
         "ukey": null,
         "uuid": "7dc1b243-f6ea-4f45-bda5-dd478dac2fd5",
         "variables": {
           "dl key1": "dl val1"
         }
       },
       "info_dlma": {
         "detail": null,
         "dl_table": "26f91049_469d_4437_8a42_0e00c7f4eaa3",
         "in_use": 1,
         "name": null,
         "tm_create": "2017-03-11T05:29:26.927951044Z",
         "tm_delete": null,
         "tm_update": null,
         "uuid": "26f91049-469d-4437-8a42-0e00c7f4eaa3",
         "variables": {
           "dlma_key1": "dlma_val1"
         }
       },
       "info_plan": {
         "caller_id": null,
         "codecs": null,
         "detail": null,
         "dial_mode": 1,
         "dial_timeout": 30000,
         "dl_end_handle": 0,
         "early_media": null,
         "in_use": 1,
         "max_retry_cnt_1": 5,
         "max_retry_cnt_2": 5,
         "max_retry_cnt_3": 5,
         "max_retry_cnt_4": 5,
         "max_retry_cnt_5": 5,
         "max_retry_cnt_6": 5,
         "max_retry_cnt_7": 5,
         "max_retry_cnt_8": 5,
         "name": "Test plan 1",
         "retry_delay": 60,
         "service_level": 0,
         "tech_name": "SIP",
         "tm_create": "2017-03-11T05:27:38.778880388Z",
         "tm_delete": null,
         "tm_update": "2017-03-11T05:51:44.923547573Z",
         "trunk_name": null,
         "uuid": "e55e4655-6065-4e9a-bdcf-50363aa6f6d4",
         "variables": {
           "plan key 1": "plan value 1"
         }
       },
       "res_dial": 4,
       "res_dial_detail": "Remote end has answered",
       "res_hangup": 0,
       "res_hangup_detail": null,
       "status": 6,
       "tm_create": "2017-03-11T12:12:17.546144349Z",
       "tm_delete": null,
       "tm_update": "2017-03-11T13:13:02.933694875Z",
       "uuid": "4d78b20b-99e5-4449-977f-261204e11a26",
       "uuid_camp": "ab02b7ef-9c12-4d24-b944-9e853c9ce0f9",
       "uuid_dest": "78c8cfb5-2ca3-471f-a843-2a287842fef4",
       "uuid_dl_list": "7dc1b243-f6ea-4f45-bda5-dd478dac2fd5",
       "uuid_dlma": "26f91049-469d-4437-8a42-0e00c7f4eaa3",
       "uuid_plan": "e55e4655-6065-4e9a-bdcf-50363aa6f6d4"
     },
     "statuscode": 200,
     "timestamp": "2017-03-11T13:13:03.573061013Z"
   }

Method: DELETE
--------------
Send hangup request of specified dialing.

If the request is success, the dialing call will be removed after received hangup event from the Asterisk.

Call
++++
::

   DELETE /ob/dialings/<uuid>

Method parameters

* ``uuid``: Dialing uuid.

Returns
+++++++
::

   {
     $defhdr
   }
   
Example
+++++++
::

   $ curl -X DELETE 192.168.200.10:8081/ob/dialings/16c680fe-0ac7-472f-9e6f-c979db05cac0
   
   {
     "api_ver": "0.1",
     "statuscode": 200,
     "timestamp": "2017-03-09T13:12:18.911505281Z"
   }
   

/ob/campaigns
=============

Methods
-------
POST : Create new campaign.

GET : Get list of all campaigns.

Method: POST
------------
Create new campaign.

Call
++++
::

   POST /ob/campaigns
   
   {
     "name": "<string>",
     "detail": "<string>",
     "status": <integer>,
     
     "plan": "<string>",
     "dlma": "<string>",
     "dest": "<string>",
     
     "variables": {},
     
     "next_campaign": "<string>",
     
     "sc_mode": <integer>,
     "sc_time_start": "<string>",
     "sc_time_end": "<string>",
     "sc_date_start": "<string>",
     "sc_date_end": "<string>",
     "sc_date_list": "<string>",
     "sc_date_list_except": "<string>",
     "sc_day_list": "<string>"
   }

Data parameters

* ``name`` : Campaign name.
* ``detail`` : Campaign detail info.
* ``status`` : Campaign status. See detail :ref:`campaign_status`.

* ``plan`` : Registered plan uuid.
* ``dlma`` : Registered dlma uuid.
* ``dest`` : Registered destination uuid.

* ``variables``: variables info json object.

* ``next_campaign`` : Campaign uuid for next campaign running.

* ``sc_mode`` : Scheduling mode. See detail :ref:`scheduling_mode`.
* ``sc_time_start`` : Campaign scheduling start time. See detail :ref:`scheduling_time`.
* ``sc_time_end`` : Campaign scheduling end time. See detail :ref:`scheduling_time`.
* ``sc_date_start`` : Campaign scheduling start date. See detail :ref:`scheduling_date`.
* ``sc_date_end`` : Campaign scheduling end date. See detail :ref:`scheduling_date`.
* ``sc_date_list`` : Campaign schedling date list. See detail :ref:`scheduling_date_list`.
* ``sc_date_list_except`` : Campaign scheduling except date list. See detail :ref:`scheduling_date_list`.
* ``sc_day_list`` : Campaign scheduling day list. See detail :ref:`scheduling_day_list`.

Returns
+++++++
Return the created campaign info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
     
       "plan": "<string>",
       "dlma": "<string>",
       "dest": "<string>",
     
       "variables": {},
     
       "next_campaign": "<string>",
     
       "sc_mode": <integer>,
       "sc_time_start": "<string>",
       "sc_time_end": "<string>",
       "sc_date_start": "<string>",
       "sc_date_end": "<string>",
       "sc_date_list": "<string>",
       "sc_date_list_except": "<string>",
       "sc_day_list": "<string>"

       "in_use": 1,
       "tm_create": "<string>",
       "tm_delete": "<string>",
       "tm_update": "<string>"
     }
   }

Return parameters

* ``uuid`` : Campaign uuid.
* ``name`` : Campaign name.
* ``detail`` : Campaign detail info.
* ``status`` : Campaign status. See detail :ref:`campaign_status`.

* ``plan`` : Registered plan uuid.
* ``dlma`` : Registered dlma uuid.
* ``dest`` : Registered destination uuid.

* ``variables``: variables info json object.

* ``next_campaign`` : Campaign uuid for next campaign running.

* ``sc_mode`` : Scheduling mode. See detail :ref:`scheduling_mode`.
* ``sc_time_start`` : Campaign scheduling start time. See detail :ref:`scheduling_time`.
* ``sc_time_end`` : Campaign scheduling end time. See detail :ref:`scheduling_time`.
* ``sc_date_start`` : Campaign scheduling start date. See detail :ref:`scheduling_date`.
* ``sc_date_end`` : Campaign scheduling end date. See detail :ref:`scheduling_date`.
* ``sc_date_list`` : Campaign schedling date list. See detail :ref:`scheduling_date_list`.
* ``sc_date_list_except`` : Campaign scheduling except date list. See detail :ref:`scheduling_date_list`.
* ``sc_day_list`` : Campaign scheduling day list. See detail :ref:`scheduling_day_list`.

Example
+++++++
::

   $ curl -X POST 192.168.200.10:8081/ob/campaigns -d'{}'

   {
     "api_ver": "0.1",
     "result": {
       "dest": null,
       "detail": null,
       "dlma": null,
       "in_use": 1,
       "name": null,
       "next_campaign": null,
       "plan": null,
       "variables": null, 
       "sc_date_end": null,
       "sc_date_list": null,
       "sc_date_list_except": null,
       "sc_date_start": null,
       "sc_day_list": null,
       "sc_mode": 0,
       "sc_time_end": null,
       "sc_time_start": null,
       "status": 0,
       "tm_create": "2017-03-05T15:25:09.788596601Z",
       "tm_delete": null,
       "tm_update": null,
       "uuid": "305c96cb-4704-4f33-bcad-6b2e4031d7d4"
     },
     "statuscode": 200,
     "timestamp": "2017-03-05T15:25:09.808247523Z"
   }


Method: GET
-----------
Get list of all campaigns.

Call
++++
::

  GET /ob/campaigns
  
Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         "uuid": "<string>",
         ...
       ]
     }
   }

Return parameters
  
* ``list`` : Array of items.
   * ``uuid``: campaign uuid.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/ob/campaigns

   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {
           "uuid": "ab02b7ef-9c12-4d24-b944-9e853c9ce0f9"
         }
       ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-12T16:03:06.987929616Z"
   }

/ob/campaigns/<uuid>
====================

Methods
-------
GET : Get specified campaign detail info.

PUT : Update specified campaign detail info.

DELETE : Delete specified campaign.

Method: GET
-----------
Get specified campaign detail info.

Call
++++
::

  GET /ob/campaign/<uuid>
  
Returns
+++++++
Get specified campaign detail info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
     
       "plan": "<string>",
       "dlma": "<string>",
       "dest": "<string>",
     
       "variables": {},
     
       "next_campaign": "<string>",
     
       "sc_mode": <integer>,
       "sc_time_start": "<string>",
       "sc_time_end": "<string>",
       "sc_date_start": "<string>",
       "sc_date_end": "<string>",
       "sc_date_list": "<string>",
       "sc_date_list_except": "<string>",
       "sc_day_list": "<string>"

       "in_use": 1,
       "tm_create": "<string>",
       "tm_delete": "<string>",
       "tm_update": "<string>"
     }
   }

Return parameters

* ``uuid`` : Campaign uuid.
* ``name`` : Campaign name.
* ``detail`` : Campaign detail info.
* ``status`` : Campaign status. See detail :ref:`campaign_status`.

* ``plan`` : Registered plan uuid.
* ``dlma`` : Registered dlma uuid.
* ``dest`` : Registered destination uuid.

* ``variables``: variables info json object.

* ``next_campaign`` : Campaign uuid for next campaign running.

* ``sc_mode`` : Scheduling mode. See detail :ref:`scheduling_mode`.
* ``sc_time_start`` : Campaign scheduling start time. See detail :ref:`scheduling_time`.
* ``sc_time_end`` : Campaign scheduling end time. See detail :ref:`scheduling_time`.
* ``sc_date_start`` : Campaign scheduling start date. See detail :ref:`scheduling_date`.
* ``sc_date_end`` : Campaign scheduling end date. See detail :ref:`scheduling_date`.
* ``sc_date_list`` : Campaign schedling date list. See detail :ref:`scheduling_date_list`.
* ``sc_date_list_except`` : Campaign scheduling except date list. See detail :ref:`scheduling_date_list`.
* ``sc_day_list`` : Campaign scheduling day list. See detail :ref:`scheduling_day_list`.


Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/ob/campaigns/9841bc9e-3103-4fea-ab11-54ccfcc8322f

  {
    "api_ver": "0.1",
    "result": {
        "dest": "45122654-5633-4af0-a739-e32eddfbd2ae",
        "detail": null,
        "dlma": "6526c474-280e-4652-a1bf-731089c981a2",
        "in_use": 1,
        "name": "test campaign",
        "next_campaign": null,
        "plan": "552e9808-23bc-40b1-947a-60b0b96581cb",
        "variables": null,
        "sc_date_end": null,
        "sc_date_list": null,
        "sc_date_list_except": null,
        "sc_date_start": null,
        "sc_day_list": null,
        "sc_mode": 0,
        "sc_time_end": null,
        "sc_time_start": null,
        "status": 0,
        "tm_create": "2017-02-07T20:32:59.812399819Z",
        "tm_delete": null,
        "tm_update": "2017-02-22T14:46:49.622227668Z",
        "uuid": "9841bc9e-3103-4fea-ab11-54ccfcc8322f"
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T09:15:41.329887897Z"
  }

Method: PUT
-----------
Update specified campaign detail info.

Call
++++
::

   PUT /ob/campaigns/<campaign-uuid>

   {
     "name": "<string>",
     "detail": "<string>",
     "status": <integer>,
     
     "plan": "<string>",
     "dlma": "<string>",
     "dest": "<string>",
     
     "variables": {},
     
     "next_campaign": "<string>",
     
     "sc_mode": <integer>,
     "sc_time_start": "<string>",
     "sc_time_end": "<string>",
     "sc_date_start": "<string>",
     "sc_date_end": "<string>",
     "sc_date_list": "<string>",
     "sc_date_list_except": "<string>",
     "sc_day_list": "<string>"
   
   }

Method parameters

* ``campaign-uuid``: Campaign uuid.

Data parameters

* ``name`` : Campaign name.
* ``detail`` : Campaign detail info.
* ``status`` : Campaign status. See detail :ref:`campaign_status`.

* ``plan`` : Registered plan uuid.
* ``dlma`` : Registered dlma uuid.
* ``dest`` : Registered destination uuid.

* ``variables``: variables info json object.

* ``next_campaign`` : Campaign uuid for next campaign running.

* ``sc_mode`` : Scheduling mode. See detail :ref:`scheduling_mode`.
* ``sc_time_start`` : Campaign scheduling start time. See detail :ref:`scheduling_time`.
* ``sc_time_end`` : Campaign scheduling end time. See detail :ref:`scheduling_time`.
* ``sc_date_start`` : Campaign scheduling start date. See detail :ref:`scheduling_date`.
* ``sc_date_end`` : Campaign scheduling end date. See detail :ref:`scheduling_date`.
* ``sc_date_list`` : Campaign schedling date list. See detail :ref:`scheduling_date_list`.
* ``sc_date_list_except`` : Campaign scheduling except date list. See detail :ref:`scheduling_date_list`.
* ``sc_day_list`` : Campaign scheduling day list. See detail :ref:`scheduling_day_list`.
  
Returns
+++++++
Updated campaign info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
     
       "plan": "<string>",
       "dlma": "<string>",
       "dest": "<string>",
     
       "variables": {},
     
       "next_campaign": "<string>",
     
       "sc_mode": <integer>,
       "sc_time_start": "<string>",
       "sc_time_end": "<string>",
       "sc_date_start": "<string>",
       "sc_date_end": "<string>",
       "sc_date_list": "<string>",
       "sc_date_list_except": "<string>",
       "sc_day_list": "<string>"

       "in_use": 1,
       "tm_create": "<string>",
       "tm_delete": "<string>",
       "tm_update": "<string>"
     }
   }

Return parameters

* ``uuid`` : Campaign uuid.
* ``name`` : Campaign name.
* ``detail`` : Campaign detail info.
* ``status`` : Campaign status. See detail :ref:`campaign_status`.

* ``plan`` : Registered plan uuid.
* ``dlma`` : Registered dlma uuid.
* ``dest`` : Registered destination uuid.

* ``variables``: variables info json object.

* ``next_campaign`` : Campaign uuid for next campaign running.

* ``sc_mode`` : Scheduling mode. See detail :ref:`scheduling_mode`.
* ``sc_time_start`` : Campaign scheduling start time. See detail :ref:`scheduling_time`.
* ``sc_time_end`` : Campaign scheduling end time. See detail :ref:`scheduling_time`.
* ``sc_date_start`` : Campaign scheduling start date. See detail :ref:`scheduling_date`.
* ``sc_date_end`` : Campaign scheduling end date. See detail :ref:`scheduling_date`.
* ``sc_date_list`` : Campaign schedling date list. See detail :ref:`scheduling_date_list`.
* ``sc_date_list_except`` : Campaign scheduling except date list. See detail :ref:`scheduling_date_list`.
* ``sc_day_list`` : Campaign scheduling day list. See detail :ref:`scheduling_day_list`.


Example
+++++++
::

   $ curl -X PUT 192.168.200.10:8081/ob/campaigns/305c96cb-4704-4f33-bcad-6b2e4031d7d4 
      -d '{"name": "Update campaign info"}'
  
   {
     "api_ver": "0.1",
     "result": {
       "dest": null,
       "detail": null,
       "dlma": null,
       "in_use": 1,
       "name": "Update campaign info",
       "next_campaign": null,
       "plan": null,
       "variables": null,
       "sc_date_end": null,
       "sc_date_list": null,
       "sc_date_list_except": null,
       "sc_date_start": null,
       "sc_day_list": null,
       "sc_mode": 0,
       "sc_time_end": null,
       "sc_time_start": null,
       "status": 0,
       "tm_create": "2017-03-05T15:25:09.788596601Z",
       "tm_delete": null,
       "tm_update": "2017-03-05T15:44:10.295794418Z",
       "uuid": "305c96cb-4704-4f33-bcad-6b2e4031d7d4"
     },
     "statuscode": 200,
     "timestamp": "2017-03-05T15:44:10.338811064Z"
   }

Method: DELETE
--------------
Delete specified campaign info.

Call
++++
Delete specified campaign info.

::

   DELETE /ob/campaign/<campaign-uuid>

Returns
+++++++
Deleted campaign info.

::

   {
     $defhdr,
     "reuslt": {
       "uuid": "<string>",
       "name": "<string>",
       "detail": "<string>",
       "status": <integer>,
     
       "plan": "<string>",
       "dlma": "<string>",
       "dest": "<string>",
     
       "variables": {},
     
       "next_campaign": "<string>",
     
       "sc_mode": <integer>,
       "sc_time_start": "<string>",
       "sc_time_end": "<string>",
       "sc_date_start": "<string>",
       "sc_date_end": "<string>",
       "sc_date_list": "<string>",
       "sc_date_list_except": "<string>",
       "sc_day_list": "<string>"

       "in_use": 0,
       "tm_create": "<string>",
       "tm_delete": "<string>",
       "tm_update": "<string>"
     }
   }

Return parameters

* ``uuid`` : Campaign uuid.
* ``name`` : Campaign name.
* ``detail`` : Campaign detail info.
* ``status`` : Campaign status. See detail :ref:`campaign_status`.

* ``plan`` : Registered plan uuid.
* ``dlma`` : Registered dlma uuid.
* ``dest`` : Registered destination uuid.

* ``variables``: variables info json object.

* ``next_campaign`` : Campaign uuid for next campaign running.

* ``sc_mode`` : Scheduling mode. See detail :ref:`scheduling_mode`.
* ``sc_time_start`` : Campaign scheduling start time. See detail :ref:`scheduling_time`.
* ``sc_time_end`` : Campaign scheduling end time. See detail :ref:`scheduling_time`.
* ``sc_date_start`` : Campaign scheduling start date. See detail :ref:`scheduling_date`.
* ``sc_date_end`` : Campaign scheduling end date. See detail :ref:`scheduling_date`.
* ``sc_date_list`` : Campaign schedling date list. See detail :ref:`scheduling_date_list`.
* ``sc_date_list_except`` : Campaign scheduling except date list. See detail :ref:`scheduling_date_list`.
* ``sc_day_list`` : Campaign scheduling day list. See detail :ref:`scheduling_day_list`.

Example
+++++++
::

   $ curl -X DELETE 192.168.200.10:8081/ob/campaigns/305c96cb-4704-4f33-bcad-6b2e4031d7d4
   
   {
     "api_ver": "0.1",
     "timestamp": "2017-03-05T15:57:05.273138955Z",
     "statuscode": 200,
     "result": {
       "tm_delete": "2017-03-05T15:57:05.206650560Z",
       "sc_day_list": null,
       "sc_date_list_except": null,
       "next_campaign": null,
       "uuid": "305c96cb-4704-4f33-bcad-6b2e4031d7d4",
       "variables": null,
       "detail": null,
       "dlma": null,
       "name": "Update campaign info",
       "sc_time_end": null,
       "dest": null,
       "sc_time_start": null,
       "status": 0,
       "in_use": 0,
       "sc_date_end": null,
       "sc_date_start": null,
       "plan": null,
       "sc_mode": 0,
       "sc_date_list": null,
       "tm_create": "2017-03-05T15:25:09.788596601Z",
       "tm_update": "2017-03-05T15:44:10.295794418Z"
     }
   }

     