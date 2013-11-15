guard :shell do
  watch %r{\.(c|h|def)*$} do | m |
    $gq.go "make test"
  end
  watch 'boot.scm' do | m |
    $gq.go "make test"
  end
  watch %r{^t/test.*\.scm$} do | m |
    $gq.go "make test TEST_FILE=#{m[0]}"
  end
  watch 'Makefile' do | m |
    $gq.go "make clean"
    $gq.go "make test"
  end
end

require 'thread'

class GuardQueue
  def initialize
    @stage = [ ]
    @queue = Queue.new
  end

  def msg msg
    $stderr.puts "  GQ #{$$} #{Thread.current.object_id} #{msg}"
  end

  def go cmd
    if @stage.include? cmd
      msg "ALREADY  #{cmd.inspect}"
    else
      @stage.unshift cmd
      msg "STAGE    #{@stage.inspect}"
    end
    if @working
      msg "WORKING  #{@working.inspect}" unless @working == cmd
    else
      while cmd = @stage.pop
        msg "QUEUEING #{cmd.inspect}"
        @queue.enq cmd
      end
    end
  end

  def start!
    at_exit do
      go :stop
    end

    Thread.new do
      msg "WAITING"
      while true
        case cmd = @queue.deq
        when :stop
          break
        else
          begin
            @working = cmd
            msg "RUNNING #{cmd.inspect}"
            `#{cmd}`
          ensure
            sleep 2
            msg "   DONE #{cmd.inspect}"
            @working = nil
          end
        end
      end
      msg "STOPPING"
    end
    self
  end
end

$gq ||= GuardQueue.new.start!

