
dir = File.dirname(File.expand_path(__FILE__))

require 'mkmf'

pkg_config('gtk+-2.0')
pkg_config('libwnck-1.0')
pkg_config('gdk-2.0')
pkg_config('glib-2.0')

load("#{dir}/build.rb")

extension_name = 'wnck'
dir_config(extension_name)
create_makefile('wnck')
