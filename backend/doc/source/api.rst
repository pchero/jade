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
