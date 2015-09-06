#include "PCInt.h"

uint8_t oldPIN[fMAXPORT+1];

SIFUNC_CPP(void,pcint_enable,1,const uint8_t,pin)
SIFUNC_CPP(void,pcint_disable,1,const uint8_t,pin)

void pcint_default(void){};

#if defined(ENABLE_PORTB) && defined(PCINT0_vect)
ISR(PCINT0_vect) {
  const uint8_t port = fPB;
  const uint8_t pin = fPXToPINX(port);
  const uint8_t pcmsk = fPXToPCMSKX(port);
  const uint8_t flags = (pin ^ oldPIN[port]) & pcmsk;
  if(fBitRead(flags,0)) pcint_pb0();
  if(fBitRead(flags,1)) pcint_pb1();
  if(fBitRead(flags,2)) pcint_pb2();
  if(fBitRead(flags,3)) pcint_pb3();
  if(fBitRead(flags,4)) pcint_pb4();
  if(fBitRead(flags,5)) pcint_pb5();
  if(fBitRead(flags,6)) pcint_pb6();
  if(fBitRead(flags,7)) pcint_pb7();
  oldPIN[port] = pin;
}
#endif

#if defined(ENABLE_PORTC) && defined(PCINT1_vect)
ISR(PCINT1_vect) {
  const uint8_t port = fPC;
  const uint8_t pin = fPXToPINX(port);
  const uint8_t pcmsk = fPXToPCMSKX(port);
  const uint8_t flags = (pin ^ oldPIN[port]) & pcmsk;
  if(fBitRead(flags,0)) pcint_pc0();
  if(fBitRead(flags,1)) pcint_pc1();
  if(fBitRead(flags,2)) pcint_pc2();
  if(fBitRead(flags,3)) pcint_pc3();
  if(fBitRead(flags,4)) pcint_pc4();
  if(fBitRead(flags,5)) pcint_pc5();
  if(fBitRead(flags,6)) pcint_pc6();
  if(fBitRead(flags,7)) pcint_pc7();
  oldPIN[port] = pin;
}
#endif

#if defined(ENABLE_PORTD) && defined(PCINT2_vect)
ISR(PCINT2_vect) {
  const uint8_t port = fPD;
  const uint8_t pin = fPXToPINX(port);
  const uint8_t pcmsk = fPXToPCMSKX(port);
  const uint8_t flags = (pin ^ oldPIN[port]) & pcmsk;
  if(fBitRead(flags,0)) pcint_pd0();
  if(fBitRead(flags,1)) pcint_pd1();
  if(fBitRead(flags,2)) pcint_pd2();
  if(fBitRead(flags,3)) pcint_pd3();
  if(fBitRead(flags,4)) pcint_pd4();
  if(fBitRead(flags,5)) pcint_pd5();
  if(fBitRead(flags,6)) pcint_pd6();
  if(fBitRead(flags,7)) pcint_pd7();
  oldPIN[port] = pin;
}
#endif
