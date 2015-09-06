#ifndef __FASTBIT_H__
#define __FASTBIT_H__

#include <Arduino.h>
#include "SIFunc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CREATE_LOOKUP_TABLE(name , type,  ...)     \
  const type PROGMEM name##_PGM[] = {__VA_ARGS__}; \
  const type         name[]       = {__VA_ARGS__};

#define READ_LOOKUP_TABLE(name,idx)   \
  (__builtin_constant_p(idx)          \
  ? (name[idx])                       \
  : pgm_read_byte(name##_PGM+(idx)))
  
// Ports
#define fPA 0
#define fPB 1
#define fPC 2
#define fPD 3
#define fPE 4
#define fPF 5

// Specific data
#if defined(__AVR_ATmega328P__)

#define fMAXPORT fPD

CREATE_LOOKUP_TABLE(lt_fPinToPX,uint8_t,
  /*  0 */ fPD,fPD,fPD,fPD,fPD,fPD,fPD,fPD,
  /*  8 */ fPB,fPB,fPB,fPB,fPB,fPB,
  /* 14 */ fPC,fPC,fPC,fPC,fPC,fPC,
);

CREATE_LOOKUP_TABLE(lt_fPinToMask,uint8_t,
  /*  0 */ _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),_BV(6),_BV(7),
  /*  8 */ _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),
  /* 14 */ _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),
);

CREATE_LOOKUP_TABLE(lt_fPXToPORTX,uint16_t,
  (uint16_t)NULL, 
  (uint16_t)&PORTB, 
  (uint16_t)&PORTC, 
  (uint16_t)&PORTD, 
  (uint16_t)NULL, 
  (uint16_t)NULL, 
);

CREATE_LOOKUP_TABLE(lt_fPXToPCMSKX,uint16_t,
  (uint16_t)NULL, 
  (uint16_t)&PCMSK0, 
  (uint16_t)&PCMSK1, 
  (uint16_t)&PCMSK2, 
  (uint16_t)NULL, 
  (uint16_t)NULL,
);

#else
#error Unknown board (FastBit.h)
#endif

// BitMask
CREATE_LOOKUP_TABLE(lt_fBit,uint8_t,
  _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),_BV(6),_BV(7)
);

#define fMaskSet(r,m)      ((r) |=  (m))
#define fMaskClear(r,m)    ((r) &= ~(m))
#define fMaskRead(r,m)     ((r) &   (m))
#define fMaskWrite(r,m,v)  ((v) ? fMaskSet(r,m) : fMaskClear(r,m))

#define fBit(b)            (READ_LOOKUP_TABLE(lt_fBit,b))

#define fBitSet(r,b)       (fMaskSet(r,fBit(b)))
#define fBitClear(r,b)     (fMaskClear(r,fBit(b)))
#define fBitRead(r,b)      (fMaskRead(r,fBit(b)))
#define fBitWrite(r,b,v)   (fMaskWrite(r,fBit(b),v))

// IO port macros
#define fPXToPORTXReg(px)  ((volatile uint8_t *)READ_LOOKUP_TABLE(lt_fPXToPORTX,px))
#define fPXToPORTX(px)     (*(fPXToPORTXReg(px)  ))
#define fPXToDDRX(px)      (*(fPXToPORTXReg(px)-1))
#define fPXToPINX(px)      (*(fPXToPORTXReg(px)-2))

#define fPXToPCMSKXReg(px) ((volatile uint8_t *)READ_LOOKUP_TABLE(lt_fPXToPCMSKX,px))
#define fPXToPCMSKX(px)    (*(fPXToPCMSKXReg(px)))

#define fPinToMask(p)      (READ_LOOKUP_TABLE(lt_fPinToMask,p))
#define fPinToPX(p)        (READ_LOOKUP_TABLE(lt_fPinToPX,p))


SIFUNC_H(void,fPinMode,2,const uint8_t,p,const uint8_t,mode) {
  const uint8_t mask = fPinToMask(p);
  const uint8_t port = fPinToPX(p);
  
  volatile uint8_t * const rddr  = &fPXToDDRX(port);
  volatile uint8_t * const rport = &fPXToPORTX(port);
  
  if(mode == OUTPUT) {
    fMaskSet(*rddr,mask);
  } else {
    fMaskClear(*rddr,mask);
    if(mode == INPUT_PULLUP) {
      fMaskSet(*rport,mask);
    } else {
      fMaskClear(*rport,mask);
    }
  }
}

SIFUNC_H(void,fDigitalWrite,1,const uint8_t,p,const uint8_t,v) {
  const uint8_t mask = fPinToMask(p);
  const uint8_t port = fPinToPX(p);
  
  volatile uint8_t * const rport = &fPXToPORTX(port);
  
  if(v) {
    fMaskSet(*rport,mask);
  } else {
    fMaskClear(*rport,mask);
  }
}

SIFUNC_H(uint8_t,fDigitalRead,1,const uint8_t,p) {
  const uint8_t mask = fPinToMask(p);
  const uint8_t port = fPinToPX(p);
  
  volatile uint8_t * const rpin = &fPXToPINX(port);
  
  return fMaskRead(*rpin,mask);
}

#ifdef __cplusplus
}
#endif

#endif

