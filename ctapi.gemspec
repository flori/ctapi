# -*- encoding: utf-8 -*-
Gem::Specification.new do |s|
  s.name = 'ctapi'
  s.version = '0.2.3'
  s.summary = 'Ruby extension for Chipcard Cardterminal-API (CTAPI)'
  s.description = <<EOF
These are Ruby bindings to a library that supports the Cardterminal-API (CTAPI)
for chipcards. It should be possible to link this against any carddriver
library that supports this standard, but I have actually only tested with
libtowitoko.
EOF

  s.files = ["CHANGES", "COPYING", "README", "Rakefile", "VERSION", "bin", "bin/cardinfo.rb", "bin/ctsh.rb", "ctapi.gemspec", "ext", "ext/ctapicore.c", "ext/extconf.rb", "install.rb", "lib", "lib/ctapi", "lib/ctapi.rb", "lib/ctapi/version.rb", "tests", "tests/test.rb"]

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
  s.homepage = 'http://ctapi.rubyforge.org'
  s.rubyforge_project = 'ctapi'
end
