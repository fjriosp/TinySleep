#ifndef RTC_h
#define RTC_h

#include <Arduino.h>

class _RTC {
  public:
    volatile static uint32_t cc;
    volatile static uint8_t  ss;
    volatile static uint8_t  mi;
    volatile static uint8_t  hh;
    volatile static uint8_t  dd;
    volatile static uint8_t  mm;
    volatile static uint8_t  yy;
    static uint8_t _event;
    static int8_t am;
    static int8_t ah;
    static int8_t ad;
    
    static void     begin(void);
    static void     sync(void);
    static uint16_t ff(void);
    static void     setTime(uint16_t,uint8_t,uint8_t,uint8_t,uint8_t,uint8_t,uint16_t);
    static uint32_t millis(void);
};

extern _RTC RTC;

#endif
