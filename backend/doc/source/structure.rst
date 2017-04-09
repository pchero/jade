.. structure:

*********
Structure
*********

Basic structures
================

.. _agent_status:

Agent status
------------
Agent's status.

.. table:: Agent status

   =============== ===========
   Status          Detail
   =============== ===========
   AGENT_LOGGEDOFF Agent is not logged in.
   AGENT_IDLE      Agent is logged in status idle.
   AGENT_ONCALL    Agent is logged in and now on call.
   =============== ===========

.. _device_state:

Device state
------------
Device's state

.. table:: Device state

   =========== ===========
   State       Detail
   =========== ===========
   UNKNOWN     Device is valid but channel didn't know state.
   NOT_INUSE   Device is not used.
   BUSY        Device is in use.
   INVALID     Device is invalid.
   UNAVAILABLE Device is unavailable.
   RINGING     Device is ringing.
   RINGINUSE   Device is ringing *and* in use.
   ONHOLD      Device is on hold.
   =========== ===========

.. _peer_monitoring_status:

Peer monitoring status
----------------------
Peer's monitoring status.

.. table:: Peer monitoring sttus

  ========== ==============
  Status     Detail
  ========== ==============
  ========== ==============


.. _peer_status:

Peer status
-----------
Peer's status.

.. table:: Peer status

  ============ =============
  Status       Detail
  ============ =============
  Unknow       Unknwon status.
  Registered   Peer has been registered.
  Unregistered Peer has been unregistered.
  Rejected     Rejected.
  Lagged       Lagged.
  ============ =============
