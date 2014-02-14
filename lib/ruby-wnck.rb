
require_relative './wnck.so'

module Wnck
  class Screen
    def inspect
      "#<Wnck::Screen: #{window_manager_name}(#{width}x#{height})>"
    end
  end
  class Window
    def inspect
      "#<Wnck::Window: #{name}>"
    end
  end
  class ClassGroup
    def inspect
      "#<Wnck::ClassGroup: #{res_class}>"
    end
  end
end

if $0 == __FILE__
  require 'irb'
  require 'gtk2'
  IRB.start
end
