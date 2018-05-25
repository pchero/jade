.. installation

************
Installation
************

Required libraries
==================

::

   Asterisk-13.6 or later
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

Install
=======

::

  $ cd <downloaded source directory>
  $ cd src/
  $ make
  $ sudo mkdir -p /opt/bin
  $ sudo mv jade_backend /opt/bin

