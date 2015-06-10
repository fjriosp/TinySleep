#include <TinyWireM.h>
#include "TWISerial.h"

// Constants
const uint8_t TWI_SERIAL_PRINT     = 'p';
const uint8_t TWI_SERIAL_AVAILABLE = 'a';
const uint8_t TWI_SERIAL_READ      = 'r';

// Initialize Class Variables
uint8_t TWI_SERIAL::_buf[TWI_SERIAL_BUF_SIZE]; // holds send and receive data
uint8_t TWI_SERIAL::_bufIdx = 0;               // current number of bytes in the send buff
uint8_t TWI_SERIAL::_slaveAddr = 0x08;         // default slave addr
uint8_t TWI_SERIAL::_error = 0;                // init error flag

// Constructors
TWI_SERIAL::TWI_SERIAL(uint8_t slaveAddr) {
  _slaveAddr = slaveAddr;
}

// Public Methods
void TWI_SERIAL::begin() {
  TinyWireM.begin();
}

size_t TWI_SERIAL::write(uint8_t data) {
  if(_bufIdx >= TWI_SERIAL_BUF_SIZE)
    flush();
  
  _buf[_bufIdx] = data;
  _bufIdx++;
  
  if(_bufIdx >= TWI_SERIAL_BUF_SIZE)
    flush();
  
  return 1;
}

void TWI_SERIAL::flush(void) {
  TinyWireM.beginTransmission(_slaveAddr);
  TinyWireM.write(TWI_SERIAL_PRINT);
  TinyWireM.write(_bufIdx);
  for(int i=0; i<_bufIdx; i++) {
    TinyWireM.write(_buf[i]);
  }
  TinyWireM.endTransmission();
  _bufIdx = 0;
}

uint8_t TWI_SERIAL::available() {
  TinyWireM.beginTransmission(_slaveAddr);
  TinyWireM.write(TWI_SERIAL_AVAILABLE);
  _error = TinyWireM.endTransmission();
  if(_error != 0) {
    return 0;
  }
  delay(5);
  _error = TinyWireM.requestFrom(_slaveAddr,1);
  if(_error != 0) {
    return 0;
  }
  uint8_t res = TinyWireM.receive();
  return res;
}

uint8_t TWI_SERIAL::read() {
  TinyWireM.beginTransmission(_slaveAddr);
  TinyWireM.write(TWI_SERIAL_READ);
  _error = TinyWireM.endTransmission();
  if(_error != 0) {
    return 0;
  }
  delay(5);
  _error = TinyWireM.requestFrom(_slaveAddr,1);
  if(_error != 0) {
    return 0;
  }
  uint8_t res = TinyWireM.receive();
  return res;
}

uint8_t TWI_SERIAL::isError() {
  return _error;
}