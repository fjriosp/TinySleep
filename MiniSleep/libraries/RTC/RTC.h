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
    volatile static int8_t   temp;
    static uint8_t _event;
    static int8_t am;
    static int8_t ah;
    
    static void     begin(void);
    static void     sync(void);
    static uint16_t ff(void);
    static void     setTime(uint16_t,uint8_t,uint8_t,uint8_t,uint8_t,uint8_t,uint16_t);
    static uint32_t millis(void);
    
    inline static int8_t getTempComp(void) {
      // From XTAL datasheet
      // ppm = -0,035 * (T-25)^2
      
      // ppm/hA = 1000000 / (freq * sec_in_hour / Prescaler)
      // ppm/hA = 1000000 / (32678*3600/32) = 0,271267361111
      
      // To Hourly Adjust
      // hA = (-0,035 / 0,271267361111) * (T-25)^2
      // hA = 0,129024 * (T-25)^2 ~= (33/256) * (T-25)^2
      int8_t dt = temp - 25;
      return ((dt*dt)*33)/256;
    }
};

extern _RTC RTC;

#endif
