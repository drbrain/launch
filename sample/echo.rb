require 'rubygems'
require 'launch'

##
# This is an echo server that runs using launchd on port 12345.
#
# To start, run:
#
#   ruby echo.rb net.segment7.launch.echo.plist
#   launchctl load net.segment7.launch.echo.plist
#
# To use the echo server run:
#
#   telnet localhost 12345
#
# To quit the echo server type ^] followed by ^D
#
# To stop run:
#
#   launchctl unload net.segment7.launch.echo.plist

class Echo

  include Launch

  def self.plist name
    file = File.expand_path __FILE__
    root = File.expand_path '../..', __FILE__

    plist = <<-PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key>
  <string>#{name}</string>
  <key>ProgramArguments</key>
  <array>
    <string>#{Gem.ruby}</string>
    <string>-I#{root}/lib</string>
    <string>-I#{root}/ext</string>
    <string>#{file}</string>
  </array>
  <key>ServiceIPC</key>
  <true/>
  <key>Sockets</key>
  <dict>
    <key>EchoSocket</key>
    <dict>
      <key>SockServiceName</key>
      <string>12345</string>
    </dict>
  </dict>
</dict>
</plist>
    PLIST

    open name, 'w' do |io|
      io.write plist
    end
  end

  def initialize
    launch_checkin
  end

  ##
  # echo lines sent from +socket+

  def echo socket
    Thread.start do
      loop do
        socket.puts socket.gets
      end
    end
  end

  ##
  # Listens on +server+ for connections to echo on.

  def listen server
    Thread.start do
      loop do
        echo server.accept
      end
    end
  end

  ##
  # Starts listening on the sockets given by launchd and waits forever

  def run
    launch_sockets('EchoSocket', TCPServer).each do |server|
      listen server
    end

    sleep
  end

end

if ARGV.empty? then
  Echo.new.run
else
  Echo.plist ARGV.first
end

