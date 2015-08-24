#ifndef __FASTBIT_H__
#define __FASTBIT_H__

#include <Arduino.h>

static const uint8_t fBitMask8[] = {bit(0),bit(1),bit(2),bit(3),bit(4),bit(5),bit(6),bit(7)};

// Redefine the bit macros
#define bit(b)          (fBitMask8[b])
#define bitRead(r,b)    (r  &  bit(b))
#define bitSet(r,b)     (r |=  bit(b))
#define bitClear(r,b)   (r &= ~bit(b))
#define bitWrite(r,b,v) (v ? bitSet(r,b) : bitClear(r,b))

#endif

