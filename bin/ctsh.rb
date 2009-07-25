#!/usr/bin/env ruby

require 'irb'
require 'irb/completion'
require 'ctapi'

module ::IRB
  def self.examine(binding = TOPLEVEL_BINDING)
    setup nil
    workspace = WorkSpace.new binding
    irb = Irb.new workspace
    @CONF[:MAIN_CONTEXT] = irb.context
    catch(:IRB_EXIT) { irb.eval_input }
  rescue Interrupt
    exit
  end
end

include CTAPI

IRB.examine Cardterminal.new((ARGV.shift || PORT_COM1).to_i)
