#ifndef __PCINT_H__
#define __PCINT_H__

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pin map:
// D0-D7           = PCINT 16-23 = PCIR2 = PD = PCIE2 = PCMSK2
// D8-D13          = PCINT 0-5   = PCIR0 = PB = PCIE0 = PCMSK0
// A0-A5 (D14-D19) = PCINT 8-13  = PCIR1 = PC = PCIE1 = PCMSK1
#if defined(PCINT_D0) || defined (PCINT_D1) || defined(PCINT_D2) || defined(PCINT_D3) || defined(PCINT_D4) || defined(PCINT_D5) || defined(PCINT_D6) || defined(PCINT_D7)
#define PCINT_USE_PD
#endif

#if defined(PCINT_D8) || defined (PCINT_D9) || defined(PCINT_D10) || defined(PCINT_D11) || defined(PCINT_D12) || defined(PCINT_D13)
#define PCINT_USE_PB
#endif

#if defined(PCINT_A0) || defined (PCINT_A1) || defined(PCINT_A2) || defined(PCINT_A3) || defined(PCINT_A4) || defined(PCINT_A5)
#define PCINT_USE_PC
#endif

#ifdef PCINT_D0
extern inline void PCINT_D0 (void);
#endif
#ifdef PCINT_D1
extern inline void PCINT_D1 (void);
#endif
#ifdef PCINT_D2
extern inline void PCINT_D2 (void);
#endif
#ifdef PCINT_D3
extern inline void PCINT_D3 (void);
#endif
#ifdef PCINT_D4
extern inline void PCINT_D4 (void);
#endif
#ifdef PCINT_D5
extern inline void PCINT_D5 (void);
#endif
#ifdef PCINT_D6
extern inline void PCINT_D6 (void);
#endif
#ifdef PCINT_D7
extern inline void PCINT_D7 (void);
#endif
#ifdef PCINT_D8
extern inline void PCINT_D8 (void);
#endif
#ifdef PCINT_D9
extern inline void PCINT_D9 (void);
#endif
#ifdef PCINT_D10
extern inline void PCINT_D10 (void);
#endif
#ifdef PCINT_D11
extern inline void PCINT_D11 (void);
#endif
#ifdef PCINT_D12
extern inline void PCINT_D12 (void);
#endif
#ifdef PCINT_D13
extern inline void PCINT_D13 (void);
#endif
#ifdef PCINT_A0
extern inline void PCINT_A0 (void);
#endif
#ifdef PCINT_A1
extern inline void PCINT_A1 (void);
#endif
#ifdef PCINT_A2
extern inline void PCINT_A2 (void);
#endif
#ifdef PCINT_A3
extern inline void PCINT_A3 (void);
#endif
#ifdef PCINT_A4
extern inline void PCINT_A4 (void);
#endif
#ifdef PCINT_A5
extern inline void PCINT_A5 (void);
#endif

#ifdef PCINT_USE_PB
volatile uint8_t oldPINB = 0;
#endif
#ifdef PCINT_USE_PC
volatile uint8_t oldPINC = 0;
#endif
#ifdef PCINT_USE_PD
volatile uint8_t oldPIND = 0;
#endif

void pcint_enable(uint8_t pin) {
  uint8_t mask;
  if(pin < 8) {
#ifdef PCINT_USE_PD
    mask = bit(pin);
    PCMSK2  |=   mask;          // set PDx to trigger an interrupt on state change
    oldPIND &= ~(mask);         // save the current PDx value
    oldPIND |=  (PIND & mask);
#endif
  } else if(pin < 14) {
#ifdef PCINT_USE_PB
    mask = bit(pin - 8);
    PCMSK0  |=   mask;          // set PBx to trigger an interrupt on state change
    oldPINB &= ~(mask);         // save the current PBx value
    oldPINB |=  (PINB & mask);
#endif
  } else if(pin < 20) {
#ifdef PCINT_USE_PC
    mask = bit(pin - 14);
    PCMSK1  |=   mask;          // set PCx to trigger an interrupt on state change
    oldPINC &= ~(mask);         // save the current PCx value
    oldPINC |=  (PINC & mask);
#endif
  }
}

void pcint_disable(uint8_t pin) {
  uint8_t mask;
  if(pin < 8) {
#ifdef PCINT_USE_PD
    mask = bit(pin);
    PCMSK2  &= ~(mask);         // disable PDx interrupt
#endif
  } else if(pin < 14) {
#ifdef PCINT_USE_PB
    mask = bit(pin-8);
    PCMSK0  &= ~(mask);         // disable PBx interrupt
#endif
  } else if(pin < 20) {
#ifdef PCINT_USE_PC
    mask = bit(pin-14);
    PCMSK1  &= ~(mask);         // disable PCx interrupt
#endif
  }
}

#ifdef PCINT_USE_PD
// PIND interrupts
ISR(PCINT2_vect) {
  const uint8_t newPIND = PIND;
  const uint8_t flags = (newPIND ^ oldPIND) & PCMSK2;
  
#ifdef PCINT_D0
  if(bitRead(flags,0)) PCINT_D0();
#endif
#ifdef PCINT_D1
  if(bitRead(flags,1)) PCINT_D1();
#endif
#ifdef PCINT_D2
  if(bitRead(flags,2)) PCINT_D2();
#endif
#ifdef PCINT_D3
  if(bitRead(flags,3)) PCINT_D3();
#endif
#ifdef PCINT_D4
  if(bitRead(flags,4)) PCINT_D4();
#endif
#ifdef PCINT_D5
  if(bitRead(flags,5)) PCINT_D5();
#endif
#ifdef PCINT_D6
  if(bitRead(flags,6)) PCINT_D6();
#endif
#ifdef PCINT_D7
  if(bitRead(flags,7)) PCINT_D7();
#endif
  
  oldPIND = newPIND;
}
#endif


#ifdef PCINT_USE_PB
// PINB interrupts
ISR(PCINT0_vect) {
  const uint8_t newPINB = PINB;
  const uint8_t flags = (newPINB ^ oldPINB) & PCMSK0;

#ifdef PCINT_D8
  if(bitRead(flags,0)) PCINT_D8();
#endif
#ifdef PCINT_D9
  if(bitRead(flags,1)) PCINT_D9();
#endif
#ifdef PCINT_D10
  if(bitRead(flags,2)) PCINT_D10();
#endif
#ifdef PCINT_D11
  if(bitRead(flags,3)) PCINT_D11();
#endif
#ifdef PCINT_D12
  if(bitRead(flags,4)) PCINT_D12();
#endif
#ifdef PCINT_D13
  if(bitRead(flags,5)) PCINT_D13();
#endif
 
  oldPINB = newPINB;
}
#endif

#ifdef PCINT_USE_PC
// PINC interrupts
ISR(PCINT1_vect) {
  const uint8_t newPINC = PINC;
  const uint8_t flags = (newPINC ^ oldPINC) & PCMSK1;
  
#ifdef PCINT_A0
  if(bitRead(flags,0)) PCINT_A0();
#endif
#ifdef PCINT_A1
  if(bitRead(flags,1)) PCINT_A1();
#endif
#ifdef PCINT_A2
  if(bitRead(flags,2)) PCINT_A2();
#endif
#ifdef PCINT_A3
  if(bitRead(flags,3)) PCINT_A3();
#endif
#ifdef PCINT_A4
  if(bitRead(flags,4)) PCINT_A4();
#endif
#ifdef PCINT_A5
  if(bitRead(flags,5)) PCINT_A5();
#endif
  
  oldPINC = newPINC;
}
#endif

#ifdef __cplusplus
}
#endif

#endif

