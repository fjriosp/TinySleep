#ifndef RTC_h
#define RTC_h

#include <Arduino.h>

class _RTC {
  public:
    volatile static uint32_t cnt;
    volatile static uint8_t  ss;
    volatile static uint8_t  mi;
    volatile static uint8_t  hh;
    volatile static uint8_t  dd;
    volatile static uint8_t  mm;
    volatile static uint8_t  yy;
    
    static void     begin(void);
    static uint32_t millis(void);
};

extern _RTC RTC;

#endif
