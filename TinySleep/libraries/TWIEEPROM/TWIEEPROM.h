 
#ifndef TWIEEPROM_h
#define TWIEEPROM_h

#include <Arduino.h>

class TWI_EEPROM
{
  private:
    static uint8_t _devaddr;    // slave addr
    static uint8_t _error;      // error on last read?
    
  public:
    TWI_EEPROM(uint8_t devaddr);
    uint8_t write(unsigned int addr, uint8_t* buf, uint8_t len);
    uint8_t read (unsigned int addr, uint8_t* buf, uint8_t len);
    
    inline uint8_t isError() { return _error; }
};

#endif

