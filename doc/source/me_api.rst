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

  $ curl -k https://localhost:8081/me/chats\?authtoken=32644555-58e5-4d03-9608-a66ed1651f12
  
  {
    "api_ver": "0.1",
    "result": [
        {
            "chat_info": {
                "members": [
                    "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                    "980404a2-f509-4140-9c92-96a018d1b61c"
                ],
                "tm_create": "2018-03-26T16:10:54.396798550Z",
                "tm_update": null,
                "type": 1,
                "uuid_creator": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                "uuid_owner": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
            },
            "detail": "test chat detail",
            "name": "test chat name",
            "tm_create": null,
            "tm_update": null,
            "uuid": "01c2f287-2e0a-4ae7-b8a8-1bc57f6fadb3"
        },
        {
            "detail": "test chat detail",
            "name": "test chat name",
            "tm_create": null,
            "tm_update": null,
            "uuid": "ba807369-b5ec-4ecf-8d18-64292fd3dcbc"
        }
    ],
    "statuscode": 200,
    "timestamp": "2018-03-26T16:12:06.425946246Z"
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
  
  $ curl -k -X GET https://localhost:8081/me/chats\?authtoken=d2718890-043f-43c8-9bf9-79c2602d2c81
  
  {
    "api_ver": "0.1",
    "result": [
        {
            "chat_info": {
                "members": [
                    "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                    "980404a2-f509-4140-9c92-96a018d1b61c"
                ],
                "tm_create": "2018-03-25T20:05:44.116027505Z",
                "tm_update": null,
                "type": null,
                "uuid_creator": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf",
                "uuid_owner": "59e3a7d5-b05f-43cd-abdf-db7009eed6cf"
            },
            "detail": "test chat detail",
            "name": "test chat name",
            "uuid": "7c817566-3c4a-4d97-b4f0-e29b2d2e1564"
        }
    ],
    "statuscode": 200,
    "timestamp": "2018-03-26T07:25:45.241009467Z"
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

  $ curl localhost:8081/me/info\?authtoken=f1b5cd49-3c54-4aad-8585-754e3ab1796c
   
  {
    "api_ver": "0.1",
    "result": {
        "contacts": [
            {
                "detail": "test target detail",
                "info": {
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
                    "auth": "auth-pjagent-01",
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
                    "device_state": "Unavailable",
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
                    "tm_update": "2018-02-04T02:01:04.409151584Z",
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
                "name": "test target",
                "target": "pjagent-01",
                "tm_create": "2018-02-04T01:34:34.972284886Z",
                "tm_update": null,
                "type": "pjsip_endpoint",
                "user_uuid": "b47977bc-913a-44d9-aaa9-33cc10970c30",
                "uuid": "a39b43a6-004f-472a-9c7b-80a9fbb91600"
            }
        ],
        "name": null,
        "password": "admin",
        "tm_create": "2018-02-03T02:28:55.640553565Z",
        "tm_update": null,
        "username": "admin",
        "uuid": "b47977bc-913a-44d9-aaa9-33cc10970c30"
    },
    "statuscode": 200,
    "timestamp": "2018-02-04T02:01:18.551806140Z"
  }

  
  