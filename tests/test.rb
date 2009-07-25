#!/usr/bin/env ruby

require 'test/unit'
$:.unshift 'lib'
$:.unshift '../lib'
require 'ctapi'

class TC_CTAPI < Test::Unit::TestCase
  include CTAPI
  CT = Cardterminal.init(PORT_COM1)
  
  def test_preconditions
    assert_instance_of(Cardterminal, CT)
    assert_instance_of(Cardterminal::Manufacturer, CT.manufacturer)
    assert(CT.card_inserted?)
    assert_instance_of(Cardterminal::Card, CT.card)
    assert(CT.card.atr_ok?)
    assert(CT.card.memory_size >= 255) # I hope there is no smaller size
    assert(CT.card.structure)
    assert(CT.card.protocol)
  end

  def test_read
    assert(!CT.read(0, 0))
    assert(CT.read(0, 255).size == 255)
    assert(CT.read.size == CT.card.memory_size)
    assert(CT.read(100).size == CT.card.memory_size - 100)
    old_cs = CT.chunk_size
    CT.chunk_size = 23
    assert(!CT.read(0, 0))
    assert(CT.read(0, 255).size == 255)
    assert(CT.read.size == CT.card.memory_size)
    assert(CT.read(100).size == CT.card.memory_size - 100)
    CT.chunk_size = old_cs
  end

  def test_write
    data = (1..255).to_a
    big = "A" * CT.card.memory_size
    assert(!CT.write(''))
    assert(CT.write(data))
    assert_equal(data.pack("C*"), CT.read(0, 255))
    assert(CT.write(big))
    assert_equal(CT.read, big)
    old_cs = CT.chunk_size
    CT.chunk_size = 23
    assert(!CT.write(''))
    assert(CT.write(data))
    assert_equal(data.pack("C*"), CT.read(0, 255))
    assert(CT.write(big))
    assert_equal(CT.read, big)
    CT.chunk_size = old_cs
  end
end
  # vim: set et sw=2 ts=2:
