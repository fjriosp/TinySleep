#ifndef TWISerial_h
#define TWISerial_h

#include <Arduino.h>

#define TWI_SERIAL_BUF_SIZE    18              // bytes in message buffer

class TWI_SERIAL : public Print
{
  private:
    static uint8_t _buf[];      // holds I2C send and receive data
    static uint8_t _bufIdx;     // current number of bytes in the send buff
    static uint8_t _slaveAddr;  // slave I2C addr
    virtual size_t write(uint8_t);
    
  public:
    TWI_SERIAL(uint8_t);
    void    begin();
    uint8_t read(); 
    uint8_t available(); 
    void    flush(void);
};

extern TWI_SERIAL TWISerial;

#endif

