.. _voicemail_api:


.. _voicemail_setting:

/voicemail/setting
==================

Methods
-------
GET : Get current voicemail setting.

PUT : Update voicemail setting.

.. _get_voicemail_setting:

Method: GET
-----------
GET : Get current voicemail setting.

This result does not mean to currently running setting. 
It shows only setting file.

Call
++++
::

  GET ^/voicemail/setting
  
Returns
+++++++
::

  {
    $defhdr,
    "result": {
      "general": {
        ...
      },
      "zonemessages": {
        ...
      },
      "<context>": {
        "<mailbox>": "<string>"
      },
      ...
    }
  }

Return parameters

* ``general`` : The general option. See detail Asterisk's voicemail setting options.
* ``zonemessages``: The zonemessages option. See detail Asterisk's voicemail setting options.
* ``context``: Voicemail context option. See detail Asterisk's voicemail context option.
    * ``mailbox``: Voicemail mailbox options. See detail Asterisk's voicemail mailbox option.
   
Example
+++++++
::

  $ curl -X GET localhost:8081/voicemail/setting
  
  {
    "api_ver": "0.1",
    "result": {
        "default": {
            "1234": "> 4242,Example Mailbox,root@localhost"
        },
        "general": {
            "attach": "yes",
            "emaildateformat": "%A, %B %d, %Y at %r",
            "format": "wav49|gsm|wav",
            "maxlogins": "3",
            "maxsilence": "10",
            "pagerdateformat": "%A, %B %d, %Y at %r",
            "sendvoicemail": "yes",
            "serveremail": "asterisk",
            "silencethreshold": "128",
            "skipms": "3000"
        },
        "other": {
            "1234": "> 5678,Company2 User,root@localhost"
        },
        "vm-demo": {
            "6001": "> 8762,Alice Jones,alice@example.com,alice2@example.com,attach=no|tz=central|maxmsg=10",
            "6002": "> 9271,Bob Smith,bob@example.com,bob2@example.com,attach=yes|tz=eastern",
            "6005": ",Bob Smith,bob@example.com,bob2@example.com,tz=eastern|attach=Yes|attachfmt=|saycid=No|saydurationm=2|dialout=|callback=|exitcontext=|review=No|operator=No|envelope=Yes|delete=No|volgain=0.000000|",
            "agent-01": "> ,agent 01,agent-01@example.com,,",
            "pjagent-01": "> ,pjagent 01,pjagent-01@example.com,,"
        },
        "zonemessages": {
            "central": "America/Chicago|'vm-received' Q 'digits/at' IMp",
            "central24": "America/Chicago|'vm-received' q 'digits/at' H N 'hours'",
            "eastern": "America/New_York|'vm-received' Q 'digits/at' IMp",
            "european": "Europe/Copenhagen|'vm-received' a d b 'digits/at' HM",
            "military": "Zulu|'vm-received' q 'digits/at' H N 'hours' 'phonetic/z_p'"
        }
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T11:03:54.209580717Z"
  }

.. _put_voicemail_setting:

Method: PUT
-----------
PUT : Update voicemail setting.

Update only setting file. To adapt to module, required module reload.

Call
++++
::

  PUT ^/voicemail/setting
  
  {
    ...
  }

Data parameters

* voicemail setting info.
  
Returns
+++++++
::

  {
    $defhdr
  }
   
Example
+++++++
::

  $ curl -X PUT localhost:8081/voicemail/setting -d
  {
    "default": {
      "1234": "> 4242,Example Mailbox,root@localhost"
    },
    "general": {
      "attach": "yes",
      "emaildateformat": "%A, %B %d, %Y at %r",
      "format": "wav49|gsm|wav",
      "maxlogins": "3",
      "maxsilence": "10",
      "pagerdateformat": "%A, %B %d, %Y at %r",
      "sendvoicemail": "yes",
      "serveremail": "asterisk",
      "silencethreshold": "128",
      "skipms": "3000"
    },
    "other": {
      "1234": "> 5678,Company2 User,root@localhost"
    },
    "vm-demo": {
      "6001": "> 8762,Alice Jones,alice@example.com,alice2@example.com,attach=no|tz=central|maxmsg=10",
      "6002": "> 9271,Bob Smith,bob@example.com,bob2@example.com,attach=yes|tz=eastern",
      "6005": ",Bob Smith,bob@example.com,bob2@example.com,tz=eastern|attach=Yes|attachfmt=|saycid=No|saydurationm=2|dialout=|callback=|exitcontext=|review=No|operator=No|envelope=Yes|delete=No|volgain=0.000000|",
      "agent-01": "> ,agent 01,agent-01@example.com,,",
      "pjagent-01": "> ,pjagent 01,pjagent-01@example.com,,"
    },
    "zonemessages": {
      "central": "America/Chicago|'vm-received' Q 'digits/at' IMp",
      "central24": "America/Chicago|'vm-received' q 'digits/at' H N 'hours'",
      "eastern": "America/New_York|'vm-received' Q 'digits/at' IMp",
      "european": "Europe/Copenhagen|'vm-received' a d b 'digits/at' HM",
      "military": "Zulu|'vm-received' q 'digits/at' H N 'hours' 'phonetic/z_p'"
    }
  }
  
  {"api_ver": "0.1", "statuscode": 200, "timestamp": "2017-12-18T11:23:36.669590706Z"}
  
  
.. _voicemail_settings:

/voicemail/settings
===================

Methods
-------
GET : Get all backup voicemail settings.

.. _get_voicemail_settings:

Method: GET
-----------
GET : Get all backup voicemail settings.


Call
++++
::

  GET ^/voicemail/settings
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        "<filename>": {
          ...
        }
      ]
    }
  }

Return parameters

* ``list``: array of backup files.
    * ``filename``: backup filename.
   
Example
+++++++
::

  $ curl -X GET localhost:8081/voicemail/settings
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "voicemail.conf.2017-12-18T11:23:36.665818303Z": {
                    "default": {
                        "1234": "> 4242,Example Mailbox,root@localhost"
                    },
                    "general": {
                        "attach": "yes",
                        "emaildateformat": "%A, %B %d, %Y at %r",
                        "format": "wav49|gsm|wav",
                        "maxlogins": "3",
                        "maxsilence": "10",
                        "pagerdateformat": "%A, %B %d, %Y at %r",
                        "sendvoicemail": "yes",
                        "serveremail": "asterisk",
                        "silencethreshold": "128",
                        "skipms": "3000"
                    },
                    "other": {
                        "1234": "> 5678,Company2 User,root@localhost"
                    },
                    "vm-demo": {
                        "6001": "> 8762,Alice Jones,alice@example.com,alice2@example.com,attach=no|tz=central|maxmsg=10",
                        "6002": "> 9271,Bob Smith,bob@example.com,bob2@example.com,attach=yes|tz=eastern",
                        "6005": ",Bob Smith,bob@example.com,bob2@example.com,tz=eastern|attach=Yes|attachfmt=|saycid=No|saydurationm=2|dialout=|callback=|exitcontext=|review=No|operator=No|envelope=Yes|delete=No|volgain=0.000000|",
                        "agent-01": "> ,agent 01,agent-01@example.com,,",
                        "pjagent-01": "> ,pjagent 01,pjagent-01@example.com,,"
                    },
                    "zonemessages": {
                        "central": "America/Chicago|'vm-received' Q 'digits/at' IMp",
                        "central24": "America/Chicago|'vm-received' q 'digits/at' H N 'hours'",
                        "eastern": "America/New_York|'vm-received' Q 'digits/at' IMp",
                        "european": "Europe/Copenhagen|'vm-received' a d b 'digits/at' HM",
                        "military": "Zulu|'vm-received' q 'digits/at' H N 'hours' 'phonetic/z_p'"
                    }
                }
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T11:23:56.197482423Z"
  }


.. _voicemail_settings_detail:

/voicemail/settings/<detail>
============================

Methods
-------
GET : Get all backup voicemail settings.

DELETE : Delete given backup voicemail setting.

.. _get_voicemail_settings_detail:

Method: GET
-----------
GET : Get backup voicemail settings of given info.


Call
++++
::

  GET ^/voicemail/settings/<detail>
  
Method parameters

* ``detail``: backup filename.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      ...
    }
  }

See detail Asterisk's voicemail setting.
   
Example
+++++++
::

  $ curl -X GET localhost:8081/voicemail/settings/voicemail.conf.2017-12-18T11:23:36.665818303Z
  
  {
    "api_ver": "0.1",
    "result": {
        "list": {
            "default": {
                "1234": "> 4242,Example Mailbox,root@localhost"
            },
            "general": {
                "attach": "yes",
                "emaildateformat": "%A, %B %d, %Y at %r",
                "format": "wav49|gsm|wav",
                "maxlogins": "3",
                "maxsilence": "10",
                "pagerdateformat": "%A, %B %d, %Y at %r",
                "sendvoicemail": "yes",
                "serveremail": "asterisk",
                "silencethreshold": "128",
                "skipms": "3000"
            },
            "other": {
                "1234": "> 5678,Company2 User,root@localhost"
            },
            "vm-demo": {
                "6001": "> 8762,Alice Jones,alice@example.com,alice2@example.com,attach=no|tz=central|maxmsg=10",
                "6002": "> 9271,Bob Smith,bob@example.com,bob2@example.com,attach=yes|tz=eastern",
                "6005": ",Bob Smith,bob@example.com,bob2@example.com,tz=eastern|attach=Yes|attachfmt=|saycid=No|saydurationm=2|dialout=|callback=|exitcontext=|review=No|operator=No|envelope=Yes|delete=No|volgain=0.000000|",
                "agent-01": "> ,agent 01,agent-01@example.com,,",
                "pjagent-01": "> ,pjagent 01,pjagent-01@example.com,,"
            },
            "zonemessages": {
                "central": "America/Chicago|'vm-received' Q 'digits/at' IMp",
                "central24": "America/Chicago|'vm-received' q 'digits/at' H N 'hours'",
                "eastern": "America/New_York|'vm-received' Q 'digits/at' IMp",
                "european": "Europe/Copenhagen|'vm-received' a d b 'digits/at' HM",
                "military": "Zulu|'vm-received' q 'digits/at' H N 'hours' 'phonetic/z_p'"
            }
        }
    },
    "statuscode": 200,
    "timestamp": "2017-12-18T11:30:24.953556796Z"
  }

  
.. _delete_voicemail_settings_detail:
  
Method: DELETE
--------------
DELETE : Delete given backup voicemail setting.


Call
++++
::

  DELETE ^/voicemail/settings/<detail>
  
Method parameters

* ``detail``: backup filename.
  
Returns
+++++++
::

  {
    $defhdr
  }
   
Example
+++++++
::

  $ curl -X DELETE localhost:8081/voicemail/settings/voicemail.conf.2017-12-17T02:25:19.494086661Z
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-18T20:52:30.462052487Z"
  }

  

.. _voicemail_users:

/voicemail/users
================

Methods
-------
GET : Get list of all voicemail users.
POST : Create new voicemail user.

.. _get_voicemail_users:

Method: GET
-----------
Get list of all voicemail users.

Call
++++
::

  GET ^/voicemail/users
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "list": [
        {
          "id": "<string>", 
          "context": "<string>",
          "mailbox": "<string>",

          "email": "<string>",
          "full_name": "<string>",
          "pager": "<string>",

          "server_email": "<string>",
          "mail_command": "<string>",
          "attach_message": "<string>",
          "attachment_format": "<string>",
          "delete_message": "<string>",

          "call_operator": "<string>",
          "callback": "<string>",
          "can_review": "<string>",

          "dialout": "<string>",
          "exit_context": "<string>",
          "from_string": "<string>",
          "language": "<string>",

          "max_message_count": <integer>,
          "max_message_length": <integer>,
          "new_message_count": <integer>,
          "old_message_count": <integer>,

          "imap_flag": "<string>",
          "imap_port": "<string>",
          "imap_server": "<string>",
          "imap_user": "<string>",


          "say_cid": "<string>",
          "say_duration_minimum": <integer>,
          "say_envelope": "<string>",

          "timezone": "<string>",
          
          "unique_id": "<string>",
          "volume_gain": <real>,

          "tm_update": "<string>"
        },
         ...
      ]
    }
  }

Return parameters

* ``list`` : array of items.
   * ``id``: user id.
   * ``context``: Context name.
   * ``mailbox``: Mailbox name.

   * ``email``: Email address(mail receiver)
   * ``full_name``: Full name
   * ``pager``: Pager info

   * ``server_email``: Server's mail address.(mail sender)
   * ``mail_command``: Mail sending command.
   * ``attach_message``: Voicemail attachment.
   * ``attachment_format``: Attached voicemail format.
   * ``delete_message``: Delete voicemail after send to email or not.
   
   * ``call_operator``: 
   * ``callback``: 
   * ``can_review``: 

   * ``dialout``: 
   * ``exit_context``: 
   * ``from_string``: 
   * ``language``: 

   * ``max_message_count``: Maximum message count.
   * ``max_message_length``:  Maximum message length.
   * ``new_message_count``: New message count.
   * ``old_message_count``: Old message count.

   * ``imap_flag``: 
   * ``imap_port``: 
   * ``imap_server``: 
   * ``imap_user``: 

   * ``say_cid``: 
   * ``say_duration_minimum``: 
   * ``say_envelope``: 

   * ``timezone``: 

   * ``unique_id``: 
   * ``volume_gain``: 

   * ``tm_update``: 

   
Example
+++++++
::

  curl -X GET 192.168.200.10:8081/voicemail/users
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "attach_message": "Yes",
                "attachment_format": "",
                "call_operator": "No",
                "callback": "",
                "can_review": "No",
                "context": "vm-demo",
                "delete_message": "No",
                "dialout": "",
                "email": "pjagent-01@example.com",
                "exit_context": "",
                "from_string": "",
                "full_name": "pjagent 01",
                "id": "pjagent-01@vm-demo",
                "imap_flag": "",
                "imap_port": "",
                "imap_server": "",
                "imap_user": "",
                "language": "",
                "mail_command": "/usr/sbin/sendmail -t",
                "mailbox": "pjagent-01",
                "max_message_count": 100,
                "max_message_length": 0,
                "new_message_count": 3,
                "old_message_count": 0,
                "pager": "",
                "say_cid": "No",
                "say_duration_minimum": 2,
                "say_envelope": "Yes",
                "server_email": "asterisk",
                "timezone": "",
                "tm_update": "2017-12-08T14:18:58.958489530Z",
                "unique_id": "",
                "volume_gain": 0.0
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-08T15:17:16.293060721Z"
  }
  

  
Method: POST
------------
Create new voicemail user.

Call
++++
::

  POST ^/voicemail/users
  
  {
    "context": "<string>", 
    "mailbox": "<string>", 
    "password": "<string>", 
    "full_name": "<string>", 
    "email": "<string>", 
    "pager": "<string>", 

    "timezone": "<string>", 
    "locale": "<string>", 
    "attach_message": "<string>", 
    "attachment_format": "<string>", 
    "say_cid": "<string>", 

    "cid_internal_contexts": "<string>", 
    "say_duration": "<string>", 
    "say_duration_minimum": "<string>", 
    "dialout": "<string>", 
    "send_voicemail": "<string>", 

    "search_contexts": "<string>", 
    "callback": "<string>", 
    "exit_context": "<string>", 
    "can_review": "<string>", 
    "call_operator": "<string>", 

    "say_envelope": "<string>", 
    "delete_message": "<string>", 
    "alias": "<string>", 
    "volume_gain": "<string>", 
    "next_after_cmd": "<string>", 

    "force_name": "<string>", 
    "force_greetings": "<string>", 
    "hide_from_dir": "<string>", 
    "temp_greet_warn": "<string>", 
    "password_location": "<string>", 

    "message_wrap": "<string>", 
    "min_password": "<string>", 
    "vm_password": "<string>", 
    "vm_new_password": "<string>", 
    "vm_pass_changed": "<string>", 

    "vm_reenter_password": "<string>", 
    "vm_mismatch": "<string>", 
    "vm_invalid_password": "<string>", 
    "vm_pls_try_again": "<string>", 
    "vm_prepend_timeout": "<string>", 

    "listen_control_forward_key": "<string>", 
    "listen_control_reverse_key": "<string>", 
    "listen_control_pause_key": "<string>", 
    "listen_control_restart_key": "<string>", 
    "listen_control_stop_key": "<string>", 

    "backup_deleted": "<string>"
  }

* ``context``: <required> Context name.
* ``mailbox``: <required> Mailbox name.
* ``password``: password.
* ``full_name``: full name.
* ``email``: email address.
* ``pager``: pager address.

* ``timezone``:
* ``locale``:
* ``attach_message``:
* ``attachment_format``:
* ``say_cid``:

* ``cid_internal_contexts``:
* ``say_duration``:
* ``say_duration_minimum``:
* ``dialout``:
* ``send_voicemail``:

* ``search_contexts``:
* ``callback``:
* ``exit_context``:
* ``can_review``:
* ``call_operator``:

* ``say_envelope``:
* ``delete_message``:
* ``alias``:
* ``volume_gain``:
* ``next_after_cmd``:

* ``force_name``:
* ``force_greetings``:
* ``hide_from_dir``:
* ``temp_greet_warn``:
* ``password_location``:

* ``message_wrap``:
* ``min_password``:
* ``vm_password``:
* ``vm_new_password``:
* ``vm_pass_changed``:

* ``vm_reenter_password``:
* ``vm_mismatch``:
* ``vm_invalid_password``:
* ``vm_pls_try_again``:
* ``vm_prepend_timeout``:

* ``listen_control_forward_key``:
* ``listen_control_reverse_key``:
* ``listen_control_pause_key``:
* ``listen_control_restart_key``:
* ``listen_control_stop_key``:

* ``backup_deleted``:
  
Returns
+++++++
::

   {
     $defhdr
   }

   
Example
+++++++
::

  $ curl -X POST localhost:8081/voicemail/users -d \
    '{"context": "test1", "mailbox": "testmail"}' 

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-15T23:07:53.444934604Z"
  }


.. _voicemail_users_detail:

/voicemail/users/<detail>
=========================

Methods
-------
GET : Get list of all voicemail users.

PUT : Update voicemail user info.

DELETE : Delete voicemail user.

.. _get_voicemail_users_detail:

Method: GET
-----------
Get detail info of given voicemail users.

Call
++++
::

  GET ^/voicemail/users/<detail>
  
Method parameter

* ``detail``: uri encoded voicemail user id.
  
Returns
+++++++
::

  {
    $defhdr,
    "reuslt": {
      "id": "<string>", 
      "context": "<string>",
      "mailbox": "<string>",

      "email": "<string>",
      "full_name": "<string>",
      "pager": "<string>",

      "server_email": "<string>",
      "mail_command": "<string>",
      "attach_message": "<string>",
      "attachment_format": "<string>",
      "delete_message": "<string>",

      "call_operator": "<string>",
      "callback": "<string>",
      "can_review": "<string>",

      "dialout": "<string>",
      "exit_context": "<string>",
      "from_string": "<string>",
      "language": "<string>",

      "max_message_count": <integer>,
      "max_message_length": <integer>,
      "new_message_count": <integer>,
      "old_message_count": <integer>,

      "imap_flag": "<string>",
      "imap_port": "<string>",
      "imap_server": "<string>",
      "imap_user": "<string>",


      "say_cid": "<string>",
      "say_duration_minimum": <integer>,
      "say_envelope": "<string>",

      "timezone": "<string>",
      
      "unique_id": "<string>",
      "volume_gain": <real>,

      "tm_update": "<string>"
    }
  }

Return parameters

* ``id``: user id.
* ``context``: Context name.
* ``mailbox``: Mailbox name.

* ``email``: Email address(mail receiver)
* ``full_name``: Full name
* ``pager``: Pager info

* ``server_email``: Server's mail address.(mail sender)
* ``mail_command``: Mail sending command.
* ``attach_message``: Voicemail attachment.
* ``attachment_format``: Attached voicemail format.
* ``delete_message``: Delete voicemail after send to email or not.

* ``call_operator``: 
* ``callback``: 
* ``can_review``: 

* ``dialout``: 
* ``exit_context``: 
* ``from_string``: 
* ``language``: 

* ``max_message_count``: Maximum message count.
* ``max_message_length``:  Maximum message length.
* ``new_message_count``: New message count.
* ``old_message_count``: Old message count.

* ``imap_flag``: 
* ``imap_port``: 
* ``imap_server``: 
* ``imap_user``: 

* ``say_cid``: 
* ``say_duration_minimum``: 
* ``say_envelope``: 

* ``timezone``: 

* ``unique_id``: 
* ``volume_gain``: 

* ``tm_update``: 

   
Example
+++++++
::

  $ curl -X GET localhost:8081/voicemail/users/6001%40vm-demo
  
  {
    "api_ver": "0.1",
    "result": {
        "attach_message": "No",
        "attachment_format": "",
        "call_operator": "No",
        "callback": "",
        "can_review": "No",
        "context": "vm-demo",
        "delete_message": "No",
        "dialout": "",
        "email": "alice@example.com",
        "exit_context": "",
        "from_string": "",
        "full_name": "Alice Jones",
        "id": "6001@vm-demo",
        "imap_flag": "",
        "imap_port": "",
        "imap_server": "",
        "imap_user": "",
        "language": "",
        "mail_command": "/usr/sbin/sendmail -t",
        "mailbox": "6001",
        "max_message_count": 10,
        "max_message_length": 0,
        "new_message_count": 1,
        "old_message_count": 0,
        "pager": "alice2@example.com",
        "say_cid": "No",
        "say_duration_minimum": 2,
        "say_envelope": "Yes",
        "server_email": "asterisk",
        "timezone": "central",
        "tm_update": "2017-12-22T21:06:44.567703838Z",
        "unique_id": "",
        "volume_gain": 0.0
    },
    "statuscode": 200,
    "timestamp": "2017-12-22T21:10:46.138227663Z"
  }

.. _put_voicemail_users_detail:

Method: PUT
-----------
Update detail info of given voicemail user.

Call
++++
::

  PUT ^/voicemail/users/<detail>
  
  {
    ...
  }
  
Method parameter

* ``detail``: uri encoded voicemail user id.

Data parameter

* ``...``: voicemail user info.
  
Returns
+++++++
::

  {
    $defhdr
  }
   
Example
+++++++
::

  $ curl -X PUT localhost:8081/voicemail/users/6001%40vm-demo -d
    '{"full_name": "pchero"}'

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-22T21:31:39.996111906Z"
  }

.. _delete_voicemail_users_detail:

Method: DELETE
--------------
Delete given voicemail user.

Call
++++
::

  DELETE ^/voicemail/users/<detail>
    
Method parameter

* ``detail``: uri encoded voicemail user id.
  
Returns
+++++++
::

  {
    $defhdr
  }
   
Example
+++++++
::

  $ curl -X DELETE localhost:8081/voicemail/users/6001%40vm-demo

  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-22T21:33:39.996111906Z"
  }

/voicemail/vms
==============

Methods
-------
GET : Get list of all voicemail info of given user.

Method: GET
-----------
GET : Get list of all voicemail info of given user.

Call
++++
::

  GET ^/voicemail/vms

Returns
+++++++
::

   {
     $defhdr,
     "reuslt": {
       "list": [
         {
            "msgname": "<string>",
            "msg_id": "<string>",

            "status": "<string>",
            "dir": "<string>",

            "context": "<string>",
            "exten": "<string>",
            "priority": "<string>",

            "callerchan": "<string>",
            "callerid": "<string>",
            "rdnis": "<string>",

            "category": "<string>",

            "flag": "<string>",
            "macrocontext": "<string>",

            "origdate": "<string>",
            "origmailbox": "<string>",
            "origtime": "<string>",
            "duration": "<string>"
         },
          ...
       ]
     }
   }

Return parameters

* ``list`` : array of items.
   * ``msgname``: Message name.
   * ``msg_id``: Message id.

   * ``status``: Message status.
   * ``dir``: Message directory.
   
   * ``context``: Message's context. When it was reaved.
   * ``exten``: Message's exten. When it was reaved.
   * ``priority``: Message's priority. When it was reaved.
   
   * ``callerchan``: Caller's channel name.
   * ``callerid``: Caller's id.
   * ``rdnis``:
   
   * ``category``:
   
   * ``flag``:
   * ``macrocontext``:
   
   * ``origdate``:
   * ``origmailbox``:
   * ``origtime``:
   * ``duration``:
  
  
Example
+++++++
::

  $ curl -X GET 192.168.200.10:8081/voicemail/vms\?context=vm-demo\&mailbox=pjagent-01
  
  {
    "api_ver": "0.1",
    "result": {
        "list": [
            {
                "callerchan": "PJSIP/pjagent-02-0000000a",
                "callerid": "pjagent-02",
                "category": "",
                "context": "voicemail-pjagent-01",
                "dir": "INBOX",
                "duration": "1",
                "exten": "s",
                "flag": "",
                "macrocontext": "",
                "msg_id": "1512659997-00000001",
                "msgname": "msg0001",
                "origdate": "Thu Dec  7 03:19:57 PM UTC 2017",
                "origmailbox": "pjagent-01",
                "origtime": "1512659997",
                "priority": "2",
                "rdnis": "unknown",
                "status": "unread"
            }
        ]
    },
    "statuscode": 200,
    "timestamp": "2017-12-08T15:31:37.267471272Z"
  }


.. _voicemail_vms_detail:
  
/voicemail/vms/<detail>
========================

Methods
-------
GET : Get binary of given voicemail.

DELETE : Delete given voicemail

.. _get_voicemail_vms_detail:

Method: GET
-----------
GET : Get binary of given voicemail.

Call
++++
::

  GET ^/voicemail/vms/<detail>?context=<context>&mailbox=<mailbox>&dir=<dir>

* ``detail``: Message name.

* ``context``: Message's context.
* ``mailbox``: Message's mailbox.
* ``dir``: Message's dir info.
  
Returns
+++++++
::

  Binary stream of given voicemail file.  
  
Example
+++++++
::

  $ curl -v -X GET 192.168.200.10:8081/voicemail/vms/msg0003\?context=vm-demo\&mailbox=pjagent-01\&dir=INBOX > /tmp/tmp.wav
  
    Note: Unnecessary use of -X or --request, GET is already inferred.
    % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                    Dload  Upload   Total   Spent    Left  Speed
    0     0    0     0    0     0      0      0 --:--:-- --:--:-- --:--:--     0*   Trying 192.168.200.10...
    0     0    0     0    0     0      0      0 --:--:-- --:--:-- --:--:--     0* Connected to 192.168.200.10 (192.168.200.10) port 8081 (#0)
    > GET /voicemail/vms/msg0003?context=vm-demo&mailbox=pjagent-01&dir=INBOX HTTP/1.1
    > Host: 192.168.200.10:8081
    > User-Agent: curl/7.47.0
    > Accept: */*
    > 
    < HTTP/1.1 200 OK
    < Content-Type: application/octet-stream
    < content-disposition: attachment; filename=msg0003.wav
    < Content-Length: 78444
    
    $ file /tmp/tmp.wav 
    /tmp/tmp.wav: RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, mono 8000 Hz


Method: DELETE
--------------
DELETE : Delete given voicemail.

Call
++++
::

  DELETE ^/voicemail/vms/<msgname>?context=<context>&mailbox=<mailbox>&dir=<dir>

* ``msgname``: Message name.

* ``context``: Message's context.
* ``mailbox``: Message's mailbox.
* ``dir``: Message's dir info.
  
Returns
+++++++
::

   {
     $defhdr
   }  
  
Example
+++++++
::

  $ curl -v -X DELETE 192.168.200.10:8081/voicemail/vms/msg0003\?context=vm-demo\&mailbox=pjagent-01\&dir=INBOX
  
  {
    "api_ver": "0.1",
    "statuscode": 200,
    "timestamp": "2017-12-08T21:13:14.145903468Z"
  }
  