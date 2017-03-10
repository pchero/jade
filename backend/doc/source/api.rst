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
