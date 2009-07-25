#!/usr/bin/env ruby

require 'ctapi'
include CTAPI

interface = (ARGV.shift || PORT_COM1).to_i
Cardterminal.open(interface) do |ct|
  puts "Cardterminal Manufacturer: " + ct.manufacturer.to_s
  if ct.card_inserted?
    card = ct.card
    puts "My Cardterminal object: #{ct.inspect}"
    puts "Current card status is: #{ct.card_status}"
    puts "Answer to Reset is #{card.atr}."
    puts "ATR ok? #{card.atr_ok?}."
    puts "Memory size of this card is #{card.memory_size} bytes " +
      "(#{card.memory_blocks} blocks x #{card.memory_bits} bit)."
    puts "Structure of this card is #{card.structure}."
    puts "Supported protocol type of this card is #{card.protocol}."
    puts "Trying to read(0, 16):"
    data = ct.read(0, 16)
    puts "Have read #{data.size} bytes:"
    p data
  else
    puts "Please insert a card into your cardterminal!"
  end
end
