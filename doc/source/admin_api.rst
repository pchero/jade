.. _admin_api:

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
