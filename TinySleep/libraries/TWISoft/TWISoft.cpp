#include "TWISoft.h"

//From DS1307 datasheet
/*
#define T_BUF    4.7
#define T_LOW    4.7
#define T_HIGH   4
#define T_HD_STA 4
#define T_SU_STA 4.7
#define T_SU_STO 4.7
*/
#define T_BUF    0
#define T_LOW    0
#define T_HIGH   0
#define T_HD_STA 0
#define T_SU_STA 0
#define T_SU_STO 0

#define TWIDDR  DDRB
#define TWIPORT PORTB
#define TWIPIN  PINB

__attribute__((always_inline)) void _ALL_output(){ TWIDDR  |=  (_BV(TWISDA) | _BV(TWISCL)); }
__attribute__((always_inline)) void _ALL_input() { TWIDDR  &= ~(_BV(TWISDA) | _BV(TWISCL)); }
__attribute__((always_inline)) void _ALL_H()     { TWIPORT |=  (_BV(TWISDA) | _BV(TWISCL)); }

__attribute__((always_inline)) void _SDA_output() { TWIDDR  |=  (_BV(TWISDA)); }
__attribute__((always_inline)) void _SDA_input()  { TWIDDR  &= ~(_BV(TWISDA)); }
__attribute__((always_inline)) void _SDA_H()      { TWIPORT |=  (_BV(TWISDA)); }
__attribute__((always_inline)) void _SDA_L()      { TWIPORT &= ~(_BV(TWISDA)); }

__attribute__((always_inline)) void _SCL_output() { TWIDDR  |=  (_BV(TWISCL)); }
__attribute__((always_inline)) void _SCL_input()  { TWIDDR  &= ~(_BV(TWISCL)); }
__attribute__((always_inline)) void _SCL_H()      { TWIPORT |=  (_BV(TWISCL)); }
__attribute__((always_inline)) void _SCL_L()      { TWIPORT &= ~(_BV(TWISCL)); }

__attribute__((always_inline)) void _wait_SDA()   { while(!(TWIPIN & _BV(TWISDA))); }
__attribute__((always_inline)) void _wait_SCL()   { while(!(TWIPIN & _BV(TWISCL))); }

uint8_t TWISoft::begin() {
  _ALL_input();
  _ALL_H();
}

uint8_t TWISoft::start(uint8_t addr) {
  _ALL_input();
  _ALL_H();
  _wait_SDA();
  _wait_SCL();
  
  // Start condition
  _delay_us(T_BUF);
  _SDA_L();
  _SDA_output();
  _delay_us(T_HD_STA);

  // Write addr byte and return ack/nack
  return write(addr);
}

uint8_t TWISoft::write(uint8_t data) {
  for(uint8_t i=0;i<8;i++) {
    _SCL_L();
    _SCL_output();
    if(data & 0x80) _SDA_H();
    else            _SDA_L();
    
    _delay_us(T_LOW);
    
    _SCL_input();
    _SCL_H();
    _wait_SCL();
    
    _delay_us(T_HIGH);
    
    data <<= 1;
  }
  
  // Read ACK/NACK
  _SDA_input();
  _SDA_H();
  
  _SCL_output();
  _SCL_L();
  
  _delay_us(T_LOW);
  
  _SCL_input();
  _SCL_H();
  _wait_SCL();
  
  uint8_t ack = TWIPIN & _BV(TWISDA);
  _delay_us(T_HIGH);
  
  return ack;
}

uint8_t TWISoft::read(uint8_t* data, bool last) {
  // Pullup on SDA
  _SDA_input();
  _SDA_H();
  
  for(uint8_t i=0;i<8;i++) {
    _SCL_L();
    _SCL_output();

    _delay_us(T_LOW);
    
    _SCL_input();
    _SCL_H();
    _wait_SCL();
    *data &= (TWIPIN & _BV(TWISDA)) << TWISDA;
    _delay_us(T_HIGH);
    
    *data <<= 1;
  }
  
  // Send ACK/NACK
  if(last) {
    _SDA_H();
  } else {
    _SDA_L();
    _SDA_output();
  }
  
  _SCL_L();
  _SCL_output();
  
  _delay_us(T_LOW);
  
  _SCL_input();
  _SCL_H();
  _wait_SCL();
  _delay_us(T_HIGH);
  
  return 0;
}

uint8_t TWISoft::stop() {
  _SDA_L();
  _SDA_output();
  
  _SCL_input();
  _SCL_H();
  _wait_SCL();
  
  // Stop condition
  _delay_us(T_SU_STO);
  _SDA_H();
  
  // Go to bus Idle
  _ALL_input();
  _ALL_H();
  
  return 0;
} 
