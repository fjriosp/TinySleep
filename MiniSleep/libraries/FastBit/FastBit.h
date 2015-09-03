#ifndef __FASTBIT_H__
#define __FASTBIT_H__

#include <Arduino.h>

// Utils
#define CREATE_LOOKUP_TABLE(name , type,  ...)     \
  const type PROGMEM name##_PGM[] = {__VA_ARGS__}; \
  const type         name[]       = {__VA_ARGS__};

#define READ_LOOKUP_TABLE(name,idx)   \
  (__builtin_constant_p(idx)          \
  ? (name[idx])                       \
  : pgm_read_byte(name##_PGM+(idx)))

// Specific data
CREATE_LOOKUP_TABLE(lt_fPinToPORTX,uint16_t,
  /*  0 */ (uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,
  /*  8 */ (uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,
  /* 14 */ (uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC,
);

CREATE_LOOKUP_TABLE(lt_fPinToPCMSKX,uint16_t,
  /*  0 */ (uint16_t)&PCMSK2,(uint16_t)&PCMSK2,(uint16_t)&PCMSK2,(uint16_t)&PCMSK2,(uint16_t)&PCMSK2,(uint16_t)&PCMSK2,(uint16_t)&PCMSK2,(uint16_t)&PCMSK2,
  /*  8 */ (uint16_t)&PCMSK0,(uint16_t)&PCMSK0,(uint16_t)&PCMSK0,(uint16_t)&PCMSK0,(uint16_t)&PCMSK0,(uint16_t)&PCMSK0,
  /* 14 */ (uint16_t)&PCMSK1,(uint16_t)&PCMSK1,(uint16_t)&PCMSK1,(uint16_t)&PCMSK1,(uint16_t)&PCMSK1,(uint16_t)&PCMSK1,
);

CREATE_LOOKUP_TABLE(lt_fPinToMask,uint8_t,
  /*  0 */ _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),_BV(6),_BV(7),
  /*  8 */ _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),
  /* 14 */ _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),
);

// BitMask
CREATE_LOOKUP_TABLE(lt_fBit,uint8_t,
  _BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),_BV(6),_BV(7)
);

#define fMaskSet(r,m)     ((r) |=  (m))
#define fMaskClear(r,m)   ((r) &= ~(m))
#define fMaskRead(r,m)    ((r) &   (m))
#define fMaskWrite(r,m,v) ((v) ? fMaskSet(r,m) : fMaskClear(r,m))

#define fBit(b)          (READ_LOOKUP_TABLE(lt_fBit,b))
#define fBitSet(r,b)     (fMaskSet(r,fBit(b)))
#define fBitClear(r,b)   (fMaskClear(r,fBit(b)))
#define fBitRead(r,b)    (fMaskRead(r,fBit(b)))
#define fBitWrite(r,b,v) (fMaskWrite(r,fBit(b),v))

// IO port macros
#define fPinToPORTXReg(p) ((volatile uint8_t *)READ_LOOKUP_TABLE(lt_fPinToPORTX,p))
#define fPORTX(p)         (*(fPinToPORTXReg(p)  ))
#define fDDRX(p)          (*(fPinToPORTXReg(p)-1))
#define fPINX(p)          (*(fPinToPORTXReg(p)-2))

#define fPinToPCMSKXReg(p) ((volatile uint8_t *)READ_LOOKUP_TABLE(lt_fPinToPCMSKX,p))
#define fPCMSKX(p)         (*(fPinToPCMSKXReg(p)))

#define fPinToMask(p) (READ_LOOKUP_TABLE(lt_fPinToMask,p))

#define fDigitalWrite(p,v) (fMaskWrite(fPORTX(p),fPinToMask(p),v))
#define fDigitalRead(p)    (fMaskRead(fPINX(p),fPinToMask(p)))

#define fPinMode(p,mode)                  \
{                                         \
  if((mode) == OUTPUT) {                  \
    fMaskSet(fDDRX(p),fPinToMask(p));     \
  }else if((mode) == INPUT) {             \
    fMaskClear(fPORTX(p),fPinToMask(p));  \
    fMaskClear(fDDRX(p),fPinToMask(p));   \
  }else if((mode) == INPUT_PULLUP) {      \
    fMaskSet(fPORTX(p),fPinToMask(p));    \
    fMaskClear(fDDRX(p),fPinToMask(p));   \
  }                                       \
}

#endif

