Jade
====
[![Travis Build Status](https://travis-ci.org/pchero/jade.svg)](https://travis-ci.org/pchero/jade)

The jade is a free and open source unified communication system, which can fulfill a variety of roles for a range of industries and organizations who wish to contact to large numbers of people by phone or else in a short space of time.

The jade has been built using an Asterisk so that it can support almost everything about the Asterisk's features.

The jade can be used by anyone who has a need for mass outbound/inbound calling, voicemail system, agent call distributing, call recording and IVR service. 
Or may be used by other companies who wish to make calls on their own behalf, or by SaaS (Software as a Service) companies that want to provide bulk dialling and SMS broadcasting facilities to their own customers.

## Features
- Restful API
- Event subscription via ZeroMQ and Websocket
- Strategical inbound/outbound call distributing
- Asterisk resource control
- Chatting
- Buddy managing
- WebRTC Call  
- Dialplan
- Trunk
- SMS(WIP)
- Mail(WIP)
- Video call(WIP)
- Conference call(WIP)
- XMPP(WIP)

## Manual
* https://jade-asterisk.readthedocs.io/en/latest/index.html

## Docker
* All pre-setting Asterisk and jade project on a docker is ready.
  * $ docker pull pchero/jade-asterisk
  * https://github.com/pchero/jade-asterisk
  * https://hub.docker.com/r/pchero/jade-asterisk/

## Demo
* jade
  * You can try apis to here.
  * http://project.pchero21.com:8081

* jade-me
  * https://github.com/pchero/jade-me
  * Simple user tool for jade.
  * You can try jade-me web application at here.
  * ID/Password
  * test1/test1, test2/test2, ...
  * https://project.pchero21.com:8202

## Related project
* jade-me(https://github.com/pchero/jade-me)
  * ![Alt Text](https://raw.githubusercontent.com/pchero/jade-me/master/jade-me.gif)
  * Angular based simple web application for jade user.
  * Chat, Voice call, Buddy services.

* jade-manager(https://github.com/pchero/jade-manager)
  * ![Screen Preview](https://raw.githubusercontent.com/pchero/jade-manager/master/jade-manager.png)
  * Angular based simple web application for jade manager.
  * Provide simplized of resource handling.
  * Easy to add/modify/delete the resources such as user, trunk.
  
* jade-admin(https://github.com/pchero/jade-admin)
  * ![Screen Preview](https://raw.githubusercontent.com/pchero/jade-admin/master/jade-admin.png)
  * Angular based simple web application for jade adminstrator.
  * Provide micro level of resource handling.
    
* jade-agent(https://github.com/pchero/jade-agent)
  * Angular based simple web application for jade.
  * Agent tools for jade.

## Library issue
* libevent
  * libevent-2.0.x has memory curruption problem. It's fixed in libevent-2.1.x.
    * It causes core crash the jade.
    * Need to use the libevnet-2.1.x or higher.
    * https://github.com/libevent/libevent/issues/335
 
* libwebsocket
  * Required libevent option. 
    * $ cd build  
    * $ cmake -DLWS_WITH_LIBEVENT=1 ../
    * $ make && sudo make install
  * v2.4.1 has memory leak problem(https://github.com/warmcat/libwebsockets/pull/1155)
    * Fixed in master branch.

* libevhtp
  * The libevhtp repository has been changed(https://github.com/criticalstack/libevhtp).
  * The libevhtp-v1.2.10, v1.2.11, v1.2.11n has memory leak problem(https://github.com/ellzey/libevhtp/issues/177).
  * It was fixed in develop branch. Recommand use the newst develop branch(https://github.com/ellzey/libevhtp.git).

## License
* BSD License

## Remember 2014.04.16
* Still remember 2014.04.16
* Rest in peace..
* https://en.wikipedia.org/wiki/Sinking_of_MV_Sewol
