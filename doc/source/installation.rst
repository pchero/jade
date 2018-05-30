.. installation

************
Installation
************

SSL certification
=================
Need to create certification file for ssl connection.
::

  $ sudo openssl req -subj '/CN=US/O=Jade project/C=US' -x509 -nodes -days 3650 -newkey rsa:2048 -keyout /opt/etc/server.key -out /opt/etc/server.crt
  $ cat /opt/etc/server.crt /opt/etc/server.key > /opt/etc/jade.pem

Asterisk
========
Required Asterisk-13.6 or later

Asterisk settings
-----------------

chan_sip.so
+++++++++++
Disable the chan_sip module.
::

  asterisk*CLI> module unload chan_sip.so

res_pjsip.so
++++++++++++
Enable the res_pjsip module
::

  asterisk*CLI> module load res_pjsip.so
  

Config manager.conf
+++++++++++++++++++
Enable and set up the ami user.
::

   /etc/asterisk/manager.conf

  [general]
  enabled = yes
  port = 5038
  bindaddr = 0.0.0.0
  
  [admin]
  secret = admin  ; this is not secure. need to be changed.
  deny = 0.0.0.0/0.0.0.0
  permit = 127.0.0.1/255.255.255.0
  read = all
  write = all


Reload manager module
::

  asterisk*CLI> module reload manager 
  Module 'manager' reloaded successfully.

  asterisk*CLI> manager show user admin
  
            username: admin
              secret: <Set>
                 ACL: yes
           read perm: system,call,log,verbose,command,agent,user,config,dtmf,reporting,cdr,dialplan,originate,agi,cc,aoc,test,security,message,all
          write perm: system,call,log,verbose,command,agent,user,config,dtmf,reporting,cdr,dialplan,originate,agi,cc,aoc,test,security,message,all
     displayconnects: yes
  allowmultiplelogin: yes
           Variables: 


Config http.conf
++++++++++++++++
Need to configure the http.conf options.
::

  /etc/asterisk/http.conf
  
  [general]
  servername=Asterisk
  enabled=yes
  bindaddr=0.0.0.0
  bindport=8088
  tlsenable=yes
  tlsbindaddr=0.0.0.0:8089
  tlscertfile=/opt/etc/jade.pem


Required libraries
==================
::

   libsqlite3
   libevent2
   libbsd
   libjansson
   libevhtp
   libssl
   libzmq5
   libonig

Libevhtp
--------
Insall libevhtp
::

  $ mkdir -p /opt/src/libevhtp
  $ curl -s https://codeload.github.com/criticalstack/libevhtp/tar.gz/1.2.16 | tar xz -C /opt/src/libevhtp --strip-components=1
  $ cd /opt/src/libevhtp/build
  $ cmake ..
  $ make
  $ make install

Libwebsockets
-------------
Install libwebsockets
::

  $ mkdir -p /opt/src/libwebsockets
  $ curl -s https://codeload.github.com/warmcat/libwebsockets/tar.gz/v2.4.2 | tar xz -C /opt/src/libwebsockets --strip-components=1
  $ cd /opt/src/libwebsockets/build
  $ cmake -DLWS_WITH_LIBEVENT=1 ../
  $ make
  $ make install

Install jade
============

::

  $ cd <downloaded source directory>
  $ cd src/
  $ make
  $ sudo mkdir -p /opt/bin
  $ sudo mv jade_backend /opt/bin

