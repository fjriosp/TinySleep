#ifndef TWISerial_h
#define TWISerial_h

#include <Arduino.h>

#define TWI_SERIAL_BUF_SIZE    10 // bytes in message buffer

class TWI_SERIAL : public Print
{
  private:
    static uint8_t _buf[];      // holds I2C send and receive data
    static uint8_t _bufIdx;     // current number of bytes in the send buff
    static uint8_t _slaveAddr;  // slave I2C addr
    static uint8_t _error;      // error on last read?
    static uint8_t _enabled;    // is enabled?
    virtual size_t write(uint8_t);
    
  public:
    TWI_SERIAL(uint8_t);
    void    begin();
    uint8_t read();
    uint8_t available(); 
    void    flush();
    
    inline uint8_t isError() { return _error; }
    inline void setEnabled(uint8_t e) { _enabled = e; }
};

extern TWI_SERIAL TWISerial;

#endif

