#include "RTC.h"

volatile uint32_t _RTC::cnt = 0;
volatile uint8_t  _RTC::ss  = 0;
volatile uint8_t  _RTC::mi  = 0;
volatile uint8_t  _RTC::hh  = 0;
volatile uint8_t  _RTC::dd  = 0;
volatile uint8_t  _RTC::mm  = 0;
volatile uint8_t  _RTC::yy  = 0;

void _RTC::begin(void) {
  // Configure Timer2
  cli();                // Disable Interrupts
  TIMSK2 = 0;           // Disable Timer2 interrupts
  ASSR   |= 1<<AS2;     // Timer2 async
  TCNT2  = 0;           // Reset Timer2
  // 1/128 Prescaler
  TCCR2A = 0;
  TCCR2B = (1<<CS22)|(1<<CS20);
  // Wait TC2 Updated
  while(ASSR & ((1<<TCN2UB)|(1<<OCR2AUB)|(1<<OCR2BUB)|(1<<TCR2AUB)|(1<<TCR2BUB))); 
  TIFR2  = (1<<TOV2);   // Clear interrupts flags
  TIMSK2|= (1<<TOIE2);	// Enable Overflow Interrupt
  sei();                // Enable Interrupts
}

uint32_t _RTC::millis(void) {
  uint32_t s;
  uint8_t  t;
  uint8_t oldSREG = SREG;
  
  cli();
  s = cnt;
  t = TCNT2;
  
  if((TIFR2 & (1<<TOV2)) && (t < 255))
    s++;

  SREG = oldSREG;
  
  // sec * 1000 + t*1000/256
  // sec * 1000 + t*125/32
  return (s*1000) + (t*125)/32;
}

ISR(TIMER2_OVF_vect) {
  // Speedup CPU @8MHz
  uint8_t old_clk = CLKPR;
  CLKPR = _BV(CLKPCE);
  CLKPR = 0; // 1/1 = 8MHz
  
  // Runs every 32.768 kHz / 128 / 256 = 1 Hz
  RTC.cnt++;
  uint8_t _ss = RTC.ss+1;
  if(_ss > 59) {
    _ss = 0;
    uint8_t _mi = RTC.mi+1;
    if(_mi > 59) {
      _mi = 0;
      uint8_t _hh = RTC.hh+1;
      if(_hh > 23) {
        _hh = 0;
        uint8_t _dd = RTC.dd + 1;
        uint8_t _mm = RTC.mm;
        uint8_t _yy = RTC.yy;
        if(_dd > 31) {
          _dd = 1;
          _mm++;
        } else if(_dd > 30) {
          if((_mm==4) || (_mm==6) || (_mm==9) || (_mm==11)) {
            _dd = 1;
            _mm++;
          }
        } else if(_dd > 29) {
          if(_mm == 2) {
            _dd = 1;
            _mm++;
          }
        } else if(_dd > 28) {
          if(_mm == 2) {
            // Leap years in range 2000 - 2255 are:
            // yy!=2000 && yy%4==0
            if(_yy==2000 || !(_yy%4)) {
              _dd = 1;
              _mm++;
            }
          }
        }
        if(_mm > 13) {
          _mm = 1;
          _yy++;
        }
        RTC.dd = _dd;
        RTC.mm = _mm;
        RTC.yy = _yy;
      }
      RTC.hh = _hh;
    }
    RTC.mi = _mi;
  }
  RTC.ss = _ss;
  
  // Restore previous speed
  CLKPR = _BV(CLKPCE);
  CLKPR = old_clk;
}
