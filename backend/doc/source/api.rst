.. _api:

**********
API basics
**********

.. index:: Title, Purpose, Methods, Method, Required Permission, Call, Returns, Example

API description
===============

Title
-----
The main category of API. Normally, it represents general API URI.

Purpose
-------
The purpose of API.

Methods
-------
List of supported command with simple description.

Method: <command>
-----------------
Method description with command in detail.
It shown also added version.

Call
++++
Description of how to call this API. It will explain about method
parameters and data parameters.

::

  <method> <call URI>

  <required data>

Method parameters
* ``method``: API calling method. i.e. GET, PUT, POST, ...
* ``call URI``: URI. Uniform Resource Identifier

Data parameters
* ``required data``: Required data to call the API.

Returns
+++++++
Description of reply. Described in detail. These are common return
objects. This objects will not be explain again.

::

  {
    $defhdr,
    "result": {
      "message": "<message-name>",
      "type": "<message-type>",
      "timestamp_ms": "<YYYY-mm-ddTHH:MM:SS.fffZ>"
    }
  }

* ``message``: Represents message name. Fixed.
* ``type``: Represents message type. Fixed.
* ``timestamp_ms`` Denotes when the message has been created.
  See the :term:`Timestamp`.

Example
+++++++
Simple example of how to use the API. It would be little bit different with real response.

Normally, to test the API curl is used. curl is a tool to transfer
data from or to a server, using one of the supported protocols. See
detail at link below.

::

  https://curl.haxx.se/docs/manpage.html

Some of APIs requires a returned uuid for the request. But
one must expect that these information are only valid within the user
sessions and are temporary.

*********
Resources
*********

/databases
==========

Methods
-------
GET : Get list of all database keys.

Method: GET
-----------
Get list of all database keys.

Call
++++
::

  GET /databases

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        "<database-key>",
        ...
      ]
    }
  }
  
* ``list`` : array of database-key.

Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/databases
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            "/Queue/PersistentMembers/sales_1",
            "/SIP/Registry/300",
            "/SIP/Registry/301",
            "/SIP/Registry/302",
            "/SIP/Registry/agent-01",
            "/SIP/Registry/agent-02",
            "/SIP/Registry/agent-03",
            "/dundi/secret",
            "/dundi/secretexpiry",
            "/pbx/UUID",
            "/sample_database/test"
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T07:51:41.829683573Z"
  }

/databases/
===========

Methods
-------
GET : Get value of input key.

Method: GET
-----------
GET : Get value of input key.

Call
++++
::

  GET /databases/

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        "<database-key>",
        ...
      ]
    }
  }
  
* ``list`` : array of database-key.

Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/databases/ -d'{"key":"/Queue/PersistentMembers/sales_1"}' 
  
  {
    "api_ver": "0.1",
    "result": {
        "key": "/Queue/PersistentMembers/sales_1",
        "tm_update": null,
        "value": "sip/agent-01;0;0;sip/agent-01;sip/agent-01;|sip/agent-02;0;0;sip/agent-02;sip/agent-02;|sip/agent-03;0;0;sip/agent-03;sip/agent-03;"
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T07:52:18.392313868Z"
  }


/plans
======

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

  POST /plans
  
  {
    "name": "<string>",
    "detail": "<string>",
    "tech_name": "<string>",
    "trunk_name": "<string>",
    "dl_end_handle": <integer>,
    "caller_id": "<string>",
    "dial_mode": <integer>,
    "dial_timeout": <integer>,
    "retry_delay": <integer>,
    "service_level": <integer>,
    "early_media": "<string>",
    "codecs": "<string>",
    "max_retry_cnt_1": <integer>,
    "max_retry_cnt_2": <integer>,
    "max_retry_cnt_3": <integer>,
    "max_retry_cnt_4": <integer>,
    "max_retry_cnt_5": <integer>,
    "max_retry_cnt_6": <integer>,
    "max_retry_cnt_7": <integer>,
    "max_retry_cnt_8": <integer>,
    "variables": "<string>"
  }


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
      "dl_end_handle": <integer>,
      "caller_id": "<string>",
      "dial_mode": <integer>,
      "dial_timeout": <integer>,
      "retry_delay": <integer>,
      "service_level": <integer>,
      "early_media": "<string>",
      "codecs": "<string>",
      "max_retry_cnt_1": <integer>,
      "max_retry_cnt_2": <integer>,
      "max_retry_cnt_3": <integer>,
      "max_retry_cnt_4": <integer>,
      "max_retry_cnt_5": <integer>,
      "max_retry_cnt_6": <integer>,
      "max_retry_cnt_7": <integer>,
      "max_retry_cnt_8": <integer>,
      "variables": "<string>",
      "in_use": <integer>,
      "tm_create": "<timestamp>",
      "tm_update": "<timestamp>",
      "tm_delete": "<timestamp>"
    }
  }

* ``uuid`` : plan uuid.
* ``name`` : Plan name. Default null.
* ``detail`` : Detail info. Default null.
* ``tech_name`` : Tech name for outbound dialing. Default null. See detail Tech name.
* ``trunk_name`` : Trunkname for outbound dialing. Default null.
* ``dl_end_handle`` : dial list end handle.
* ``caller_id``: Caller’s id.
* ``dial_mode``: Dialling mode. See detail Dial mode.
* ``dial_timeout``: Ringing timeout(ms).
* ``retry_delay``: 
* ``service_level``: Determine service level.
* ``early_media``: 
* ``codecs``: 
* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.
* ``variables``: 

Example
+++++++
::

  $ curl -X POST 192.168.200.10:8081/plans -d '{"name": "Test plan create"}'

  {
    "api_ver": "0.1",
    "timestamp": "2017-02-22T16:57:49.973941490Z",
    "statuscode": 200,
    "result": {
      "detail": null,
      "tech_name": "SIP",
      "uuid": "c8b521ab-8114-4a62-b4e9-488d770d4ee8",
      "dl_end_handle": 1,
      "name": "Test plan create",
      "tm_delete": null,
      "max_retry_cnt_5": 5,
      "caller_id": null,
      "variables": null,
      "max_retry_cnt_4": 5,
      "in_use": 1,
      "dial_mode": 1,
      "retry_delay": 60,
      "dial_timeout": 30000,
      "trunk_name": null,
      "tm_create": "2017-02-22T16:57:49.962162166Z",
      "service_level": 0,
      "early_media": null,
      "codecs": null,
      "max_retry_cnt_1": 5,
      "max_retry_cnt_2": 5,
      "max_retry_cnt_3": 5,
      "max_retry_cnt_6": 5,
      "max_retry_cnt_7": 5,
      "max_retry_cnt_8": 5,
      "tm_update": null
    }
  }


Method: GET
-----------
Get list of all plans.

Call
++++
::

  GET /plans
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        "<plan-uuid>",
        ...
      ]
    }
  }
  
* ``list`` : array of plan-uuid.

Example
+++++++
::

  curl -X GET 192.168.200.10:8081/plans
  
  {
    "api_ver": "0.1",
    "result": {
      "list": [
         "552e9808-23bc-40b1-947a-60b0b96581cb"
      ]
    },
    "statuscode": 200,
    "timestamp": "2017-02-22T16:48:04.890697147Z"
  }
  
/plans/<uuid>
=============

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

   GET /plans/<plan-uuid>

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
       "dl_end_handle": <integer>,
       "caller_id": "<string>",
       "dial_mode": <integer>,
       "dial_timeout": <integer>,
       "retry_delay": <integer>,
       "service_level": <integer>,
       "early_media": "<string>",
       "codecs": "<string>",
       "max_retry_cnt_1": <integer>,
       "max_retry_cnt_2": <integer>,
       "max_retry_cnt_3": <integer>,
       "max_retry_cnt_4": <integer>,
       "max_retry_cnt_5": <integer>,
       "max_retry_cnt_6": <integer>,
       "max_retry_cnt_7": <integer>,
       "max_retry_cnt_8": <integer>,
       "variables": "<string>",
       "in_use": <integer>,
       "tm_create": "<timestamp>",
       "tm_update": "<timestamp>",
       "tm_delete": "<timestamp>"
     }
   }

* ``uuid`` : plan uuid.
* ``name`` : Plan name. Default null.
* ``detail`` : Detail info. Default null.
* ``tech_name`` : Tech name for outbound dialing. Default null. See detail Tech name.
* ``trunk_name`` : Trunkname for outbound dialing. Default null.
* ``dl_end_handle`` : dial list end handle.
* ``caller_id``: Caller’s id.
* ``dial_mode``: Dialling mode. See detail Dial mode.
* ``dial_timeout``: Ringing timeout(ms).
* ``retry_delay``: 
* ``service_level``: Determine service level.
* ``early_media``: 
* ``codecs``: 
* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.
* ``variables``: 

Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/plans/552e9808-23bc-40b1-947a-60b0b96581cb

  {
    "api_ver": "0.1",
    "result": {
      "caller_id": null,
      "codecs": null,
      "detail": null,
      "dial_mode": 1,
      "dial_timeout": 30000,
      "dl_end_handle": 1,
      "early_media": null,
      "in_use": 1,
      "max_retry_cnt_1": 1000,
      "max_retry_cnt_2": 5,
      "max_retry_cnt_3": 5,
      "max_retry_cnt_4": 5,
      "max_retry_cnt_5": 5,
      "max_retry_cnt_6": 5,
      "max_retry_cnt_7": 5,
      "max_retry_cnt_8": 5,
      "name": null,
      "retry_delay": 60,
      "service_level": 0,
      "tech_name": "SIP",
      "tm_create": "2017-02-21T13:53:16.684583113Z",
      "tm_delete": null,
      "tm_update": "2017-02-21T14:08:21.484784844Z",
      "trunk_name": null,
      "uuid": "552e9808-23bc-40b1-947a-60b0b96581cb",
      "variables": null
    },
    "statuscode": 200,
    "timestamp": "2017-02-22T17:43:11.217963213Z"
  }
  
Method: PUT
-----------
Get plan detail info.

Call
++++
Update specified plan info.

::

  PUT /plans/<plan-uuid>
  
  {
    "name": "<string>",
    "detail": "<string>",
    "tech_name": "<string>",
    "trunk_name": "<string>",
    "dl_end_handle": <integer>,
    "caller_id": "<string>",
    "dial_mode": <integer>,
    "dial_timeout": <integer>,
    "retry_delay": <integer>,
    "service_level": <integer>,
    "early_media": "<string>",
    "codecs": "<string>",
    "max_retry_cnt_1": <integer>,
    "max_retry_cnt_2": <integer>,
    "max_retry_cnt_3": <integer>,
    "max_retry_cnt_4": <integer>,
    "max_retry_cnt_5": <integer>,
    "max_retry_cnt_6": <integer>,
    "max_retry_cnt_7": <integer>,
    "max_retry_cnt_8": <integer>,
    "variables": "<string>"
  }

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
      "dl_end_handle": <integer>,
      "caller_id": "<string>",
      "dial_mode": <integer>,
      "dial_timeout": <integer>,
      "retry_delay": <integer>,
      "service_level": <integer>,
      "early_media": "<string>",
      "codecs": "<string>",
      "max_retry_cnt_1": <integer>,
      "max_retry_cnt_2": <integer>,
      "max_retry_cnt_3": <integer>,
      "max_retry_cnt_4": <integer>,
      "max_retry_cnt_5": <integer>,
      "max_retry_cnt_6": <integer>,
      "max_retry_cnt_7": <integer>,
      "max_retry_cnt_8": <integer>,
      "variables": "<string>",
      "in_use": <integer>,
      "tm_create": "<timestamp>",
      "tm_update": "<timestamp>",
      "tm_delete": "<timestamp>"
    }
  }

* ``uuid`` : plan uuid.
* ``name`` : Plan name. Default null.
* ``detail`` : Detail info. Default null.
* ``tech_name`` : Tech name for outbound dialing. Default null. See detail Tech name.
* ``trunk_name`` : Trunkname for outbound dialing. Default null.
* ``dl_end_handle`` : dial list end handle.
* ``caller_id``: Caller’s id.
* ``dial_mode``: Dialling mode. See detail Dial mode.
* ``dial_timeout``: Ringing timeout(ms).
* ``retry_delay``: 
* ``service_level``: Determine service level.
* ``early_media``: 
* ``codecs``: 
* ``max_retry_cnt_1``: Max retry count for number 1.
* ``max_retry_cnt_2``: Max retry count for number 2.
* ``max_retry_cnt_3``: Max retry count for number 3.
* ``max_retry_cnt_4``: Max retry count for number 4.
* ``max_retry_cnt_5``: Max retry count for number 5.
* ``max_retry_cnt_6``: Max retry count for number 6.
* ``max_retry_cnt_7``: Max retry count for number 7.
* ``max_retry_cnt_8``: Max retry count for number 8.
* ``variables``: 

Example
+++++++
::

   $ curl -X PUT 192.168.200.10:8081/plans/c8b521ab-8114-4a62-b4e9-488d770d4ee8 \
    -d '{"name": "Updated plan name"}'
   
   {
     "api_ver": "0.1",
     "timestamp": "2017-02-22T17:59:41.91267195Z",
     "statuscode": 200,
     "result": {
        "detail": null,
        "tech_name": "SIP",
        "uuid": "c8b521ab-8114-4a62-b4e9-488d770d4ee8",
        "dl_end_handle": 1,
        "name": "Updated plan name",
        "tm_delete": null,
        "max_retry_cnt_5": 5,
        "caller_id": null,
        "variables": null,
        "max_retry_cnt_4": 5,
        "in_use": 1,
        "dial_mode": 1,
        "retry_delay": 60,
        "dial_timeout": 30000,
        "trunk_name": null,
        "tm_create": "2017-02-22T16:57:49.962162166Z",
        "service_level": 0,
        "early_media": null,
        "codecs": null,
        "max_retry_cnt_1": 5,
        "max_retry_cnt_2": 5,
        "max_retry_cnt_3": 5,
        "max_retry_cnt_6": 5,
        "max_retry_cnt_7": 5,
        "max_retry_cnt_8": 5,
        "tm_update": "2017-02-22T17:59:41.59248781Z"
      }
    }

Method: DELETE
--------------
Delete specified plan info.

Call
++++
Delete specified plan info.

::

   DELETE /plans/<plan-uuid>

Returns
+++++++
Deleted plan info.

::

   ...
   
Example
+++++++
::

   curl -X DELETE 192.168.200.10:8081/plans/c8b521ab-8114-4a62-b4e9-488d770d4ee8
   
   {
     "api_ver": "0.1",
     "timestamp": "2017-02-22T18:05:00.598284056Z",
     "statuscode": 200,
     "result": {
       "detail": null,
       "tech_name": "SIP",
       "uuid": "c8b521ab-8114-4a62-b4e9-488d770d4ee8",
       "dl_end_handle": 1,
       "name": "Updated plan name",
       "tm_delete": "2017-02-22T18:05:00.548497354Z",
       "max_retry_cnt_5": 5,
       "caller_id": null,
       "variables": null,
       "max_retry_cnt_4": 5,
       "in_use": 0,
       "dial_mode": 1,
       "retry_delay": 60,
       "dial_timeout": 30000,
       "trunk_name": null,
       "tm_create": "2017-02-22T16:57:49.962162166Z",
       "service_level": 0,
       "early_media": null,
       "codecs": null,
       "max_retry_cnt_1": 5,
       "max_retry_cnt_2": 5,
       "max_retry_cnt_3": 5,
       "max_retry_cnt_6": 5,
       "max_retry_cnt_7": 5,
       "max_retry_cnt_8": 5,
       "tm_update": "2017-02-22T17:59:41.59248781Z"
     }
   }

/destinations
=============

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

  POST /destinations
  
  {
    "application": null,
    "context": null,
    "data": null,
    "detail": null,
    "exten": null,
    "in_use": 1,
    "name": null,
    "priority": null,
    "type": 0,
    "variables": null
  }

Returns
+++++++
Created destination info.

::

  {
    $defhdr,
    "reuslt": {
      "uuid": "5ff9611c-136b-47d7-b8d4-0bd6f0d5b037",
      "application": null,
      "context": null,
      "data": null,
      "detail": null,
      "exten": null,
      "in_use": 1,
      "name": null,
      "priority": null,
      "type": 0,
      "variables": null,
      "tm_create": "2017-03-05T08:48:43.745658496Z",
      "tm_delete": null,
      "tm_update": null
    }
  }


Example
+++++++
::

  $ curl -X POST 192.168.200.10:8081/destinations -d'{}'

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

  GET /destinations
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        "<destination-uuid>",
        ...
      ]
    }
  }
  
* ``list`` : array of destination-uuid.

Example
+++++++
::

  curl -X GET 192.168.200.10:8081/destinations
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            "45122654-5633-4af0-a739-e32eddfbd2ae",
            "5ff9611c-136b-47d7-b8d4-0bd6f0d5b037"
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T08:58:09.460418643Z"
  }

/destinations/<uuid>
====================

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

  GET /destinations/<uuid>

Returns
+++++++
Specified destination detail info.

::

  {
    $defhdr,
    "reuslt": {
      "application": "<string>",
      "context": "<string>",
      "data": "<string>",
      "detail": "<string>",
      "exten": "<string>",
      "in_use": 1,
      "name": "<string>",
      "priority": null,
      "type": 0,
      "uuid": "<string>",
      "variables": null,
      "tm_create": "2017-03-05T08:48:43.745658496Z",
      "tm_delete": null,
      "tm_update": null
    }
  }


Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/destinations/5ff9611c-136b-47d7-b8d4-0bd6f0d5b037

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
Get specified destination detail info.

Call
++++
::

  PUT /destinations/<uuid>
  
  {
  }

Returns
+++++++
Return the updated destination detail info.

::

  {
    $defhdr,
    "reuslt": {
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
    }
  }


Example
+++++++
::

  $ curl -X PUT 192.168.200.10:8081/destinations/5ff9611c-136b-47d7-b8d4-0bd6f0d5b037 \
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

  DELETE /destinations/<uuid>  

Returns
+++++++
Return the deleted destination detail info.

::

  {
    $defhdr,
    "reuslt": {
    }
  }


Example
+++++++
::

  $ curl -X DELETE 192.168.200.10:8081/destinations/38dd5d48-4758-4194-9b7d-24acada05e08

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


/campaigns
==========

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

  POST /campaign
  
  {
  }

Returns
+++++++
Created campaign info.

::

  {
    $defhdr,
    "reuslt": {
    }
  }


Example
+++++++
::

  $ curl -X POST 192.168.200.10:8081/destinations -d'{}'

  {
    }
  }

Method: GET
-----------
Get list of all campaigns.

Call
++++
::

  GET /campaigns
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        "<campaign-uuid>",
        ...
      ]
    }
  }
  
* ``list`` : array of campaign-uuid

Example
+++++++
::

  curl -X GET 192.168.200.10:8081/campaigns
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            "9841bc9e-3103-4fea-ab11-54ccfcc8322f"
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T09:10:23.658031316Z"
  }

/campaigns/<uuid>
=================

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

  GET /campaign/uuid
  
  {
  }

Returns
+++++++
Specified campaign detail info.

::

  {
    $defhdr,
    "reuslt": {
    }
  }


Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/campaigns/9841bc9e-3103-4fea-ab11-54ccfcc8322f

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

Method: GET
-----------
Get list of all campaigns.

Call
++++
::

  GET /campaigns
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        "<campaign-uuid>",
        ...
      ]
    }
  }
  
* ``list`` : array of campaign-uuid

Example
+++++++
::

  curl -X GET 192.168.200.10:8081/campaigns
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            "9841bc9e-3103-4fea-ab11-54ccfcc8322f"
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-03-05T09:10:23.658031316Z"
  }



   