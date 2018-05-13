.. _manager_api:

.. _manager_sdialplans:
/manager/sdialplans
===================

Methods
-------
GET : Get the all static dialplans info.

POST : Create static dialplan info


.. _manager_sdialplans_detail:
/manager/sdialplans/<detail>
============================

Methods
-------
GET : Get the given detail static dialplan info.

PUT : Update the given detail static dialplan info.

DELETE : Delete the given detail static dialplan info.


.. _manager_trunks:
/manager/trunks
===============

Methods
-------
GET : Get the all trunks info.

POST : Create trunk info


.. _manager_trunks_detail:
/manager/trunks/<detail>
========================

Methods
-------
GET : Get the given detail trunk info.

PUT : Update the given detail trunk info.

DELETE : Delete the given detail trunk info.


.. _manager_users:
/manager/users
==============

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

  $ curl -k https://192.168.200.10:8081/v1/manager/users?authtoken=06d838a0-71b4-4236-8e93-b7ff59c61502
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "context": "",
                "name": "Test Administrator",
                "password": "admin",
                "permissions": [
                    {
                        "permission": "admin"
                    },
                    {
                        "permission": "user"
                    }
                ],
                "tm_create": "2018-04-28T05:03:36.560694725Z",
                "tm_update": "2018-04-28T07:10:29.116108422Z",
                "username": "admin",
                "uuid": "ed629979-802b-40f0-9098-b30b2988f9b6"
            },
            {
                "context": "demo",
                "name": "test1",
                "password": "test1",
                "permissions": [
                    {
                        "permission": "user"
                    }
                ],
                "tm_create": "2018-04-29T07:19:00.56688782Z",
                "tm_update": null,
                "username": "test1",
                "uuid": "65abf0b4-9cd5-4bff-8ec9-c03c1aea22d4"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-05-02T13:24:50.821980931Z"
  }

  
Method: POST
------------
Create user info

Example
+++++++
::

  $ curl -k -X POST https://192.168.200.10:8081/v1/manager/users?authtoken=06d838a0-71b4-4236-8e93-b7ff59c61502 -d '{"context": "demo", "name": "test2", "username": "test2", "password": "test2", "permissions": [{"permission": "user"}]}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-02T13:28:31.201418110Z"
  }

/manager/users/<detail>
=======================

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
  
  $ curl -k https://192.168.200.10:8081/v1/manager/users/3305c47b-fc87-4186-b4ef-30cef0c8f5b2?authtoken=06d838a0-71b4-4236-8e93-b7ff59c61502
  
  {
    "api_ver": "0.1",
    "result": {
        "context": "demo",
        "name": "test2",
        "password": "test2",
        "permissions": [
            {
                "permission": "user"
            }
        ],
        "tm_create": "2018-05-02T13:28:31.101851891Z",
        "tm_update": null,
        "username": "test2",
        "uuid": "3305c47b-fc87-4186-b4ef-30cef0c8f5b2"
    },
    "statuscode": 200,
    "timestamp": "2018-05-02T13:30:52.782019666Z"
  }
  
Method: PUT
-----------
Update the given detail user info.

Example
+++++++
::

  $ curl -k -X PUT https://192.168.200.10:8081/v1/manager/users/3305c47b-fc87-4186-b4ef-30cef0c8f5b2?authtoken=06d838a0-71b4-4236-8e93-b7ff59c61502 -d '{"context": "demo-test", "name": "test2", "username": "test2", "password": "test2", "permissions": [{"permission": "user"}]}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-02T13:38:39.731423546Z"
  }
  
Method: DELETE
--------------
Delete the given detail user info.

Example
+++++++
::

  $ curl -k -X DELETE https://192.168.200.10:8081/v1/manager/users/3305c47b-fc87-4186-b4ef-30cef0c8f5b2?authtoken=06d838a0-71b4-4236-8e93-b7ff59c61502

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-05-02T13:40:24.837434999Z"
  }
