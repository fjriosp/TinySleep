#include <TinyWireM.h>
#include "TWISerial.h"

// Initialize Class Variables
uint8_t TWI_SERIAL::TWISERIAL_Buf[TWISERIAL_BUF_SIZE]; // holds I2C send and receive data
uint8_t TWI_SERIAL::TWISERIAL_BufIdx = 0;                    // current number of bytes in the send buff

// Constructors
TWI_SERIAL::TWI_SERIAL() {
}

// Public Methods
void TWI_SERIAL::begin() {
  TinyWireM.begin();
}



size_t  write(uint8_t);
uint8_t read(); 
uint8_t available(); 
void    flush(void);