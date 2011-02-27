##
# Launch is a wrapper for the launch(3) API for daemons and agents spawned by
# launchd(8).
#
# Launch agents and daemons MUST NOT:
#
# * Call daemon(3)
# * Do the equivalent of daemon(3) by calling fork and having the parent exit
#
# Launch agents and daemons SHOULD NOT do the following as part of their
# starup initialization:
#
# * Set the user ID or group ID (Process::Sys.setuid, Process::Sys.setgid and
#   friends)
# * Setup the working directory (Dir.chdir)
# * chroot(2)
# * setsid(2) (Process.setsid)
# * Close "stray" file descriptors
# * Change stdio(3) to /dev/null (STDOUT.reopen and friends)
# * Setup resource limits with setrusage (Process.setrlimit)
# * Ignore the SIGTERM signal (trap 'TERM', 'IGNORE')
#
# The above is from launchd.plist(5).  Please read it for further details.
#
# To shut down cleanly <tt>trap 'TERM'</tt> and perform any shutdown steps
# before exiting.

module Launch

  ##
  # The version of launch you are using

  VERSION = '1.0'

  ##
  # Checks in with launch and retrieves the agent's configuration.  The
  # configuration can be retrieved later through +@launch_checkin+.

  def launch_checkin
    response = launch_message Launch::Key::CHECKIN

    return if response.nil?

    @launch_checkin = response
  end

  ##
  # Creates ruby sockets from the sockets list in +name+.
  # <tt>socket_class.for_fd</tt> is called for each socket in the named list.
  #
  # +name+ comes from the socket's name key in the launchd plist.
  #
  # Example plist:
  #
  #  <key>Sockets</key>
  #  <dict>
  #    <key>EchoSocket</key>
  #    <dict>
  #      <key>SockServiceName</key>
  #      <string>12345</string>
  #    </dict>
  #  </dict>
  #
  # Example call:
  #
  #   servers = launch_sockets 'Echo', TCPServer
  #
  #   p servers.map { |server| server.addr }

  def launch_sockets name, socket_class
    require 'socket'

    sockets = @launch_checkin[Launch::JobKey::SOCKETS][name]

    raise Error, "no sockets found for #{name.inspect}" unless sockets

    sockets.map do |fd|
      socket_class.for_fd fd
    end
  end

end

require 'launch/launch'
require 'socket'

