begin
  require 'rake/gempackagetask'
rescue LoadError
end
require 'rake/clean'
require 'rbconfig'
include Config

PKG_NAME = 'ctapi'
PKG_VERSION = File.read('VERSION').chomp
PKG_FILES = FileList['**/*'].exclude(/(CVS|\.svn|pkg|coverage|doc)/)
CLEAN.include 'coverage', 'doc'

desc "Run unit tests"
task(:test => [ :compile ]) do
  ruby %{-Iext tests/test.rb}
end

desc "Run unit tests with rcov"
task(:coverage => [:compile]) do
  system %{rcov -x tests -Ilib -Iext tests/test.rb}
end

desc "Creating documentation"
task :doc do
  ruby 'make_doc.rb'
end

desc "Compiling library"
task :compile do
  cd 'ext'  do
    ruby %Q{extconf.rb}
    system "make"
  end
end

desc "Installing library"
task(:install => [:compile]) do
  ruby 'install.rb'
end

desc "Cleaning built files"
task :clean do
  cd 'ext'  do
    File.exist?('Makefile') and system "make distclean"
  end
end

if defined? Gem
  spec_src =<<GEM
# -*- encoding: utf-8 -*-
Gem::Specification.new do |s|
  s.name = '#{PKG_NAME}'
  s.version = '#{PKG_VERSION}'
  s.summary = 'Ruby extension for Chipcard Cardterminal-API (CTAPI)'
  s.description = <<EOF
These are Ruby bindings to a library that supports the Cardterminal-API (CTAPI)
for chipcards. It should be possible to link this against any carddriver
library that supports this standard, but I have actually only tested with
libtowitoko.
EOF

  s.files = #{PKG_FILES.to_a.sort.inspect}

  s.extensions << 'ext/extconf.rb'
  s.require_path = 'lib'

  s.bindir = "bin"
  s.executables = %w[cardinfo.rb ctsh.rb]
  s.default_executable = "ctsh.rb"

  s.has_rdoc = true
  s.rdoc_options << '--main' << 'doc-main.txt'
  s.extra_rdoc_files << 'doc-main.txt'
  s.test_files << 'tests/test.rb'

  s.author = 'Florian Frank'
  s.email = 'flori@ping.de'
  s.homepage = 'http://#{PKG_NAME}.rubyforge.org'
  s.rubyforge_project = '#{PKG_NAME}'
end
GEM

  desc 'Create a gemspec file'
  task :gemspec do
    File.open("#{PKG_NAME}.gemspec", 'w') do |f|
      f.puts spec_src
    end
  end

  spec = eval(spec_src)
  Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_tar = true
    pkg.package_files += PKG_FILES
  end
end

desc m = "Writing version information for #{PKG_VERSION}"
task :version do
  puts m
  File.open(File.join('lib', 'ctapi', 'version.rb'), 'w') do |v|
    v.puts <<EOT
module CTAPI
  # CTAPI version
  VERSION         = '#{PKG_VERSION}'
  VERSION_ARRAY   = VERSION.split(/\\./).map { |x| x.to_i } # :nodoc:
  VERSION_MAJOR   = VERSION_ARRAY[0] # :nodoc:
  VERSION_MINOR   = VERSION_ARRAY[1] # :nodoc:
  VERSION_BUILD   = VERSION_ARRAY[2] # :nodoc:
end
EOT
  end
end

desc "Default"
task :default => [ :version, :gemspec, :test ]

desc "Prepare a release"
task :release => [ :clean, :version, :gemspec, :package ]
