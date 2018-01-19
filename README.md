Jade
====
The jade is a free and open source unified communication system, which can fulfill a variety of roles for a range of industries and organizations who wish to contact to large numbers of people by phone or else in a short space of time.

The jade has been built using an Asterisk so that it can support almost of Asterisk's features. 

The jade can be used by anyone who has a need for mass outbound/inbound calling, voicemail system, agent call distributing, call recording and IVR service. 
Or may be used by either companies who wish to make calls on their own behalf, or by SaaS (Software as a Service) companies that want to provide bulk dialling and SMS broadcasting facilities to their own customers.

## Features
* Restful API

* Event subscription via ZeroMQ and Websocket

* Strategical inbound/outbound call distributing
  * Call balancing
  * Strategical call retrying
  * Call monitoring
  * Result report
  * Scheduling dial

* Asterisk resource control
  * Channel
  * Voicemail
  * Agent
  * Queue
  * Park
  * PJSIP
  * SIP

* SMS(WIP)

* Mail(WIP)

* Chat(WIP)
* Video call(WIP)
* Conference call(WIP)

## Manual
* https://rawgit.com/pchero/jade/master/doc/build/html/index.html

## Related project
* jade-admin(https://github.com/pchero/jade-admin)
  * Angular based simple web application for jade.
  * Administrator tools for jade.
* jade-agent(https://github.com/pchero/jade-agent) - WIP
  * Angular based simple web application for jade.
  * Agent tools for jade.
  * WebRTC support

## Demo
* jade
  * You can try apis to here.
  * http://project.pchero21.com/jade/

* jade-admin
  * You can try jade-admin web application at here.
  * http://project.pchero21.com/jade-admin/
  * ![Screen Preview](https://raw.githubusercontent.com/pchero/jade-admin/master/jade-admin.png)

## Library issue
* libwebsocket
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
