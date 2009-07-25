require 'mkmf'
require 'fileutils'

DEFAULT_LIBRARY = 'towitoko'
unless libraryname = ENV['CTAPI_LIBRARY']
  libraryname = DEFAULT_LIBRARY
  warn "Using default library #{libraryname}!"
end
if have_library(libraryname)
  create_makefile('ctapicore')
end
