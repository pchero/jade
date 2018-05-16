Jade
====
The jade is a free and open source unified communication system, which can fulfill a variety of roles for a range of industries and organizations who wish to contact to large numbers of people by phone or else in a short space of time.

The jade has been built using an Asterisk so that it can support almost of Asterisk's features. 

The jade can be used by anyone who has a need for mass outbound/inbound calling, voicemail system, agent call distributing, call recording and IVR service. 
Or may be used by either companies who wish to make calls on their own behalf, or by SaaS (Software as a Service) companies that want to provide bulk dialling and SMS broadcasting facilities to their own customers.

## Features
* Restful API

* Event subscription via ZeroMQ and Websocket

* Static/Dynamic dialplan configuration

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

* Chat

* Buddy

* SMS(WIP)
* Mail(WIP)
* Video call(WIP)
* Conference call(WIP)

## Manual
* https://rawgit.com/pchero/jade/master/doc/build/html/index.html

## Related project
* jade-manager(https://github.com/pchero/jade-manager)
  * Angular based simple web application for jade manager.
  * Provide simplized of resource handling.
  * Easy to add/modify/delete the resources such as user, trunk.
  
* jade-admin(https://github.com/pchero/jade-admin)
  * Angular based simple web application for jade adminstrator.
  * Provide micro level of resource handling.
  
* jade-me(https://github.com/pchero/jade-me)
  * Angular based simple web application for jade user.
  * Provide chat, buddy, call services.
  * Minor version of skype/wechat/facebook-messenger.
  
* jade-agent(https://github.com/pchero/jade-agent)
  * Angular based simple web application for jade.
  * Agent tools for jade.

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
  * ![Screen Preview](https://raw.githubusercontent.com/pchero/jade-me/master/jade-me.png)

* jade-manager
  * https://github.com/pchero/jade-manager
  * Simple manager tool for jade.
  * You can try jade-admin web application at here.
  * ID: admin
  * Password: admin
  * https://project.pchero21.com:8203
  * ![Screen Preview](https://raw.githubusercontent.com/pchero/jade-manager/master/jade-manager.png)

* jade-admin
  * https://github.com/pchero/jade-admin
  * Simple administrator tool for jade.
  * You can try jade-admin web application at here.
  * ID: admin
  * Password: admin
  * https://project.pchero21.com:8200
  * ![Screen Preview](https://raw.githubusercontent.com/pchero/jade-admin/master/jade-admin.png)

* jade-agent
  * https://github.com/pchero/jade-agent
  * Simple agent tool for jade.
  * Webrtc(jssip) support.
  * ID: test1
  * Password: admin
  * https://project.pchero21.com:8201
  * ![Screen Preview](https://raw.githubusercontent.com/pchero/jade-agent/master/jade-agent.png)

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
