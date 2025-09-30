#include <inttypes.h>
#include "OneWire_direct_regtype.h"
#include "OneWire_direct_gpio.h"

#ifndef HikokiSerial_h
#define HikokiSerial_h


class HikokiSerial {
  public:
    //the makita uart implementation is not so tolerant so each cpu cycle counts here
    HikokiSerial(uint8_t outpin, uint8_t inv_outpin, uint8_t inpin) {
      out_bitmask = PIN_TO_BITMASK(outpin);
      out_basereg = PIN_TO_BASEREG(outpin);

      out_inv_bitmask = PIN_TO_BITMASK(inv_outpin);
      out_inv_basereg = PIN_TO_BASEREG(inv_outpin);

      in_bitmask = PIN_TO_BITMASK(inpin);
      in_basereg = PIN_TO_BASEREG(inpin);
      
      DIRECT_MODE_INPUT(in_basereg, in_bitmask);
      DIRECT_MODE_OUTPUT(out_basereg, out_bitmask);
      DIRECT_MODE_OUTPUT(out_inv_basereg, out_inv_bitmask);

      DIRECT_WRITE_LOW(out_basereg, out_bitmask);
      DIRECT_WRITE_HIGH(out_inv_basereg, out_inv_bitmask);
      DIRECT_WRITE_LOW(in_basereg,in_bitmask);

      #ifdef ESP32
           _tx_delay = 1000000/9600;
      #else
      _tx_delay = ((F_CPU / 9600) >> 2) - 3;
      #endif
    }

  void write(uint8_t * buff, int len) {
    for (int i = 0; i < len; i++) write(buff[i]);
  }

  void read(uint8_t * buff, int len) {
    for (int i = 0; i < len; i++) buff[i] = recv();
  }

  int recv() {
    int data = 0;

    DIRECT_MODE_INPUT(in_basereg, in_bitmask);
    DIRECT_WRITE_LOW(in_basereg,in_bitmask);

    //wait until we find the start bit
    int32_t waitIter = 256;
    for (; waitIter > 0 && !DIRECT_READ(in_basereg, in_bitmask); waitIter--) _delay_loop_2(_tx_delay>>3);
    if (waitIter <= 0) return 0;

    //wait half a bit to poll in the center, then another wait to skip the start bit ( at the beginning of the loop )
    _delay_loop_2(_tx_delay >> 1);

    // Read each of the 8 bits
    for (uint8_t i = 1; i > 0; i <<= 1) {
      _delay_loop_2(_tx_delay);
      if (!DIRECT_READ(in_basereg, in_bitmask)) data |= i;
    }

    // skip the stop bit
    _delay_loop_2(_tx_delay);

    return data;
  }

  void write(uint8_t b) {
    DIRECT_MODE_OUTPUT(out_basereg, out_bitmask);
    DIRECT_MODE_OUTPUT(out_inv_basereg, out_inv_bitmask);

    //write start bit 
    DIRECT_WRITE_HIGH(out_basereg, out_bitmask);
    DIRECT_WRITE_LOW(out_inv_basereg, out_inv_bitmask);
    _delay_loop_2(_tx_delay);

    // Write each of the 8 bits
    for (uint8_t i = 1; i > 0; i <<= 1) {
      if (b & i){ // choose bit
        DIRECT_WRITE_LOW(out_basereg, out_bitmask);
        DIRECT_WRITE_HIGH(out_inv_basereg, out_inv_bitmask);
      }else{
        DIRECT_WRITE_HIGH(out_basereg, out_bitmask);
        DIRECT_WRITE_LOW(out_inv_basereg, out_inv_bitmask);    
      }

      _delay_loop_2(_tx_delay);
    }

    // stop bit
    DIRECT_WRITE_LOW(out_basereg, out_bitmask);
    DIRECT_WRITE_HIGH(out_inv_basereg, out_inv_bitmask);
    _delay_loop_2(_tx_delay);
    
    // restore pin to natural state
    DIRECT_WRITE_LOW(out_basereg, out_bitmask);
    DIRECT_WRITE_HIGH(out_inv_basereg, out_inv_bitmask);

   // _delay_loop_2(_tx_delay);
  }


  private: 

  #ifdef ESP32
     inline  void     _delay_loop_2(uint16_t delay){
        esp_rom_delay_us(delay);
      }
  #endif

  IO_REG_TYPE in_bitmask;
  IO_REG_TYPE out_bitmask;
  IO_REG_TYPE out_inv_bitmask;
  volatile IO_REG_TYPE * in_basereg;
  volatile IO_REG_TYPE * out_basereg;
  volatile IO_REG_TYPE * out_inv_basereg;

  uint16_t _tx_delay;
};

#endif