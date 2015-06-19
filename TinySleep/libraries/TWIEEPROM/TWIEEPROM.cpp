#include <TWISoft.h>
#include "TWIEEPROM.h"

// Initialize Class Variables
uint8_t TWI_EEPROM::_devaddr = 0xA0;         // default slave addr
uint8_t TWI_EEPROM::_error = 0;              // init error flag

// Constructors
TWI_EEPROM::TWI_EEPROM(uint8_t devaddr) {
  _devaddr = devaddr;
}

// Public Methods
uint8_t TWI_EEPROM::write(unsigned int addr, uint8_t* buf, uint8_t len) {
  _error = TWISoft::start(_devaddr|TWI_WRITE);
  if(_error != 0) return 0;
  _error = TWISoft::write((uint8_t)(addr >> 8)); // MSB
  if(_error != 0) return 0;
  _error = TWISoft::write((uint8_t)(addr));      // LSB
  if(_error != 0) return 0;
  for(uint8_t i=0; i<len; i++) {
    _error = TWISoft::write(buf[i]);
    if(_error != 0) return 0;
  }
  _error = TWISoft::stop();
  if(_error != 0) return 0;
  return len;
}

uint8_t TWI_EEPROM::read (unsigned int addr, uint8_t* buf, uint8_t len) {
  _error = TWISoft::start(_devaddr|TWI_WRITE);
  if(_error != 0) return 0;
  _error = TWISoft::write((uint8_t)(addr >> 8)); // MSB
  if(_error != 0) return 0;
  _error = TWISoft::write((uint8_t)(addr));      // LSB
  if(_error != 0) return 0;
  _error = TWISoft::stop();
  if(_error != 0) return 0;
  
  _error = TWISoft::start(_devaddr|TWI_READ);
  if(_error != 0) return 0;
  for(uint8_t i=0; i<len; i++) {
    _error = TWISoft::read(buf+i,i!=len-1);
    if(_error != 0) return 0;
  }
  
  return len;
}
