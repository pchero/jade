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
      "timestamp": "<YYYY-mm-ddTHH:MM:SS.fffZ>"
    }
  }

* ``message``: Represents message name. Fixed.
* ``type``: Represents message type. Fixed.
* ``timestamp`` Denotes when the message has been created. See the :term:`Timestamp`.

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

***
API
***

/agents
=======

Methods
-------
GET : Get list of all agents info.

Method: GET
-----------
Get list of all agents info.

Call
++++
::

   GET /agents

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           "id": "<string>"
         },
         ...
       ]
     }
   }
  
* ``list`` : array of agents id.
   * ``id``: agent id.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/agents
   
   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {
           "id": "1001"
         },
         {
           "id": "1002"
         }
       ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-13T17:19:07.48261155Z"
   }


/agents/
========

Methods
-------
GET : Get agent's detail info of given agent id.

Method: GET
-----------
Get agent's detail info of given agent id.

Call
++++
::

   GET /agents/
  
   {
     "id": "<string>"
   }

Data parameters

* ``id``: agent id.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "id": "<string>",
       "name": "<string>",
       "status": "<string>",
       "logged_in_time": <integer>,

       "channel_name": "<string>",
       "channel_state": <integer>,
       "channel_state_desc": "<string>",

       "caller_id_name": "<string>",
       "caller_id_num": "<string>",

       "connected_line_name": "<string>",
       "connected_line_num": "<string>",

       "context": "<string>",
       "exten": "<string>",
       "priority": "<string>",

       "account_code": "<string>",
       "language": "<string>",

       "unique_id": "<string>",
       "linked_id": "<string>",

       "tm_update": "<string>",
     }
   }

Return parameters

* ``id``: Agent id.
* ``name``: Agent name.
* ``status``: Agent status. See detail :ref:`agent_status`.
* ``logged_in_time``: Logged in time.

* ``channel_name``: Agent channel's name.
* ``channel_state``: Agent channel's state.
* ``channel_state_desc``: Agent channel's state description.

* ``caller_id_name``: Caller's id name.
* ``caller_id_num``: Caller's id number.

* ``connected_line_name``: Connected line name.
* ``connected_line_num``: Connected line number.

* ``context``: Context.
* ``exten``: Extension.
* ``priority``: Priority

* ``caller_id_name``: Caller's id name.
* ``caller_id_num``: Caller's id number.
* ``connected_line_name``: Caller's line name.
* ``connected_line_num``: Caller's line number.

* ``account_code``: Account code.
* ``language``: Language.

* ``unique_id``: Channel's unique id.
* ``linked_id``: Channel's linked id.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/agents/ -d '{"id": "1001"}'
   
   {
     "api_ver": "0.1",
     "result": {
       "account_code": "",
       "caller_id_name": "300",
       "caller_id_num": "300",
       "channel_name": "SIP/300-00000425",
       "channel_state": 6,
       "channel_state_desc": "Up",
       "connected_line_name": "<unknown>",
       "connected_line_num": "<unknown>",
       "context": "sample_agent_login",
       "exten": "s",
       "id": "1001",
       "language": "en",
       "linked_id": "1489423716.2037",
       "logged_in_time": 1489423716,
       "name": "Agent 1001",
       "priority": "2",
       "status": "AGENT_IDLE",
       "tm_update": "2017-03-13T17:19:06.724533682Z",
       "unique_id": "1489423716.2037"
     },
     "statuscode": 200,
     "timestamp": "2017-03-13T17:20:01.778206702Z"
   }



/device_states
==============

Methods
-------
GET : Get list of all device_state device info.

Method: GET
-----------
Get list of all device_state device info.

Call
++++
::

   GET /device_states

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
           "device": "<string>"
         },
         ...
       ]
     }
   }

Return parameters

* ``list`` : array of channels.
   * ``device``: device name.

Example
+++++++
::

   $ curl -X GET example.com:8081/device_states
   
   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {"device": "Agent:1001"},
         ...
       ]
     },
     "statuscode": 200,
     "timestamp": "2017-04-04T23:09:36.539677164Z"
   }


/device_states/
===============

Methods
-------
GET : Get device_state's detail info of given info.

Method: GET
-----------
Get device_state's detail info of given info.

Call
++++
::

   GET /device_states/
  
   {
     "device": "<string>"
   }
   
Data parameters

* ``device``: unique id.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "device": "<string>",
       "state": "<string>",
       
       "tm_update": "2017-04-04T23:08:40.878892472Z"
     }
   }

Return parameters

* ``device``: Device name.
* ``state``: Device state. See detail :ref:`device_state`.

Example
+++++++
::

   $ curl -X GET example.com/device_states/ -d \
   '{"device": "Agent:1001"}'
   
   {
     "api_ver": "0.1",
     "result": {
       "device": "Agent:1001",
       "state": "UNAVAILABLE",
       "tm_update": "2017-04-04T23:08:40.878892472Z"
     },
     "statuscode": 200,
     "timestamp": "2017-04-04T23:10:37.684581876Z"
   }


/peers
======

Methods
-------
GET : Get list of all peer info.

Method: GET
-----------
Get list of all peer info.

Call
++++
::

   GET /peers

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {"peer": "<string>"},
         ...
       ]
     }
   }
  
* ``list`` : array of peer entry info.
   * ``peer``: Peer entry info.

Example
+++++++
::

  $ curl -X GET saturn.pchero21.com:8081/peers
  
  {
    "api_ver": "0.1",
    "result": {
      "list": [
        {
            "peer": "SIP/300"
        },
        ...
      ]
    },
    "statuscode": 200,
    "timestamp": "2017-04-09T01:42:24.765567356Z"
  }

/peers/
=======

Methods
-------
GET : Get peer detail info of given peer info.

Method: GET
-----------
Get peer detail info of given peer info.

Call
++++
::

   GET /peers/
  
   {
     "peer": "<string>"  
   }
   
Data parameters

* ``peer``: peer info.

Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "peer": "<string>",

      "status": "<string>",
      "address": "<string>",
      "monitor_status": "<string>",

      "channel_type": "<string>",
      "chan_object_type": "<string>",

      "dynamic": "<string>",
      "auto_comedia": "<string>",
      "auto_force_port": "<string>",
      "acl": "<string>",
      "comedia": "<string>",
      "description": "<string>",
      "force_port": "<string>",
      "realtime_device": "<string>",
      "text_support": "<string>",
      "video_support": "<string>",
      
      "tm_update": "<timestamp>",
    }
  }

* ``peer``: Peer name.

* ``status``: Peer's status. See detail :ref:`peer_status`.
* ``address``: Registered peer's address. "<ip>:<port>"
* ``monitor_status``: Peer monitoring status. See detail :ref:`peer_monitoring_status`.

* ``dynamic``: Peer's dynamic option value.
* ``auto_comedia``: Peer's auto_comedia option value.
* ``auto_force_port``: Peer's auto_force_port option value.
* ``acl``: Peer's dynamic acl value.
* ``comedia``: Peer's comedia option value.
* ``description``: Peer's description option value.
* ``force_port``: Peer's force_port option value.
* ``realtime_device``: Peer's realtime_device option value.
* ``text_support``: Peer's text support option value.
* ``video_support``: Peer's video support option value.

* ``tm_update``: Updated timestamp.

Example
+++++++
::

   $ curl -X GET saturn.pchero21.com:8081/peers/ -d '{"peer": "agent-01"}'
  
  {
    "api_ver": "0.1",
    "result": {
      "acl": "no",
      "address": "192.168.200.1:5060",
      "auto_comedia": "yes",
      "auto_force_port": "no",
      "chan_object_type": "peer",
      "channel_type": "SIP",
      "comedia": "yes",
      "description": "",
      "dynamic": "yes",
      "force_port": "no",
      "monitor_status": "OK (22 ms)",
      "peer": "SIP/agent-01",
      "realtime_device": "no",
      "status": "Registered",
      "text_support": "no",
      "tm_update": "2017-04-09T02:08:18.756977409Z",
      "video_support": "no"
    },
    "statuscode": 200,
    "timestamp": "2017-04-09T02:09:55.226677432Z"
  }

/registries
===========

Methods
-------
GET : Get list of all registry accounts info.

Method: GET
-----------
Get list of all registry accounts info.

Call
++++
::

   GET /registries

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {"account": "<string>"},
         ...
       ]
     }
   }
  
* ``list`` : array of registry account.
   * ``account``: Registry account info.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/registries
  
   {
     "api_ver": "0.1",
     "result": {
       "list": [
         {"account": "2345@sip_proxy:5060"},
         {"account": "1234@mysipprovider.com:5060"}
       ]
     },
     "statuscode": 200,
     "timestamp": "2017-03-12T00:27:18.798367307Z"
   }

/registries/
============

Methods
-------
GET : Get registry detail info of given account info.

Method: GET
-----------
Get registry detail info of given account info.

Call
++++
::

   GET /registries/
  
   {
     "account": "<string>"  
   }
   
Data parameters

* ``account``: account info.

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "account": "<string>",
       "state": "<string>",
       
       "username": "<string>",
       "domain": "<string>",
       "domain_port": <integer>,
       "host": "<string>",
       "port": <integer>,
       
       "refresh": <integer>,
       "registration_time": <integer>,

       "tm_update": "<timestamp>"
     }
   }

* ``account``: Registry's account info.
* ``state``: state info.

* ``username``: Registry's username.
* ``domain``: Registry's domain info.
* ``domain_port``: Registry's domain port.
* ``host``: Registry's host info.
* ``port``: Registry's port info.

* ``refresh``: Refresh.
* ``registration_time``: registration time.

* ``tm_update``: Updated timestamp.

Example
+++++++
::

   $ curl -X GET 192.168.200.10:8081/registries/ -d '{"account": "2345@sip_proxy"}'
  
   {
     "api_ver": "0.1",
     "result": {
       "account": "2345@sip_proxy",
       "domain": "sip_proxy",
       "domain_port": 5060,
       "host": "sip_proxy",
       "port": 5060,
       "refresh": 120,
       "registration_time": "0",
       "state": "Request Sent",
       "tm_update": "2017-03-09T09:12:06.652539075Z",
       "username": "2345"
     },
     "statuscode": 200,
     "timestamp": "2017-03-09T09:13:00.969355467Z"
   }


.. include:: core_api.rst
.. include:: ob_api.rst
.. include:: park_api.rst
.. include:: pjsip_api.rst
.. include:: queue_api.rst
.. include:: voicemail_api.rst
