.. _dialplan_api:

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
  
