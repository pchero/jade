.. _pjsip_api:

.. _pjsip_aors:

/pjsip/aors
==================

Methods
-------
GET : Get all list of aors.

.. _get_pjsip_aors:

Method: GET
-----------
GET : Get all list of aors.

Call
++++
::

  GET ^/pjsip/aors
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          "object_name": "<string>",
          "object_type": "<string>",
     
          "endpoint_name": "<string>",
          "contacts": "<string>",
          "contacts_registered": <number>,
          "total_contacts": <number>,
          
          "authenticate_qualify": "<string>",
          
          "default_expiration": <number>,
          
          "mailboxes": "<string>",
     
          "max_contacts": <number>,
          "maximum_expiration": <number>,
          "minimum_expiration": <number>,
     
          "outbound_proxy": "<string>",
          
          "qualify_frequency": <number>,
          "qualify_timeout": <number>,
          
          "remove_existing": "<string>",
          "support_path": "<string>",
          
          "voicemail_extension": "<string>",
          
          "tm_update": "<timestamp>"
        },
        ...
      ]
    }
  }

* ``list``
    * ``object_name``:
   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/aors
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "authenticate_qualify": "false",
                "contacts": "",
                "contacts_registered": 0,
                "default_expiration": 3600,
                "endpoint_name": "199",
                "mailboxes": "",
                "max_contacts": 1,
                "maximum_expiration": 7200,
                "minimum_expiration": 60,
                "object_name": "199",
                "object_type": "aor",
                "outbound_proxy": "",
                "qualify_frequency": 0,
                "qualify_timeout": 3,
                "remove_existing": "true",
                "support_path": "false",
                "tm_update": "2017-12-19T20:03:34.948197060Z",
                "total_contacts": 0,
                "voicemail_extension": ""
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:03:41.263715886Z"
  }

.. _pjsip_aors_detail:

/pjsip/aors/<detail>
============================

Methods
-------
GET : Get detail aor info of given aor info.

.. _get_pjsip_aors_detail:

Method: GET
-----------
GET : Get detail aor info of given aor info.


Call
++++
::

  GET ^/pjsip/aors/<detail>
  
Method parameters

* ``detail``: aor name.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "object_name": "<string>",
      "object_type": "<string>",
      
      "endpoint_name": "<string>",
      "contacts": "<string>",
      "contacts_registered": <number>,
      "total_contacts": <number>,
      
      "authenticate_qualify": "<string>",
      
      "default_expiration": <number>,
      
      "mailboxes": "<string>",
      
      "max_contacts": <number>,
      "maximum_expiration": <number>,
      "minimum_expiration": <number>,
      
      "outbound_proxy": "<string>",
      
      "qualify_frequency": <number>,
      "qualify_timeout": <number>,
      
      "remove_existing": "<string>",
      "support_path": "<string>",
      
      "voicemail_extension": "<string>",
      
      "tm_update": "<timestamp>"
    }
  }

* ``object_name``: aor name.
* ``object_type``: type. Always would be "aor".

   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/aors/pjagent-01
  
  {
    "api_ver": "0.1",
    "result": {
        "authenticate_qualify": "false",
        "contacts": "pjagent-01/sip:35740219@10.12.118.159:49027",
        "contacts_registered": 1,
        "default_expiration": 3600,
        "endpoint_name": "pjagent-01",
        "mailboxes": "",
        "max_contacts": 1,
        "maximum_expiration": 7200,
        "minimum_expiration": 60,
        "object_name": "pjagent-01",
        "object_type": "aor",
        "outbound_proxy": "",
        "qualify_frequency": 0,
        "qualify_timeout": 3,
        "remove_existing": "true",
        "support_path": "false",
        "tm_update": "2017-12-19T20:03:36.64487822Z",
        "total_contacts": 1,
        "voicemail_extension": ""
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:21:29.736727558Z"
  }

.. _pjsip_auths:
  
/pjsip/auths
==================

Methods
-------
GET : Get all list of auths.

.. _get_pjsip_auths:

Method: GET
-----------
GET : Get all list of auths.

Call
++++
::

  GET ^/pjsip/auths
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          "object_name": "<string>",
          "object_type": "<string>",

          "username": "<string>",
          "auth_type": "<string>",
          "password": "<string>",
          "md5_cred": "<string>",

          "realm": "<string>",
          "nonce_lifetime": <number>,
          
          "endpoint_name": "<string>",

          "tm_update": "<timestamp>"
        },
        ...
      ]
    }
  }

* ``list``
    * ``object_name``: auth name.
    * ``object_type``: type. Always will be "auth".
   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/auths
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "auth_type": "userpass",
                "endpoint_name": "pjagent-01",
                "md5_cred": "",
                "nonce_lifetime": 32,
                "object_name": "pjagent-01",
                "object_type": "auth",
                "password": "pjagent-01",
                "realm": "",
                "tm_update": "2017-12-19T20:03:36.42243261Z",
                "username": "pjagent-01"
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:26:09.770460034Z"
  }

.. _pjsip_auths_detail:

/pjsip/auths/<detail>
====================

Methods
-------
GET : Get detail auth info of given auth info.

.. _get_pjsip_auths_detail:

Method: GET
-----------
GET : Get detail auth info of given auth info.


Call
++++
::

  GET ^/pjsip/auths/<detail>
  
Method parameters

* ``detail``: auth name.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "object_name": "<string>",
      "object_type": "<string>",

      "username": "<string>",
      "auth_type": "<string>",
      "password": "<string>",
      "md5_cred": "<string>",

      "realm": "<string>",
      "nonce_lifetime": <number>,
      
      "endpoint_name": "<string>",

      "tm_update": "<timestamp>"
    }
  }

* ``object_name``: aor name.
* ``object_type``: type. Always would be "aor".

   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/auths/pjagent-01
  
  {
    "api_ver": "0.1",
    "result": {
        "auth_type": "userpass",
        "endpoint_name": "pjagent-01",
        "md5_cred": "",
        "nonce_lifetime": 32,
        "object_name": "pjagent-01",
        "object_type": "auth",
        "password": "pjagent-01",
        "realm": "",
        "tm_update": "2017-12-19T20:03:36.42243261Z",
        "username": "pjagent-01"
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:34:54.165290324Z"
  }

.. _pjsip_contacts:
  
/pjsip/contacts
===============

Methods
-------
GET : Get all list of contacts.

.. _get_pjsip_contacts:

Method: GET
-----------
GET : Get all list of contacts.

Call
++++
::

  GET ^/pjsip/contacts
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          "id": "<string>",
          "call_id": "<string>",
          
          "uri": "<string>",
          "aor": "<string>",
          "endpoint_name": "<string>",
          "status": "<string>",
          
          "qualify_frequency": 0,
          "qualify_timout": 3,
          
          "authentication_qualify": 0,
          
          
          "outbound_proxy": "<string>",
          "path": "<string>",
          
          "reg_expire": 1513714265,
          "round_trip_usec": "<string>",
          
          "user_agent": "<string>",
          "via_address": "<string>",
          
          "tm_update": "<string>"
        },
        ...
      ]
    }
  }

* ``list``
    * ``object_name``: auth name.
    * ``object_type``: type. Always will be "auth".
   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/contacts
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "aor": "pjagent-01",
                "authentication_qualify": 0,
                "call_id": "c67ef58c-3b65-44cd-a9a8-784e2af542d3",
                "endpoint_name": "pjagent-01",
                "id": "pjagent-01;@116e48d167ff94856cbc9009cdc735b2",
                "outbound_proxy": "",
                "path": "",
                "qualify_frequency": 0,
                "qualify_timout": 3,
                "reg_expire": 1513714265,
                "round_trip_usec": "0",
                "status": "Unknown",
                "tm_update": "2017-12-19T20:29:36.751968068Z",
                "uri": "sip:35740219@10.12.118.159:49027",
                "user_agent": "Blink 3.0.3 (Linux)",
                "via_address": "10.12.118.159:49027"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:38:09.405901164Z"
  }


.. _pjsip_contacts_detail:

/pjsip/contacts/<detail>
========================

Methods
-------
GET : Get detail contact info of given contact info.

.. _get_pjsip_contacts_detail:

Method: GET
-----------
GET : Get detail contact info of given contact info.

Call
++++
::

  GET ^/pjsip/contacts/<detail>
  
Method parameters

* ``detail``: uri encoded contact id.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "id": "<string>",
      "call_id": "<string>",
      
      "uri": "<string>",
      "aor": "<string>",
      "endpoint_name": "<string>",
      "status": "<string>",
      
      "qualify_frequency": 0,
      "qualify_timout": 3,
      
      "authentication_qualify": 0,
      
      
      "outbound_proxy": "<string>",
      "path": "<string>",
      
      "reg_expire": 1513714265,
      "round_trip_usec": "<string>",
      
      "user_agent": "<string>",
      "via_address": "<string>",
      
      "tm_update": "<string>"
    }
  }

* ``id``: 

   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/contacts/pjagent-01%3B%40116e48d167ff94856cbc9009cdc735b2
  
  {
    "api_ver": "0.1",
    "result": {
        "aor": "pjagent-01",
        "authentication_qualify": 0,
        "call_id": "c67ef58c-3b65-44cd-a9a8-784e2af542d3",
        "endpoint_name": "pjagent-01",
        "id": "pjagent-01;@116e48d167ff94856cbc9009cdc735b2",
        "outbound_proxy": "",
        "path": "",
        "qualify_frequency": 0,
        "qualify_timout": 3,
        "reg_expire": 1513716547,
        "round_trip_usec": "N/A",
        "status": "Unknown",
        "tm_update": "2017-12-19T20:45:42.698151858Z",
        "uri": "sip:35740219@10.12.118.159:49027",
        "user_agent": "Blink 3.0.3 (Linux)",
        "via_address": "10.12.118.159:49027"
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:51:43.977637876Z"
  }


.. _pjsip_endpoints:
  
/pjsip/endpoints
================

Methods
-------
GET : Get all list of endpoints.


.. _get_pjsip_endpoints:

Method: GET
-----------
GET : Get all list of endpoints.

Call
++++
::

  GET ^/pjsip/endpoints
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "list": [
        {
          ...
        },
        ...
      ]
    }
  }

   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/endpoints
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "account_code": "",
                "acl": "",
                "active_channels": "",
                "aggregate_mwi": "true",
                "allow": "(codec2|g723|ulaw|alaw|gsm|g726|g726aal2|adpcm|slin|slin|slin|slin|slin|slin|slin|slin|slin|lpc10|g729|speex|speex|speex|ilbc|g722|siren7|siren14|testlaw|g719|opus|jpeg|png|h261|h263|h263p|h264|mpeg4|vp8|vp9|red|t140|t38|silk|silk|silk|silk)",
                "allow_overlap": "true",
                "allow_subscribe": "true",
                "allow_transfer": "true",
                "aors": "pjagent-01",
                "asymmetric_rtp_codec": "false",
                "auth": "pjagent-01",
                "bind_rtp_to_media_address": "false",
                "bundle": "false",
                "call_group": "",
                "caller_id": "<unknown>",
                "caller_id_privacy": "allowed_not_screened",
                "caller_id_tag": "",
                "connected_line_method": "invite",
                "contact_acl": "",
                "context": "demo",
                "cos_audio": 0,
                "cos_video": 0,
                "device_state": "Not in use",
                "device_state_busy_at": 0,
                "direct_media": "true",
                "direct_media_glare_mitigation": "none",
                "direct_media_method": "invite",
                "disable_direct_media_on_nat": "false",
                "disallow": "",
                "dtls_ca_file": "",
                "dtls_ca_path": "",
                "dtls_cert_file": "",
                "dtls_cipher": "",
                "dtls_fingerprint": "SHA-256",
                "dtls_private_key": "",
                "dtls_rekey": 0,
                "dtls_setup": "active",
                "dtls_verify": "No",
                "dtmf_mode": "rfc4733",
                "fax_detect": "false",
                "fax_detect_time": 0,
                "force_avp": "false",
                "force_rport": "true",
                "from_domain": "",
                "from_user": "",
                "g_726_non_standard": "false",
                "ice_support": "false",
                "identify_by": "username,ip",
                "inband_progress": "false",
                "incoming_mwi_mailbox": "pjagent-01@vm-demo",
                "language": "",
                "mailboxes": "pjagent-01@vm-demo",
                "max_audio_streams": 1,
                "max_video_streams": 1,
                "media_address": "",
                "media_encryption": "no",
                "media_encryption_optimistic": "false",
                "media_use_received_transport": "false",
                "message_context": "",
                "moh_passthrough": "false",
                "moh_suggest": "default",
                "mwi_from_user": "",
                "mwi_subscribe_replaces_unsolicited": "false",
                "named_call_group": "",
                "named_pickup_group": "",
                "notify_early_inuse_ringing": "false",
                "object_name": "pjagent-01",
                "object_type": "endpoint",
                "one_touch_recording": "false",
                "outbound_auth": "",
                "outbound_proxy": "",
                "pickup_group": "",
                "preferred_codec_only": "false",
                "record_off_feature": "automixmon",
                "record_on_feature": "automixmon",
                "redirect_method": "",
                "refer_blind_progress": "true",
                "rel_100": "yes",
                "rewrite_contact": "false",
                "rpid_immediate": "false",
                "rtcp_mux": "false",
                "rtp_engine": "asterisk",
                "rtp_ipv6": "false",
                "rtp_keepalive": 0,
                "rtp_symmetric": "false",
                "rtp_timeout": 0,
                "rtp_timeout_hold": 0,
                "sdp_owner": "-",
                "sdp_session": "Asterisk",
                "send_diversion": "true",
                "send_pai": "false",
                "send_rpid": "false",
                "set_var": "",
                "srtp_tag32": "false",
                "sub_min_expiry": "0",
                "subscribe_context": "",
                "t38_udptl": "false",
                "t38_udptl_ec": "none",
                "t38_udptl_ipv6": "false",
                "t38_udptl_maxdatagram": 0,
                "t38_udptl_nat": "false",
                "timers": "yes",
                "timers_min_se": "90",
                "timers_sess_expires": 1800,
                "tm_update": "2017-12-19T20:45:42.632334496Z",
                "tone_zone": "",
                "tos_audio": 0,
                "tos_video": 0,
                "transport": "transport-udp",
                "trust_id_inbound": "false",
                "trust_id_outbound": "false",
                "use_avpf": "false",
                "use_ptime": "false",
                "user_eq_phone": "false",
                "voicemail_extension": "",
                "webrtc": "no"
            },
            ...
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T20:55:13.546370914Z"
  }


.. _pjsip_endpoints_detail:

/pjsip/endpoints/<detail>
=========================

Methods
-------
GET : Get detail endpoint info of given endpoint info.


.. _get_pjsip_endpoints_detail:

Method: GET
-----------
GET : Get detail endpoint info of given endpoint info.

Call
++++
::

  GET ^/pjsip/endpoints/<detail>
  
Method parameters

* ``detail``: uri encoded endpoint name.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      ...
    }
  }


   
Example
+++++++
::

  $ curl -X GET localhost:8081/pjsip/endpoints/pjagent-01
  
  {
    "api_ver": "0.1",
    "result": {
        "account_code": "",
        "acl": "",
        "active_channels": "",
        "aggregate_mwi": "true",
        "allow": "(codec2|g723|ulaw|alaw|gsm|g726|g726aal2|adpcm|slin|slin|slin|slin|slin|slin|slin|slin|slin|lpc10|g729|speex|speex|speex|ilbc|g722|siren7|siren14|testlaw|g719|opus|jpeg|png|h261|h263|h263p|h264|mpeg4|vp8|vp9|red|t140|t38|silk|silk|silk|silk)",
        "allow_overlap": "true",
        "allow_subscribe": "true",
        "allow_transfer": "true",
        "aors": "pjagent-01",
        "asymmetric_rtp_codec": "false",
        "auth": "pjagent-01",
        "bind_rtp_to_media_address": "false",
        "bundle": "false",
        "call_group": "",
        "caller_id": "<unknown>",
        "caller_id_privacy": "allowed_not_screened",
        "caller_id_tag": "",
        "connected_line_method": "invite",
        "contact_acl": "",
        "context": "demo",
        "cos_audio": 0,
        "cos_video": 0,
        "device_state": "Not in use",
        "device_state_busy_at": 0,
        "direct_media": "true",
        "direct_media_glare_mitigation": "none",
        "direct_media_method": "invite",
        "disable_direct_media_on_nat": "false",
        "disallow": "",
        "dtls_ca_file": "",
        "dtls_ca_path": "",
        "dtls_cert_file": "",
        "dtls_cipher": "",
        "dtls_fingerprint": "SHA-256",
        "dtls_private_key": "",
        "dtls_rekey": 0,
        "dtls_setup": "active",
        "dtls_verify": "No",
        "dtmf_mode": "rfc4733",
        "fax_detect": "false",
        "fax_detect_time": 0,
        "force_avp": "false",
        "force_rport": "true",
        "from_domain": "",
        "from_user": "",
        "g_726_non_standard": "false",
        "ice_support": "false",
        "identify_by": "username,ip",
        "inband_progress": "false",
        "incoming_mwi_mailbox": "pjagent-01@vm-demo",
        "language": "",
        "mailboxes": "pjagent-01@vm-demo",
        "max_audio_streams": 1,
        "max_video_streams": 1,
        "media_address": "",
        "media_encryption": "no",
        "media_encryption_optimistic": "false",
        "media_use_received_transport": "false",
        "message_context": "",
        "moh_passthrough": "false",
        "moh_suggest": "default",
        "mwi_from_user": "",
        "mwi_subscribe_replaces_unsolicited": "false",
        "named_call_group": "",
        "named_pickup_group": "",
        "notify_early_inuse_ringing": "false",
        "object_name": "pjagent-01",
        "object_type": "endpoint",
        "one_touch_recording": "false",
        "outbound_auth": "",
        "outbound_proxy": "",
        "pickup_group": "",
        "preferred_codec_only": "false",
        "record_off_feature": "automixmon",
        "record_on_feature": "automixmon",
        "redirect_method": "",
        "refer_blind_progress": "true",
        "rel_100": "yes",
        "rewrite_contact": "false",
        "rpid_immediate": "false",
        "rtcp_mux": "false",
        "rtp_engine": "asterisk",
        "rtp_ipv6": "false",
        "rtp_keepalive": 0,
        "rtp_symmetric": "false",
        "rtp_timeout": 0,
        "rtp_timeout_hold": 0,
        "sdp_owner": "-",
        "sdp_session": "Asterisk",
        "send_diversion": "true",
        "send_pai": "false",
        "send_rpid": "false",
        "set_var": "",
        "srtp_tag32": "false",
        "sub_min_expiry": "0",
        "subscribe_context": "",
        "t38_udptl": "false",
        "t38_udptl_ec": "none",
        "t38_udptl_ipv6": "false",
        "t38_udptl_maxdatagram": 0,
        "t38_udptl_nat": "false",
        "timers": "yes",
        "timers_min_se": "90",
        "timers_sess_expires": 1800,
        "tm_update": "2017-12-19T20:45:42.632334496Z",
        "tone_zone": "",
        "tos_audio": 0,
        "tos_video": 0,
        "transport": "transport-udp",
        "trust_id_inbound": "false",
        "trust_id_outbound": "false",
        "use_avpf": "false",
        "use_ptime": "false",
        "user_eq_phone": "false",
        "voicemail_extension": "",
        "webrtc": "no"
    },
    "statuscode": 200,
    "timestamp": "2017-12-19T21:07:25.80668047Z"
  }



