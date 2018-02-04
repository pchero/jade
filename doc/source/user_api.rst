.. _user_api:


.. _user_contacts:

/user/contacts
==============

Methods
-------
GET : Get list of all user contacts info.

POST : Create user contact info

.. _get_user_contacts:

Method: GET
-----------
Get list of all user contacts info.

Example
+++++++
::

  $ curl localhost:8081/user/contacts\?authtoken=f1b5cd49-3c54-4aad-8585-754e3ab1796c
   
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "detail": "test target detail",
                "name": "test target",
                "target": "pjagent-01",
                "tm_create": "2018-02-04T01:34:34.972284886Z",
                "tm_update": null,
                "type": "pjsip_endpoint",
                "user_uuid": "b47977bc-913a-44d9-aaa9-33cc10970c30",
                "uuid": "a39b43a6-004f-472a-9c7b-80a9fbb91600"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2018-02-04T01:37:48.578013046Z"
  }

.. _post_user_contacts:

Method: POST
-----------
Create user contacts info.

Example
+++++++
::

  $ curl -X POST localhost:8081/user/contacts -d \
    '{"user_uuid": "b47977bc-913a-44d9-aaa9-33cc10970c30", "type": "pjsip_endpoint", "target": "pjagent-01", "name": "test target", "detail": "test target detail"}'
  
  {"api_ver": "0.1", "timestamp": "2018-02-04T01:34:34.983477950Z", "statuscode": 200}

