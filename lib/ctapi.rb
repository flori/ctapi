module CTAPI
  require 'ctapicore'
  require 'ctapi/version'
  include CTAPICore

  require 'sync'

  # Represents a cardterminal. Mixes in CTAPI methods and constants.
  class Cardterminal
    include CTAPI

    # Global lock for managment of cardterminals.
    LOCK = Sync.new

    # Returns the mapping of cardterminal numbers to Cardterminal instances as
    # a hash.
    def self.cardterminals
      @cardterminals ||= {}
    end

    # Convenience method to access Cardterminal.cardterminals.
    def cardterminals
      self.class.cardterminals
    end

    # Creates a new Cardterminal object for this interface and initializes
    # the card terminal that is connected. If no cardterminal number is
    # given, a number (that is not currently in use) is assigned to this
    # object. Otherwise this object is forced to use the given card
    # terminal number. BTW: number has to be >= 0.
    def initialize(interface, number = nil)
      LOCK.synchronize do
        @interface = interface
        @slot = ICC1
        @chunk_size = 255
        if number
          @number = number
        else
          @number = 0
          while cardterminals.key?(@number)
            @number += 1
          end
        end
        ct_init(@number, interface)
        cardterminals[@number] = true
        @manufacturer = get_manufacturer
        reset
        select_file # master file
      end
    end

    class << self
      alias init new
    end

    # Returns a Manufacturer object with information about the manufacturer
    # of this cardterminal. Could return nil if the response was not
    # successful.
    def get_manufacturer
      get_manufacturer_status = [ CTBCS_CLA, CTBCS_INS_STATUS,
        CTBCS_P1_CT_KERNEL, CTBCS_P2_STATUS_MANUFACTURER, 0 ]
      response = data(CT, HOST, get_manufacturer_status)
      return nil unless response.successful?
      Manufacturer.new(response)
    end
    private :get_manufacturer

    # The cardterminal number assigned to this object.
    attr_reader :number

    # The interface this cardterminal is connected to. This is
    # a value of PORT_COM1, PORT_COM2, PORT_COM3, PORT_COM4, PORT_LPT1,
    # PORT_LPT2, PORT_Modem or PORT_Printer.
    attr_reader :interface

    # The slot of the card terminal where the card is inserted. This
    # is a value in the range of ICC1, ICC2,..., ICC14. Default is
    # ICC1.
    attr_accessor :slot

    # The maximum size of the chunks of data that are read from/written to
    # the card. Can be set by calling the chunk_size= method.
    attr_reader :chunk_size

     # Sets the size of the chunks of data that are read from/written to
    # the card to size: (0 < size <= 255).
    def chunk_size=(size)
      raise ArgumentError.new("size must be > 0") unless size > 0 
      raise ArgumentError.new("size must be <= 255") unless size <= 255
      @chunk_size = size
    end

    # The Manufacturer object for this cardterminal.
    attr_reader :manufacturer

    # Card object of the inserted chipcard.
    attr_reader :card

    # Takes a block and yields to an initialized Cardterminal object.
    # After the block has been called, the cardterminal is closed
    # again.
    def self.open(interface, number = nil)
      cardterminal = self.new(interface, cardterminal)
      yield cardterminal
    ensure
      cardterminal.close if cardterminal
    end

    # Sends a sequence of commands to the card or the cardterminal
    # (depending on destination/dad and source address/sad) and returns the
    # response (or responses) to the calling program. A command can be
    # given as an array of bytes [ 0x12, 0x23 ] or as a string of the form
    # '12:23:a4' or '12 23 a4' or as a Command object.
    def data(dad, sad, *commands)
      responses = []
      commands.each do |command|
        command =
          case command
          when String then Command.from_string(command)
          when Array then Command.from_array(command)
          else command
          end
        $DEBUG and debug(2, command)
        data = ct_data(@number, dad, sad, command.data)
        response = Response.new(data)
        $DEBUG and debug(1, response)
        responses << response
      end
      return *responses
    end

    # Terminates the communication with the cardterminal associated with
    # this object and releases the assigned cardterminal number.
    def close
      LOCK.synchronize do
        ct_close(@number)
        cardterminals.delete @number
      end
    end

    # Sends the select file byte sequence to the card with a default value
    # for the master file. Returns true if the response was successful,
    # false otherwise.
    def select_file(fid = [ 0x3f, 0 ])
      select_file = [ 0, 0xa4, 0, 0, 0x02, *fid ]
      response = data(@slot, HOST, select_file)
      response.successful?
    end

    def read_chunk(address, size)
      unless address <= 65535
        raise ArgumentError.new("address must be <= 65535")
      end
      unless size <= 255
        raise ArgumentError.new("size must be <= 255")
      end
      read_file = [ 0, 0xb0, address >> 8, address & 0xff, size ]
      response = data(@slot, HOST, read_file)
      response.successful? or return
      data = response.data
      data.slice!(-2, 2)
      data
    end
    private :read_chunk

    # Attempts to read data of length size starting at address. If
    # size is nil, an attempt is made to read the whole card memory.
    def read(address = 0, size = nil)
      if size == nil
        if @card
          size = @card.memory_size - address
        else
          size = chunk_size
        end
      elsif @card and address + size > @card.memory_size
        size = @card.memory_size - address
      end
      return if size <= 0
      data = ''
      caught = catch(:break) do
        while size >= chunk_size
          d = read_chunk(address, chunk_size)
          if d
            data << d
            address += chunk_size
            size -= chunk_size
          else
            break :break  
          end
        end
      end
      if caught != :break and size > 0
        d = read_chunk(address, size) and data << d
      end
      data
    end

    def write_chunk(address, data)
      unless address <= 65535
        raise ArgumentError.new("address must be <= 65535")
      end
      unless data.size <= 255
        raise ArgumentError.new("size of data must be <= 255")
      end
      data = data.unpack("C*") if data.is_a? String
      write_file = [ 0, 0xd6, address >> 8, address & 0xff, data.size ] +
        data
      response = data(@slot, HOST, write_file)
      response.successful? or return
      true
    end
    private :write_chunk

    # Attempts to write the string data to the card starting at address.
    # On success returns a true value.
    def write(data, address = 0)
      size = data.size
      if @card and address + size > @card.memory_size
        size = @card.memory_size - address
      end
      return if size <= 0
      offset = 0
      caught = catch(:break) do
        while size >= chunk_size
          write_chunk(address, data[offset, chunk_size]) or break :break
          address += chunk_size
          offset += chunk_size
          size -= chunk_size
        end
      end
      if caught == :break
        return false
      elsif size > 0
        write_chunk(address, data[offset, size])
      end
      true
    end

    # The pin is sent to the card. It can be given as a string or an array
    # of characters. A true result is returned if the sending was
    # successful.
    def enter_pin(pin)
      unless pin.size <= 255
        raise ArgumentError.new("size of pin must be <= 255")
      end
      pin = pin.unpack("C*") if pin.is_a? String
      enter_pin = [ 0, 0x20, 0, 0, pin.size ] + pin
      response = data(@slot, HOST, enter_pin)
      response.successful? or return
      true
    end

    # The pin of this card is changed from old_pin to new_pin. They can be
    # given as strings or arrays of characters. A true result is returned
    # if the sending was successful.
    def change_pin(old_pin, new_pin)
      old_pin = old_pin.unpack("C*") if old_pin.is_a? String
      new_pin = new_pin.unpack("C*") if new_pin.is_a? String
      data = old_pin + new_pin
      unless data.size <= 255
        raise ArgumentError.new("size of old and new pin must be <= 255")
      end
      change_pin = [ 0, 0x24, 0, 0, data.size ] + data
      response = data(@slot, HOST, change_pin)
      response.successful? or return
      true
    end

    # Requests the card status from the cardterminal. Returns the
    # Response object or nil if the response wasn't successful.
    # This method is called by the card_inserted? method to find
    # out if a card is inserted into the terminal.
    def request_card_status
      get_card_status = [ CTBCS_CLA, CTBCS_INS_STATUS,
        CTBCS_P1_CT_KERNEL, CTBCS_P2_STATUS_ICC, 0 ]
      response = data(CT, HOST, get_card_status)
      response.successful? ? response : nil
    end

    # Sends a byte sequence to the card to get the answer to reset
    # Response object. This method is called by the Cardterminal#reset
    # method.
    def request_icc
        get_atr = [ CTBCS_CLA, CTBCS_INS_REQUEST, CTBCS_P1_INTERFACE1,
               CTBCS_P2_REQUEST_GET_ATR, 0 ]
      @card_old = @card if @card
      @card, atr = nil, nil
      begin
        if card_inserted?
          atr = data(CT, HOST, get_atr)
          if atr
            if atr.not_changed?
              @card = @card_old
              return @card.atr
            end
            @card = Card.new(atr)
          end
        end
      rescue CTAPIError => e
        STDERR.puts "Caught: #{e}."
      end
      @card.atr
    end

    # Sends the eject card byte sequence to the card terminal and
    # returns the Response object. This method is called by the
    # Cardterminal#reset method.
    def eject_icc
      eject = [ 0x20, 0x15, 0x01, 0x00, 0x00 ]
      data(CT, HOST, eject)
    end

    # The cardterminal is reset by first calling eject_icc and
    # then request_icc. If the reset was successful the Response
    # object of request_icc is returned, otherwise a nontrue value is
    # returned.
    def reset
      eject_icc
      response = request_icc or return
      response.successful? or return
      response
    end

    # Returns the card status as symbol: :no_card, :card, :card_connect.
    def card_status
      response = request_card_status
      case response[0]
      when CTBCS_DATA_STATUS_NOCARD
        :no_card
      when CTBCS_DATA_STATUS_CARD
        :card
      when CTBCS_DATA_STATUS_CARD_CONNECT
        :card_connect
      end
    end
  
    # Returns true if a card is inserted in the cardterminal at the moment,
    # false if the cardterminal is empty.
    def card_inserted?
      cs = card_status
      cs == :card || cs == :card_connect ? true : false
    end

    # Returns true if the card has been changed in the cardterminal,
    # false it is still the same card. If an error occured nil
    # is returned.
    def card_changed?
      response = reset or return
      response.changed? and return true
      response.not_changed? and return false
      return
    end
    private :card_changed?

    # A little structure for manufacturer information.
    class Manufacturer

      # Creates a Manufacturer object for this cardterminal from
      # a manufacturer status response.
      def initialize(response)
        @manufacturer, @model, @revision =
          response[0, 5], response[5, 3], response[10, 5]
      end

      # The manufacturer of this cardterminal.
      attr_reader :manufacturer

      # The model name of this cardterminal.
      attr_reader :model

      # The revision number of this cardterminal.
      attr_reader :revision

      # A string of the form 'manufacturer model revision' is returned.
      def to_s
        [manufacturer, model, revision].join(" ")
      end

    end

    # Prints data with indication of direction to STDERR.
    def debug(direction, data)
      direction =
      case direction
        when 1 then '>>> '
        when 2 then '<<< '
        else ''
      end
      STDERR.puts direction + data.to_s
    end
    private :debug
  end

  # APDU (Application Protocol Data Unit) that is sent to/received from the
  # cardterminal. This is the parent class of Command and Response.
  class APDU
    # An APDU object is generated from the String data.
    def initialize(data)
      @data = data
    end

    # This is the data string.
    attr_reader :data

    # Access data string by indices.
    def [](*args) @data[*args] end

    # We use Ruby's inspect representation to inspect the data.
    def inspect
      @data.inspect
    end

    # To display the data, a string of the form '12:23:a4' is built.
    def to_s
      @data.unpack('C*').map { |x| sprintf("%02x", x) }.join(':')
    end
  end

  # A command sent to the cardterminal.
  class Command < APDU
    # A Command is generated from a string of the form '12:23:a4' or
    # '12 23 a4'.
    def self.from_string(command)
      command = command.split(/[ :]/).map { |x| Integer '0x' + x }
      from_array(command)
    end

    # A Command is generated from an array of bytes of the form
    # [ 0x12, 0x23, 0xa4].
    def self.from_array(command)
      new(command.pack('C*'))
    end
  end

  # A response received from the cardterminal.
  class Response < APDU
    # If this response was successful true is returned, false otherwise.
    # A response is successful if the last two bytes are '90:00'.
    def successful?
      return false if @data.size < 2
      return true if @data[-2, 2] == "\x90\x00"
      false
    end

    def changed?
      @data[-2, 2] == "\x62\x00" ? true : false
    end

    def not_changed?
      @data[-2, 2] == "\x62\x01" ? true : false
    end
  end

  class Card
    def initialize(atr)
      @atr = atr
    end

    # The answer to reset (ATR) of the inserted chipcard.
    attr_accessor :atr

    # Check if the ATR is correct. If false, the data of this Card
    # object could be wrong and the calculations based on this data wrong.
    def atr_ok?
      @atr[4, 2] == "\x90\x00" && @atr[2] == 0x10
    end

    # The bit width of this chipcard.
    def memory_bits
      1 << (@atr[1] & 0x07)
    end

    # Number of memory blocks on this chipcard.
    def memory_blocks
      1 << (((@atr[1] & 0x78) >> 3) + 6)
    end

    # Computes the memory size of the inserted chipcard in bytes.
    def memory_size
      (memory_blocks * memory_bits >> 3)
    end

    # The structure of this chipcard.
    def structure
      if (@atr[0] & 0x3) == 0
        'ISO'
      elsif (@atr[0] & 0x7) == 2
        'common use'
      elsif (@atr[0] & 0x7) == 6
        'proprietary use'
      else
        'special use'
      end
    end

    # Supported protocol of this chipcard.
    def protocol
      if (@atr[0] & 0x80) == 0x00
        'ISO'
      elsif (@atr[0] & 0xf0) == 0x80
        'I2C'
      elsif (@atr[0] & 0xf0) == 0x90
        '3W'
      elsif (@atr[0] & 0xf0) == 0xa0
        '2W'
      else
        'unknown'
      end
    end

    # Returns a short description of this card as a string.
    def to_s
      "<#{self.class}:Structure #{structure} Protocol #{protocol} "  +
      "(#{memory_blocks}x#{memory_bits}=#{memory_size}B)" + '>'
    end

    # Returns adescription of this card including the ATR as a string.
    def inspect
      string = to_s
      string[-1] = " #{atr}>"
      string
    end
  end
end
