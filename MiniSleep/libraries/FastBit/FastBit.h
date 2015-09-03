#ifndef __FASTBIT_H__
#define __FASTBIT_H__

#include <Arduino.h>

#define CREATE_LOOKUP_TABLE(name , type,  ...)     \
  const type PROGMEM name##_PGM[] = {__VA_ARGS__}; \
  const type         name[]       = {__VA_ARGS__};

#define READ_LOOKUP_TABLE(name,idx)   \
  (__builtin_constant_p(idx)          \
  ? (name[idx])                       \
  : pgm_read_byte(name##_PGM+(idx)))

// bit* macros
CREATE_LOOKUP_TABLE(lt_fBit,uint8_t,_BV(0),_BV(1),_BV(2),_BV(3),_BV(4),_BV(5),_BV(6),_BV(7));
#define fBit(b)          (READ_LOOKUP_TABLE(lt_fBit,b))
#define fBitSet(r,b)     ((r) |=  fBit(b))
#define fBitClear(r,b)   ((r) &= ~fBit(b))
#define fBitRead(r,b)    ((r) &   fBit(b))
#define fBitWrite(r,b,v) ((v) ? fBitSet(r,b) : fBitClear(r,b))

// specific macros
CREATE_LOOKUP_TABLE(lt_fPinToPORTX,uint16_t,
  /*  0 */ (uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,(uint16_t)&PORTD,
  /*  8 */ (uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,(uint16_t)&PORTB,
  /* 14 */ (uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC,(uint16_t)&PORTC
);


#define fPinToPORTX(p) ((volatile uint8_t *)READ_LOOKUP_TABLE(lt_fPinToPORTX,p))
#define fPinToDDRX(p)  (fPinToPORTX(p)+1)
#define fPinToPINX(p)  (fPinToPORTX(p)+2)

#endif

