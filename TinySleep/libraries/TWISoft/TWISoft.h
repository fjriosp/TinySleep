#ifndef TWISoft_h
#define TWISoft_h

#include <Arduino.h>

#define TWI_READ  1
#define TWI_WRITE 0

#define TWISDA 0
#define TWISCL 2

class TWISoft
{
  public:
    static uint8_t begin();
    static uint8_t start(uint8_t addr);
    static uint8_t write(uint8_t data);
    static uint8_t read(uint8_t* data, bool last);
    static uint8_t stop();
};

#endif

 
