#ifndef __PCINT_H__
#define __PCINT_H__

#include <../FastBit/FastBit.h>

// Undefine this to reduce the flash size
#define ENABLE_PORTB
#define ENABLE_PORTC
#define ENABLE_PORTD

// Board specific defines
#if defined(__AVR_ATmega328P__)

#define PCISR(p) void CAT(PCISR_,p)(void)
// PORTD (0-7)
#define PCISR_0  pcint_pd0
#define PCISR_1  pcint_pd1
#define PCISR_2  pcint_pd2
#define PCISR_3  pcint_pd3
#define PCISR_4  pcint_pd4
#define PCISR_5  pcint_pd5
#define PCISR_6  pcint_pd6
#define PCISR_7  pcint_pd7
// PORTB (8-14)
#define PCISR_8  pcint_pb0
#define PCISR_9  pcint_pb1
#define PCISR_10 pcint_pb2
#define PCISR_11 pcint_pb3
#define PCISR_12 pcint_pb4
#define PCISR_13 pcint_pb5
// PORTC (14-19) (A0-A5)
#define PCISR_14 pcint_pc0
#define PCISR_15 pcint_pc1
#define PCISR_16 pcint_pc2
#define PCISR_17 pcint_pc3
#define PCISR_18 pcint_pc4
#define PCISR_19 pcint_pc5

#else
#error Unknown board (PCInt.h)
#endif

// End of config

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t oldPIN[];

inline void pcint_begin(void) {
#if defined(PCMSK0)
  PCMSK0 = 0;
  PCICR |= fBit(PCIE0);
#endif
#if defined(PCMSK1)
  PCMSK1 = 0;
  PCICR |= fBit(PCIE1);
#endif
#if defined(PCMSK2)
  PCMSK2 = 0;
  PCICR |= fBit(PCIE2);
#endif
}

// void pcint_enable(const uint8_t pin);
SIFUNC_H(void,pcint_enable,1,const uint8_t,pin) {
  const uint8_t mask = fPinToMask(pin);
  const uint8_t port = fPinToPX(pin);
  
  volatile uint8_t * const rpcmsk = &fPXToPCMSKX(port);
  volatile uint8_t * const rpin   = &fPXToPINX(port);
  
  const uint8_t sreg = SREG;
  cli();

  *rpcmsk      |=   mask;
  oldPIN[port] &= ~(mask);
  oldPIN[port] |=  (*rpin & mask);
  
  SREG = sreg;
}

// void pcint_disable(const uint8_t pin);
SIFUNC_H(void,pcint_disable,1,const uint8_t,pin) {
  const uint8_t mask = fPinToMask(pin);
  const uint8_t port = fPinToPX(pin);
  volatile uint8_t * const rpcmsk = &fPXToPCMSKX(port);
  
  *rpcmsk &= ~(mask);
}

// Vectors
#ifdef ENABLE_PORTB
__attribute__((weak)) void pcint_pb0(void);
__attribute__((weak)) void pcint_pb1(void);
__attribute__((weak)) void pcint_pb2(void);
__attribute__((weak)) void pcint_pb3(void);
__attribute__((weak)) void pcint_pb4(void);
__attribute__((weak)) void pcint_pb5(void);
__attribute__((weak)) void pcint_pb6(void);
__attribute__((weak)) void pcint_pb7(void);
#endif

#ifdef ENABLE_PORTC
__attribute__((weak)) void pcint_pc0(void);
__attribute__((weak)) void pcint_pc1(void);
__attribute__((weak)) void pcint_pc2(void);
__attribute__((weak)) void pcint_pc3(void);
__attribute__((weak)) void pcint_pc4(void);
__attribute__((weak)) void pcint_pc5(void);
__attribute__((weak)) void pcint_pc6(void);
__attribute__((weak)) void pcint_pc7(void);
#endif

#ifdef ENABLE_PORTD
__attribute__((weak)) void pcint_pd0(void);
__attribute__((weak)) void pcint_pd1(void);
__attribute__((weak)) void pcint_pd2(void);
__attribute__((weak)) void pcint_pd3(void);
__attribute__((weak)) void pcint_pd4(void);
__attribute__((weak)) void pcint_pd5(void);
__attribute__((weak)) void pcint_pd6(void);
__attribute__((weak)) void pcint_pd7(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

