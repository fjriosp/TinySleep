#include <TWIUSI.h>
#include "TWIEEPROM.h"

// Initialize Class Variables
uint8_t TWI_EEPROM::_devaddr = 0xA0;         // default slave addr
uint8_t TWI_EEPROM::_error = 0;              // init error flag

// Constructors
__attribute__ ((always_inline))
TWI_EEPROM::TWI_EEPROM(uint8_t devaddr) {
  _devaddr = devaddr;
}

// Public Methods
__attribute__ ((always_inline))
void TWI_EEPROM::begin(void) {
  TWIUSI.begin();
}

__attribute__ ((noinline))
uint8_t TWI_EEPROM::write(unsigned int addr, uint8_t* buf, uint8_t len) {
  _error = TWIUSI.start(_devaddr|TWI_WRITE);
  if(_error != 0) return 0;
  _error = TWIUSI.write((uint8_t)(addr >> 8)); // MSB
  if(_error != 0) return 0;
  _error = TWIUSI.write((uint8_t)(addr));      // LSB
  if(_error != 0) return 0;
  
  for(uint8_t i=0; i<len; i++) {
    _error = TWIUSI.write(buf[i]);
    if(_error != 0) return 0;
  }
  _error = TWIUSI.stop();
  if(_error != 0) return 0;
  
  return len;
}

__attribute__ ((noinline))
uint8_t TWI_EEPROM::read (unsigned int addr, uint8_t* buf, uint8_t len) {
  _error = TWIUSI.start(_devaddr|TWI_WRITE);
  if(_error != 0) return 0;
  _error = TWIUSI.write((uint8_t)(addr >> 8)); // MSB
  if(_error != 0) return 0;
  _error = TWIUSI.write((uint8_t)(addr));      // LSB
  if(_error != 0) return 0;
  
  _error = TWIUSI.start(_devaddr|TWI_READ);
  if(_error != 0) return 0;
  for(uint8_t i=0; i<len; i++) {
    _error = TWIUSI.read(buf+i,i==len-1);
    if(_error != 0) return 0;
  }
  _error = TWIUSI.stop();
  if(_error != 0) return 0;
  
  return len;
}
