require 'launch'
require 'webrick'

##
# Launch::WEBrickHTTPServer adds launchd support to WEBrick.
#
# To use, replace WEBrick::HTTPServer with Launch::WEBrickHTTPServer.
#
# By default Launch::WEBrickHTTPServer expects to find the socket list under
# <tt>'WEBrickSockets'</tt> but this may be overridden through the
# +:LaunchdSockets+ option.
#
# The server will automatically shut down when a TERM signal is sent.  If you
# wish to perform other shutdown actions override TERM but be sure to shut
# down webrick.
#
# An example WEBrick server using Launch::WEBrickHTTPServer would be:
#
#   require 'launch/webrick'
#
#   Launch::WEBrickHTTPServer.new(:DocumentRoot => ARGV.shift).start
#
# Here is an example plist for this server which listens on port 8000:
#
#   <?xml version="1.0" encoding="UTF-8"?>
#   <!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
#   <plist version="1.0">
#   <dict>
#     <key>Label</key>
#     <string>net.segment7.launch.webrick</string>
#     <key>ProgramArguments</key>
#     <array>
#       <string>/path/to/ruby</string>
#       <string>/path/to/webrick</string>
#       <string>/Users/your_user/Sites</string>
#     </array>
#     <key>ServiceIPC</key>
#     <true/>
#     <key>Sockets</key>
#     <dict>
#       <key>WEBrickSockets</key>
#       <dict>
#         <key>SockServiceName</key>
#         <string>8000</string>
#       </dict>
#     </dict>
#   </dict>
#   </plist>

class Launch::WEBrickHTTPServer < WEBrick::HTTPServer
  
  include Launch

  ##
  # Initializes an HTTP server with +options+ and set the server's listeners
  # using Launch.  A TERM handler to shut down the server is automatically
  # set.
  #
  # +:LaunchdSockets+ may be set to change the socket key from the default of
  # <tt>'WEBrickSockets'</tt>

  def initialize options = {}
    options[:DoNotListen] = true
    sockets_key = options.delete(:LaunchdSockets) || 'WEBrickSockets'

    super

    launch_checkin

    servers = launch_sockets sockets_key, TCPServer

    listeners.replace servers

    trap 'TERM' do shutdown end
  end

end

