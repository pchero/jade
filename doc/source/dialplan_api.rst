.. _dialplan_api:

.. _dp_dialplans:

/dp/dialplans
=============

Methods
-------
GET : Get list of all dialplans info.

POST : Create new dialplan info.

.. _get_dp_dialplans:

Method: GET
-----------
Get list of all dialplan info.

Call
++++
::

   GET /dp/dialplans

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

  $ curl -X GET localhost:8081/dp/dialplans
  
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
Create new dialplan info.

Call
++++
::

  POST /dp/dialplans
  
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

  $ curl -X POST localhost:8081/dp/dialplans -d 
  '{"name": "test_dialplan_1", "dpma_uuid":"1d47735d-2265-463c-908a-d37dea085c21", "sequence": 1}'

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:38:38.709550080Z"
  }


.. _dp_dialplans_detail:
  
/dp/dialplans/<detail>
==========================

Methods
-------
GET : Get dialplan detail info of given detail.

PUT : Update dialplan detail info of given detail.

DELETE : Delete the given dialplan info.

.. _get_dp_dialplans_detail:

Method: GET
-----------
Get dialplan detail info of given detail.

Call
++++
::

  GET /dp/dialplans/<detail>


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

  $ curl localhost:8081/dp/dialplans/abde9ef2-7255-4e64-acbe-a6edb964c442
  
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


.. _put_dp_dialplans_detail:

Method: PUT
-----------
Update dialplan detail info of given detail.

Call
++++
::

  PUT /dp/dialplans/<detail>
  
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

  $ curl -X PUT localhost:8081/dp/dialplans/91bba56b-7ec5-4fe2-a724-9f39d527da8c -d \
  '{"uuid": "91bba56b-7ec5-4fe2-a724-9f39d527da8c", \
  "dpma_uuid": "1d47735d-2265-463c-908a-d37dea085c21", "sequence": 10}'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:27:54.683794074Z"
  }
  

.. _delete_dp_dialplans_detail:

Method: DELETE
--------------
DELETE : Delete the given dialplan info.

Call
++++
::

  DELETE /dp/dialplans/<detail>

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -X DELETE localhost:8081/dp/dialplans/91bba56b-7ec5-4fe2-a724-9f39d527da8c
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:29:34.538575375Z"
  }

  
  
.. _dp_dpmas:

/dp/dpmas
=========

Methods
-------
GET : Get list of all dpma info.

POST : Create new dpma info.

.. _get_dp_dpmas:

Method: GET
-----------
Get list of all dpma info.

Call
++++
::

   GET /dp/dpmas

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
    * See detail dpma detail info.

Example
+++++++
::

  $ curl localhost:8081/dp/dpmas
  
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
Create new dpma info.

Call
++++
::

  POST /dp/dpmas
  
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

  $ curl -X POST localhost:8081/dp/dpmas -d '{"name": "test dpma", "detail": "test dpma dpma"}'

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-22T23:40:55.524091997Z"
  }


.. _dp_dpmas_detail:
  
/dp/dpmas/<detail>
==========================

Methods
-------
GET : Get dpmas detail info of given detail.

PUT : Update dpmas detail info of given detail.

DELETE : Delete the given dpmas info.

.. _get_dp_dpmas_detail:

Method: GET
-----------
Get dpma detail info of given detail.

Call
++++
::

  GET /dp/dpmas/<detail>


Method parameters

* ``detail``: dpma uuid

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

* ``uuid``: dlma uuid.

* ``name``: Name.
* ``detail``: Detail info.

* ``tm_create``: timestamp.
* ``tm_update``: timestamp.

Example
+++++++
::

  $ curl localhost:8081/dp/dpmas/1d47735d-2265-463c-908a-d37dea085c21
  
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


.. _put_dp_dpmas_detail:

Method: PUT
-----------
Update dpmas detail info of given detail.

Call
++++
::

  PUT /dp/dpmas/<detail>
  
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

  $ curl -X PUT localhost:8081/dp/dpmas/1d47735d-2265-463c-908a-d37dea085c21 -d 
  '{"name": "test change name" }'
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-22T23:46:38.238159711Z"
  }
  

.. _delete_dp_dpmas_detail:

Method: DELETE
--------------
DELETE : Delete the given dpma info.

Call
++++
::

  DELETE /dp/dpmas/<detail>

Returns
+++++++
::

  {
    $defhdr
  }

Example
+++++++
::

  $ curl -X DELETE localhost:8081/dp/dpmas/1d47735d-2265-463c-908a-d37dea085c21
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2018-01-23T01:59:22.411717755Z"
  }

.. _dialplan_setting:

/dialplan/setting
=================

Methods
-------
GET : Get current dialplan setting.

PUT : Update current dialplan setting.

.. _get_dialplan_setting:

Method: GET
-----------
GET : Get current dialplan setting.

This result does not mean to currently running setting. 
It shows the only setting file.

Call
++++
::

  GET ^/dialplan/setting?<format=<value>>
  
Method parameters

* ``format``: The only text type is allowed.
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      ...
    }
  }

Return parameters

* See detail at dialplan setting.

Example
+++++++
::

  $ curl -X GET localhost:8081/dialplan/setting\?format=text
  
  {
    "api_ver": "0.1", 
    "timestamp": "2018-01-21T19:19:12.139325742Z", 
    "statuscode": 200, 
    "result": "; extensions.conf - the Asterisk dial plan\n;\n;..."
  }


.. _put_dialplan_setting:

Method: PUT
-----------
PUT : Update dialplan setting.

Update only setting file. To adapt to the module, it required module reload.

Call
++++
::

  PUT ^/dialplan/setting
  
  {
    ...
  }

Data parameters

* dialplan setting info.
  
Returns
+++++++
::

  {
    $defhdr
  }
   
Example
+++++++
::

  $ curl -X PUT localhost:8081/dialplan/setting -d 'Test update'
  
  {"api_ver": "0.1", "timestamp": "2018-01-21T19:24:50.827410461Z", "statuscode": 200}
  
