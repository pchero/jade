.. _websocket:

**************
Websock basics
**************
The jade supports subscription through the Websocket.

Topic subscription
==================
To subscribe topic through the websocket, the client needs to send subscription request first.

::

  {
    "type": "<message type>",
    "topic": "<topic string>"
  }

* ``message type``: message type.
* * subscribe: add subsciption.
* * unsubscribe: remove subscription.
* ``topic``: Subscribe/Unsubscribe topic.

Example
+++++++

::

  {"type":"subscribe", "topic": "/"}
