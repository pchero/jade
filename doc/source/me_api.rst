.. _me_api:


.. _me_chats:

/me/chats
=========

Methods
-------
GET : Get the all chat info

POST : Create chat info.

Method: GET
-----------
Get the all chat info

Example
+++++++
::

  $ curl -k https://localhost:8081/me/chats\?authtoken=17cd592d-2706-44b6-b6e2-1c244b946cf0

  {
    "api_ver": "0.1",
    "result": [
        {
            "detail": "test chat detail",
            "name": "test chat name",
            "room": {
                "members": [
                    "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                    "980404a2-f509-4140-9c92-96a018d1b61c"
                ],
                "tm_create": "2018-03-27T06:53:37.432474880Z",
                "tm_update": null,
                "type": 1,
                "uuid": "57b8706a-67e7-4c3a-a070-b164a08562ab",
                "uuid_creator": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                "uuid_owner": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
            },
            "tm_create": "2018-03-27T06:53:37.496918935Z",
            "tm_update": null,
            "uuid": "15130428-6f27-456d-b744-6156e3a4b7a8"
        }
    ],
    "statuscode": 200,
    "timestamp": "2018-03-27T10:44:06.311937832Z"
  }


Method: POST
------------
Create chat info

Example
+++++++
::

  $ curl -k -X POST https://localhost:8081/me/chats\?authtoken=32644555-58e5-4d03-9608-a66ed1651f12 -d \
  '{"name": "test chat name", "detail": "test chat detail", "type": 1, \
  "members": ["980404a2-f509-4140-9c92-96a018d1b61c", "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"]}'
  
  {"api_ver": "0.1", "timestamp": "2018-03-26T16:10:54.451912673Z", "statuscode": 200}

.. _me_chats_detail:

/me/chats/<detail>
==================

Methods
-------
GET : Get the given detail chat info.

PUT : Update the given detail chat info.

DELETE : Delete the given detail chat info.

.. _get_me_chats_detail:

Method: GET
-----------
Get the given detail chat info.

Example
+++++++
::
  
  $ curl -k https://localhost:8081/me/chats/15130428-6f27-456d-b744-6156e3a4b7a8\?authtoken=17cd592d-2706-44b6-b6e2-1c244b946cf0
  
  {
    "api_ver": "0.1",
    "result": {
        "detail": "test chat detail",
        "name": "test chat name",
        "room": {
            "members": [
                "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                "980404a2-f509-4140-9c92-96a018d1b61c"
            ],
            "tm_create": "2018-03-27T06:53:37.432474880Z",
            "tm_update": null,
            "type": 1,
            "uuid": "57b8706a-67e7-4c3a-a070-b164a08562ab",
            "uuid_creator": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
            "uuid_owner": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
        },
        "tm_create": "2018-03-27T06:53:37.496918935Z",
        "tm_update": null,
        "uuid": "15130428-6f27-456d-b744-6156e3a4b7a8"
    },
    "statuscode": 200,
    "timestamp": "2018-03-27T10:39:34.496808298Z"
  }
  
Method: PUT
-----------
Update the given detail chat info.

Example
+++++++
::

  $ curl -k -X PUT https://localhost:8081/me/chats/7c817566-3c4a-4d97-b4f0-e29b2d2e1564\?authtoken=d2718890-043f-43c8-9bf9-79c2602d2c81 -d '{"name": "update test chat name"}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-03-26T07:27:03.713679335Z"
  }
  
Method: DELETE
--------------
Delete the given detail chat info.

Example
+++++++
::
  $ curl -k -X DELETE https://localhost:8081/me/chats/7c817566-3c4a-4d97-b4f0-e29b2d2e1564\?authtoken=d2718890-043f-43c8-9bf9-79c2602d2c81
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-03-26T07:28:19.397554581Z"
  }
  
.. _me_chats_detail_messages:
  
/me/chats/<detail>/messages
===========================

Methods
-------
GET : Get chat messages

POST: Create chat messages

.. _get_me_chats_detail_messages:

Method: GET
-----------
Get chat message

Call
++++
::

   GET /me/chats/<detail>/messages?authtoken=<string>[&timestamp=<string>&count=<number>]
  
   
Method parameters

* ``detail``: chat uuid.
* ``timestamp``: Uri encoded UTC timestamp.
* ``count``: number of message

Example
+++++++
::

  $ curl -k https://localhost:8081/me/chats/15130428-6f27-456d-b744-6156e3a4b7a8/messages\?authtoken=17cd592d-2706-44b6-b6e2-1c244b946cf0\&timestamp=2018-03-30T08%3A30%3A02.364443446Z\&count=2

  {
    "api_ver": "0.1",
    "result": [
        {
            "message": {
                "message": "test message"
            },
            "tm_create": "2018-03-27T10:26:14.452323600Z",
            "uuid": "1800fcee-1077-47f0-9d7c-3c7cde768e93",
            "uuid_owner": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
        },
        {
            "message": {
                "message": "test message"
            },
            "tm_create": "2018-03-27T08:30:50.225964433Z",
            "uuid": "eb251f63-8ed1-4a00-b757-20a88caa8a20",
            "uuid_owner": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
        }
    ],
    "statuscode": 200,
    "timestamp": "2018-03-27T10:36:04.543077586Z"
  }


Method: POST
------------

Example
+++++++
::

  $ curl -k -X POST https://localhost:8081/me/chats/15130428-6f27-456d-b744-6156e3a4b7a8/messages\?authtoken=b0da6bea-f654-446b-8900-2e52cf4f3cd6 -d '{"test message"}'

.. _me_info:

/me/info
========

Methods
-------
GET : Get me info.

.. _get_me_info:

Method: GET
-----------
Get me info.

Example
+++++++
::

  $ curl -k https://localhost:8081/me/info\?authtoken=88fa51b6-e980-4e57-93cb-7cdefd21b2a1
  
  {
    "api_ver": "0.1",
    "result": {
        "chats": [
            {
                "detail": "test chat detail",
                "name": "test chat name",
                "room": {
                    "members": [
                        "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                        "980404a2-f509-4140-9c92-96a018d1b61c"
                    ],
                    "tm_create": "2018-03-27T06:53:37.432474880Z",
                    "tm_update": null,
                    "type": 1,
                    "uuid": "57b8706a-67e7-4c3a-a070-b164a08562ab",
                    "uuid_creator": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                    "uuid_owner": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
                },
                "tm_create": "2018-03-27T06:53:37.496918935Z",
                "tm_update": null,
                "uuid": "15130428-6f27-456d-b744-6156e3a4b7a8"
            }
        ],
        "contacts": [
            {
                "detail": "test target detail 4",
                "info": {
                    "id": "199",
                    "password": "199",
                    "public_url": "sip:199@192.168.200.10",
                    "realm": "localhost"
                },
                "name": "test target",
                "target": "199",
                "tm_create": "2018-02-13T17:54:12.399972783Z",
                "tm_update": "2018-03-27T20:24:51.355343689Z",
                "type": "pjsip_endpoint",
                "user_uuid": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                "uuid": "62a78a12-34ba-4b4f-b9ea-e52e4bac6459"
            }
        ],
        "name": "teset admin",
        "tm_create": "2018-02-13T17:42:16.453145450Z",
        "tm_update": "2018-03-27T08:30:03.254978318Z",
        "username": "admin",
        "uuid": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
    },
    "statuscode": 200,
    "timestamp": "2018-03-27T20:25:32.526413444Z"
  }

  
  