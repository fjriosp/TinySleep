#include "TWIUSI.h"

// Constants
const uint8_t USISR_8b = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)| // Prepare register value to: Clear flags, and
                         (0x0<<USICNT0);                                // set USI to shift 8 bits i.e. count 16 clock edges.
const uint8_t USISR_1b = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)| // Prepare register value to: Clear flags, and
                         (0xE<<USICNT0);                                // set USI to shift 1 bit i.e. count 2 clock edges.

// Initialize object
TWI_USI TWIUSI = TWI_USI();

// Private
__attribute__ ((noinline))
void _TWIUSI_pasv_idle(void) {
  DDR_USI  &= ~(1<<PIN_USI_SDA); // Enable SDA as input.
  DDR_USI  &= ~(1<<PIN_USI_SCL); // Enable SCL as input.
  PORT_USI |= (1<<PIN_USI_SDA);  // Release SDA.
  PORT_USI |= (1<<PIN_USI_SCL);  // Release SCL.
}

__attribute__ ((always_inline))
void _TWIUSI_act_idle(void) {
  DDR_USI  |= (1<<PIN_USI_SDA);  // Enable SDA as output.
  DDR_USI  |= (1<<PIN_USI_SCL);  // Enable SCL as output.
  PORT_USI |= (1<<PIN_USI_SDA);  // Release SDA.
  PORT_USI |= (1<<PIN_USI_SCL);  // Release SCL.
}

__attribute__ ((noinline))
uint8_t _TWIUSI_transfer()
{
  do
  { 
    _delay_us(T2_TWI);
    USICR |= (1<<USITC);                   // Generate positve SCL edge.
    while( !(PIN_USI & (1<<PIN_USI_SCL)) );// Wait for SCL to go high.
    _delay_us(T4_TWI);
    USICR |= (1<<USITC);                   // Generate negative SCL edge.
  }while( !(USISR & (1<<USIOIF)) );        // Check for transfer complete.
  
  _delay_us(T2_TWI);
  uint8_t data  = USIDR;                   // Read out data.
  USIDR = 0xFF;                            // Release SDA.

  DDR_USI  |= (1<<PIN_USI_SDA);            // Enable SDA as output.
  
  return data;                             // Return the data from the USIDR
}

// Public
__attribute__ ((noinline))
void TWI_USI::begin(void) {
  // Prepare bus passive IDLE
  _TWIUSI_pasv_idle();
  
  USIDR = 0xFF;                                // Preload dataregister with "released level" data.
  USICR = (0<<USISIE)|(0<<USIOIE)|             // Disable Interrupts.
          (1<<USIWM1)|(0<<USIWM0)|             // Set USI in Two-wire mode.
          (1<<USICS1)|(0<<USICS0)|(1<<USICLK)| // Software stobe as counter clock source
          (0<<USITC);
  USISR = USISR_8b;
}

__attribute__ ((noinline))
uint8_t TWI_USI::start(uint8_t addr) {
  // Prepare bus active IDLE
  _TWIUSI_act_idle();
  while( !(PORT_USI & (1<<PIN_USI_SCL)) ); // Verify that SCL becomes high.
  _delay_us(T2_TWI);

  /* Generate Start Condition */
  PORT_USI &= ~(1<<PIN_USI_SDA);           // Force SDA LOW.
  _delay_us(T4_TWI);
  PORT_USI &= ~(1<<PIN_USI_SCL);           // Pull SCL LOW.

  PORT_USI |= (1<<PIN_USI_SDA);            // Release SDA.
  
  if( !(USISR & (1<<USISIF)) ) {
    stop();
    return TWIUSI_MISSING_START_CON;
  }
  
  return write(addr);
}

__attribute__ ((noinline))
uint8_t TWI_USI::write(uint8_t byte) {
  USIDR = byte;                    // Setup data.
  USISR = USISR_8b;                // Send 8 bits on bus.
  _TWIUSI_transfer();
  
  // Verify (N)ACK
  DDR_USI &= ~(1<<PIN_USI_SDA);    // Enable SDA as input.
  USISR = USISR_1b;
  if( _TWIUSI_transfer() & 0x01 ) { // NACK
    stop();
    return TWIUSI_NO_ACK;
  }
  
  return 0;
}

__attribute__ ((noinline))
uint8_t TWI_USI::read(uint8_t *byte, bool last) {
  DDR_USI  &= ~(1<<PIN_USI_SDA); // Enable SDA as input.
  USISR = USISR_8b;              // Read 8 bits on bus.
  *byte = _TWIUSI_transfer();
  
  // Send (N)ACK
  if(last) USIDR = 0xFF;         // Load NACK to confirm End Of Transmission.
  else     USIDR = 0x00;         // Load ACK. Set data register bit 7 (output for SDA) low.
  USISR = USISR_1b;
  _TWIUSI_transfer();
  
  return 0;
}

__attribute__ ((noinline))
uint8_t TWI_USI::stop(void) {
  // Prepare bus
  PORT_USI &= ~(1<<PIN_USI_SDA);           // Pull SDA low.
  PORT_USI |= (1<<PIN_USI_SCL);            // Release SCL.
  while( !(PIN_USI & (1<<PIN_USI_SCL)) );  // Wait for SCL to go high.
  _delay_us(T4_TWI);
  
  // Stop condition
  PORT_USI |= (1<<PIN_USI_SDA);            // Release SDA.
  _delay_us(T2_TWI);
  
  // Go to passive IDLE
  _TWIUSI_pasv_idle();
  
  if( !(USISR & (1<<USIPF)) ) {
    return TWIUSI_MISSING_STOP_CON;
  }
  
  return 0;
}
