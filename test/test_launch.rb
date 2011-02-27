require 'minitest/autorun'
require 'launch'

class TestLaunch < MiniTest::Unit::TestCase

  include Launch

  def launch_message message
    @message = message

    return { 'response' => 'hash' }
  end

  def test_launch_checkin
    response = launch_checkin

    assert_equal Launch::Key::CHECKIN, @message

    expected = { 'response' => 'hash' }

    assert_equal expected, response
    assert_equal expected, @launch_checkin
  end

  def test_launch_sockets
    @launch_checkin = {
      Launch::JobKey::SOCKETS => {
        'MySockets' => [STDIN.to_i, STDOUT.to_i]
      }
    }

    sockets = launch_sockets 'MySockets', IO

    assert_equal 2, sockets.length

    assert_instance_of IO, sockets.first
    assert_instance_of IO, sockets.last

    assert_equal [0, 1], sockets.map { |io| io.to_i }
  end

  def test_launch_sockets_none
    @launch_checkin = {
      Launch::JobKey::SOCKETS => {
      }
    }

    e = assert_raises Launch::Error do
      launch_sockets 'NoSockets', IO
    end

    assert_equal 'no sockets found for "NoSockets"', e.message
  end

end
