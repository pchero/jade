.. ob_basic

**************
Outbound Basic
**************

Concept
=======

Principle
---------
The asterisk-outbound has a 5W1H principle.

.. figure:: _static/Basic_concept.png
   :align: center
   
   Basic concept

* Campaign : Determine why make a call(Why).
* Plan : Determine how/when to make a call(How/When).
* Destination : Determine who get a call after answer(Who).
* Dial list(Dial List Master) : Determine where/what call heading for(Where/What).


Features
========

Call balancing
--------------
The res_outbound supports call balancing.

With this feature, the res_outbound can managing the amount of outbound call depends on the destination's condition.

For example, if the destination is queue, it calculate waiting agent's number and queue's performance.


Stratigical retrying
--------------------
The res_outbound supports the stratigical retrying.

The user can set retry counts and retry delay time for each call.


Customer info based dial list
-----------------------------
The res_outbound supports customer based dial list.

The user can set the customer's detail info such as name, detail, database key, email, ... not only for numbers
And supports several types of numbers.


Call capsulization
------------------
The res_outbound supports the call capsulization.

When the res_outbound making a call, the call kept all the resource info itself. So, if the other resources chagned later, it doesn't affect to the existing call.

It makes call-capsulization.


Dynamic resource management
---------------------------
The res_outbound supports the dynamic resource management.


Campaign scheduling
-------------------
The res_outbound supports the campaign scheduling.

The user can set the auto-start, auto-end.


Set variables
-------------
The res_outbound supports variables setting.

The user can set the variables for each resources.

It can deliver to the dialplan or SIP headers.


Monitoring
----------


Detail dial result
------------------
The res_outbound supports the detail dial result.

In the result, the user can check the all the call info(original info, dialing info, result info).


Resources
=========

Campaign
--------
* Set destination, plan, dial list(dlma).

Destination
-----------
* Call's transfer destination after customer has answered.

Plan
----
Dialling stragegy.
* UUI delivery.
* Dial mode
* Dialing control.

Dial list
---------
Customer info list.
* Dial number.
* UUI info.

Campaign
========
Determine why make a call(Why).

To dial to the customer, the user need to create the campaign first and need to assign the correct resources(destination, plan, dial list master).

Then the resources determine where/what/who/how/when make a call to the customer.

Status
------
The campaign has a status. See detail :ref:`campaign_status`.

Scheduling
----------
The user can sets scheduling for campaign. See detail :ref:`scheduling`.


Plan
====
Determine how/when to make a call(How/When).

Dial mode
---------
The user can sets the dial mode. See detail :ref:`dial_mode`.

Predictive
++++++++++
* Predict the number of customers to dial based on the deliver application/agent's answer rate.
* Predict how many call will be answered or not answered.
* Calculate possilbilties automatically.

Preview
+++++++
* The destination makes decision to make a call.
* Developing.

SMS
+++
* Send an SMS messages
* Developing.

Fax
+++
* Send a fax
* Developing.


Service level
-------------
Service level controling the amount of dailing.


.. _service_level:

::

   (Max available outbound call count) - (Current outbound call count) + (Service level) = (Available call count)

   If the (Available call count) is bigger than 0, make a call.


Max available outbound call count
+++++++++++++++++++++++++++++++++

The max available outbound call count is depends on the destination.

See detail :ref:`destination`.


.. _destination:

Destination
===========
Determine who get a call after answer(Who). And it determine the max available outbound call count.

Normaly, the destination suppose to be an agent. But in the asterisk system, the destination could be anything. For example, it could be extension or application(queue).

If the destination type is application, then the res_outbound calcaulate applciation's availablity.


Result
======
Result data devided by 5 sections.

Identity info, Dial info, Result info, Timestamp info, Related resources info.


