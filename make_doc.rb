#!/usr/bin/env ruby

$outdir = 'doc/'
puts "Creating documentation in '#$outdir'."
system "rdoc -m doc-main.txt -o #$outdir doc-main.txt lib/ctapi.rb lib/ctapi/version.rb ext/ctapicore.c" or
  fail "Couldn't create documentation!"
