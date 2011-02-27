= launch

* https://github.com/drbrain/launch
* http://docs.seattlerb.org/launch

== DESCRIPTION:

Launch allows agents and daemons launched by launchd to retrieve their list of
sockets.  Launchd is an open source framework for launching and managing
daemons, programs and scripts provided by Apple.

== FEATURES/PROBLEMS:

* Provides the Launch module for easy integration with existing programs
* Allows on-demand execution of ruby programs through launchd

== SYNOPSIS:

  require 'launch'

  class MyDaemon
    def initialize
      launch_checkin
      launch_sockets('MyDaemonSocket', TCPServer).each do |server|
        # handle requests, see sample/echo.rb
      end
    end
  end

  MyDaemon.new

== REQUIREMENTS:

* An installed launchd

== INSTALL:

  gem install launch

== DEVELOPERS:

After checking out the source, run:

  $ rake newb

This task will install any missing dependencies, run the tests/specs,
and generate the RDoc.

== LICENSE:

(The MIT License)

Copyright (c) 2011 Eric Hodel

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
