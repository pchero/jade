Jade
====
Asterisk based call manager.

This module makes easy to managing the calls.

## Call manapulating
* Call monitoring.
* Call handling.

## Stratigical outbound call managing
* Call balancing
* Stratigical retrying
* Customer info based dial list
* Call capsulization
* Dynamic resource management
* Campaign scheduling
* Set variables
* Monitoring
* Detail dial result

## Requirements
* Asterisk-13.6 or later
* Asterisk-14.3 or later
* Asterisk-manager module
* libsqlite3
* libevent2
* libbsd
* libjansson
* libevhtp(https://github.com/ellzey/libevhtp)
* libssl

## Manual
* https://rawgit.com/pchero/jade/master/backend/doc/build/html/index.html

## Library issue
* libevhtp
** The libevhtp-v1.2.10, v1.2.11, v1.2.11n has memory leak problem(https://github.com/ellzey/libevhtp/issues/177).
** It was fixed in develop branch. Recommand use the newst develop branch(https://github.com/ellzey/libevhtp.git).

## License
* BSD License

## Remember 2014.04.16
* Still remember 2014.04.16
* Rest in peace..
* https://en.wikipedia.org/wiki/Sinking_of_MV_Sewol
