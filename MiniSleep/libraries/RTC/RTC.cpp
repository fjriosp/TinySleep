#include "RTC.h"

volatile uint32_t _RTC::cc = 0;
volatile uint8_t  _RTC::ss = 0;
volatile uint8_t  _RTC::mi = 0;
volatile uint8_t  _RTC::hh = 0;
volatile uint8_t  _RTC::dd = 1;
volatile uint8_t  _RTC::mm = 1;
volatile uint8_t  _RTC::yy = 0;

volatile int8_t   _RTC::temp = 25;

const uint8_t EV_MINUTE = 0;
const uint8_t EV_HOUR   = 1;
uint8_t  _RTC::_event   = EV_MINUTE;

int8_t _RTC::am = 0;
int8_t _RTC::ah = 0;

void _RTC::begin(void) {
  // Configure Timer2
  cli();                // Disable Interrupts
  TIMSK2 = 0;           // Disable Timer2 interrupts
  ASSR   |= 1<<AS2;     // Timer2 async
  TCNT2  = 0;           // Reset Timer2
  // 1/32 Prescaler
  // TIMER2_OVF every 32.768 kHz / 32 / 256 = 4 Hz = 250ms
  TCCR2A = 0;
  TCCR2B = (1<<CS21)|(1<<CS20);
  
  // Wait TC2 Updated
  while(ASSR & ((1<<TCN2UB)|(1<<TCR2AUB)|(1<<TCR2BUB))); 
  
  OCR2A  = 0x7F;        // COMPA used to adjust
  // Wait OCR2A Updated
  while(ASSR & (1<<OCR2AUB));
  
  TIFR2  = (1<<TOV2);   // Clear interrupt flag
  TIMSK2|= (1<<TOIE2);	// Enable Overflow Interrupt
  
  sei();                // Enable Interrupts
}

void _RTC::sync(void) {
  TCCR2A = 0x00;         //Dummy
  while(ASSR & (1<<TCR2AUB));
}

uint16_t _RTC::ff(void) {
  uint8_t c;
  uint8_t t;
  uint8_t oldSREG = SREG;
  
  cli();
  c = cc;
  t = TCNT2;
  
  if((TIFR2 & (1<<TOV2)) && (t < 255))
    c++;

  SREG = oldSREG;
  
  // c incremented every 250ms
  // c = c % 4 => c < 1 sec
  c &= 0x03;
  
  // c*250 + t*250/256
  // c*250 + t*125/128
  return (c*250) + (t*125)/128;
}

void _RTC::setTime(uint16_t pyy, uint8_t pmm, uint8_t pdd, 
		   uint8_t phh, uint8_t pmi, uint8_t pss, 
		   uint16_t pff) {
  uint8_t oldSREG = SREG;
  
  if(pyy >= 2000)
    pyy -= 2000;
  
  cli();
  yy    = pyy;
  mm    = pmm;
  dd    = pdd;
  hh    = phh;
  mi    = pmi;
  ss    = pss;
  cc   &= ~0x03;
  cc   += pff / 256;
  TCNT2 = pff % 256;
  
  while(ASSR & (1<<TCN2UB));

  SREG = oldSREG;
}

uint32_t _RTC::millis(void) {
  uint32_t c;
  uint8_t  t;
  uint8_t oldSREG = SREG;
  
  cli();
  c = cc;
  t = TCNT2;
  
  if((TIFR2 & (1<<TOV2)) && (t < 255))
    c++;

  SREG = oldSREG;
  
  // c incremented every 250ms
  // c*250 + t*250/256
  // c*250 + t*125/128
  return (c*250) + (t*125)/128;
}

ISR(TIMER2_OVF_vect) {
  // Speedup CPU @8MHz
  uint8_t old_clk = CLKPR;
  CLKPR = _BV(CLKPCE);
  CLKPR = 0; // 1/1 = 8MHz
  
  RTC.cc++;
  if((RTC.cc & 0x03) == 0x00) {
    uint8_t _ss = RTC.ss+1;
    if(_ss > 59) {
      _ss = 0;
      uint8_t _mi = RTC.mi+1;
      RTC._event = EV_MINUTE;
      TIFR2  = (1<<OCF2A);  // Clear interrupt flag
      TIMSK2|= (1<<OCIE2A); // Enable COMPA Interrupt
      if(_mi > 59) {
	_mi = 0;
	uint8_t _hh = RTC.hh+1;
	RTC._event = EV_HOUR;
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
  }
  
  // Restore previous speed
  CLKPR = _BV(CLKPCE);
  CLKPR = old_clk;
}

ISR(TIMER2_COMPA_vect) {
  RTC.sync();
  
  TIMSK2 &= ~(1<<OCIE2A);  // Disable COMPA Interrupt
  
  switch(RTC._event) {
    case EV_HOUR:
      TCNT2 += RTC.getTempComp() + RTC.ah;
      break;
    case EV_MINUTE:
      TCNT2 += RTC.am;
      break;
  }
  while(ASSR & (1<<TCN2UB));
}
