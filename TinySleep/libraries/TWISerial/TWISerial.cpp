#include <TWIUSI.h>
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
uint8_t TWI_SERIAL::_enabled = 1;              // is enabled?

// Constructors
__attribute__ ((always_inline))
TWI_SERIAL::TWI_SERIAL(uint8_t slaveAddr) {
  _slaveAddr = slaveAddr;
}

// Public Methods
__attribute__ ((always_inline))
void TWI_SERIAL::begin(void) {
  TWIUSI.begin();
}

__attribute__ ((noinline))
size_t TWI_SERIAL::write(uint8_t data) {
  if(!_enabled) return 0;
  
  if(_bufIdx >= TWI_SERIAL_BUF_SIZE)
    flush();
  
  _buf[_bufIdx] = data;
  _bufIdx++;
  
  if(_bufIdx >= TWI_SERIAL_BUF_SIZE)
    flush();
  
  return 1;
}

__attribute__ ((always_inline))
void TWI_SERIAL::_flush(void) {
  if(!_enabled) return;
  
  _error = TWIUSI.start(_slaveAddr|TWI_WRITE);
  if(_error != 0) return;
  _error = TWIUSI.write(TWI_SERIAL_PRINT);
  if(_error != 0) return;
  _error = TWIUSI.write(_bufIdx);
  if(_error != 0) return;
  for(int i=0; i<_bufIdx; i++) {
    _error = TWIUSI.write(_buf[i]);
    if(_error != 0) return;
  }
  _error = TWIUSI.stop();
  if(_error != 0) return;
}

__attribute__ ((noinline))
void TWI_SERIAL::flush(void) {
  if(_bufIdx == 0) return;
  _flush();
  _bufIdx = 0;
}

__attribute__ ((noinline))
uint8_t TWI_SERIAL::available(void) {
  if(!_enabled) return 0;
  
  _error = TWIUSI.start(_slaveAddr|TWI_WRITE);
  if(_error != 0) return 0;
  _error = TWIUSI.write(TWI_SERIAL_AVAILABLE);
  if(_error != 0) return 0;
  _error = TWIUSI.stop();
  if(_error != 0) return 0;
  
  //delay(5);
  
  _error = TWIUSI.start(_slaveAddr|TWI_READ);
  if(_error != 0) return 0;
  uint8_t a;
  _error = TWIUSI.read(&a,true);
  if(_error != 0) return 0;
  _error = TWIUSI.stop();
  if(_error != 0) return 0;
  
  return a;
}

__attribute__ ((noinline))
uint8_t TWI_SERIAL::read(void) {
  if(!_enabled) return 0;
  
  _error = TWIUSI.start(_slaveAddr|TWI_WRITE);
  if(_error != 0) return 0;
  _error = TWIUSI.write(TWI_SERIAL_READ);
  if(_error != 0) return 0;
  _error = TWIUSI.stop();
  if(_error != 0) return 0;
  
  //delay(5);
  
  _error = TWIUSI.start(_slaveAddr|TWI_READ);
  if(_error != 0) return 0;
  uint8_t c;
  _error = TWIUSI.read(&c,true);
  if(_error != 0) return 0;
  _error = TWIUSI.stop();
  if(_error != 0) return 0;
  
  return c;
}
