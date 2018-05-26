#!/bin/bash

if [ "$TRAVIS_OS_NAME" == "linux" ];
then
  sudo apt-get update -qq
  
  sudo apt-get install -y -qq build-essential openssl libxml2-dev libncurses5-dev uuid-dev sqlite3 libsqlite3-dev pkg-config curl libjansson-dev 
  sudo apt-get install -y -qq curl libssl-dev git cmake libevent-dev libjansson-dev libbsd-dev libzmq3-dev libonig-dev
  
  
  # libevhtp
  sudo mkdir -p /opt/src/libevhtp
  sudo curl -s https://codeload.github.com/criticalstack/libevhtp/tar.gz/1.2.16 | sudo tar xz -C /opt/src/libevhtp --strip-components=1
  sudo mkdir -p /opt/src/libevhtp/build
  cd /opt/src/libevhtp/build
  sudo cmake ..
  sudo make
  sudo make install


  # libwebsockets  
  sudo mkdir -p /opt/src/libwebsockets
  sudo curl -s https://codeload.github.com/warmcat/libwebsockets/tar.gz/v2.4.2 | sudo tar xz -C /opt/src/libwebsockets --strip-components=1
  sudo mkdir -p /opt/src/libwebsockets/build
  cd /opt/src/libwebsockets/build
  sudo cmake -DLWS_WITH_LIBEVENT=1 ../
  sudo make
  sudo make install
  
fi

