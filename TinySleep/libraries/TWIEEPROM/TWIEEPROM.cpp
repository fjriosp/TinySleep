#include <TinyWireM.h>
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
  TinyWireM.beginTransmission(_devaddr);
  TinyWireM.write((uint8_t)(addr >> 8)); // MSB
  TinyWireM.write((uint8_t)(addr));      // LSB
  for(uint8_t i=0; i<len; i++) {
    TinyWireM.write(buf[i]);
  }
  _error = TinyWireM.endTransmission();
  if(_error != 0) {
    return 0;
  }
  return len;
}

uint8_t TWI_EEPROM::read (unsigned int addr, uint8_t* buf, uint8_t len) {
  TinyWireM.beginTransmission(_devaddr);
  TinyWireM.write((uint8_t)(addr >> 8)); // MSB
  TinyWireM.write((uint8_t)(addr));      // LSB
  _error = TinyWireM.endTransmission();
  if(_error != 0) {
    return 0;
  }
  _error = TinyWireM.requestFrom(_devaddr,len);
  if(_error != 0) {
    return 0;
  }
  for(uint8_t i=0; i<len; i++) {
    if(!TinyWireM.available()) {
      break;
    }
    buf[i] = TinyWireM.read();
  }
  return len;
}
