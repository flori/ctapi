#!/usr/bin/env ruby

require 'rbconfig'
require 'fileutils'
include FileUtils::Verbose

include Config

unless ARGV.empty?
  ENV['CTAPI_LIBRARY'] = ARGV.shift.to_s
end
cd 'ext' do
  File.exist?('Makefile') and system("make clean")
  system("ruby extconf.rb") or
    fail "Could not make makefile for library!"
  system("make") or fail "Could not make library"
end

libdir = CONFIG["sitelibdir"]
for file in ["ext/ctapicore.#{CONFIG['DLEXT']}", 'lib/ctapi.rb']
  install(file, libdir)
end
subdir = File.join(libdir, 'ctapi')
mkdir subdir
install(file, File.join(subdir, 'version.rb'))
