#ifndef TWIUSI_h
#define TWIUSI_h

// Timing
#define T2_TWI 5 // >4,7us
#define T4_TWI 4 // >4,0us

// Errors
#define TWIUSI_NO_ACK            0x01  // The slave did not acknowledge  the address
#define TWIUSI_MISSING_START_CON 0x02  // Generated Start Condition not detected on bus
#define TWIUSI_MISSING_STOP_CON  0x04  // Generated Stop Condition not detected on bus

// R/W Flags
#define TWI_WRITE 0
#define TWI_READ  1

// Pins
#if defined(__AVR_AT90Mega169__) | defined(__AVR_ATmega169__) | \
    defined(__AVR_AT90Mega165__) | defined(__AVR_ATmega165__) | \
    defined(__AVR_ATmega325__) | defined(__AVR_ATmega3250__) | \
    defined(__AVR_ATmega645__) | defined(__AVR_ATmega6450__) | \
    defined(__AVR_ATmega329__) | defined(__AVR_ATmega3290__) | \
    defined(__AVR_ATmega649__) | defined(__AVR_ATmega6490__)
    #define DDR_USI             DDRE
    #define PORT_USI            PORTE
    #define PIN_USI             PINE
    #define PORT_USI_SDA        PORTE5
    #define PORT_USI_SCL        PORTE4
    #define PIN_USI_SDA         PINE5
    #define PIN_USI_SCL         PINE4
#endif

#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__) | \
    defined(__AVR_AT90Tiny26__) | defined(__AVR_ATtiny26__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB0
    #define PORT_USI_SCL        PORTB2
    #define PIN_USI_SDA         PINB0
    #define PIN_USI_SCL         PINB2
#endif

#if defined(__AVR_ATtiny84__) | defined(__AVR_ATtiny44__)
    #  define DDR_USI           DDRA
    #  define PORT_USI          PORTA
    #  define PIN_USI           PINA
    #  define PORT_USI_SDA      PORTA6
    #  define PORT_USI_SCL      PORTA4
    #  define PIN_USI_SDA       PINA6
    #  define PIN_USI_SCL       PINA4
#endif

#if defined(__AVR_AT90Tiny2313__) | defined(__AVR_ATtiny2313__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB5
    #define PORT_USI_SCL        PORTB7
    #define PIN_USI_SDA         PINB5
    #define PIN_USI_SCL         PINB7
#endif

#include <Arduino.h>

void    TWIUSI_begin(void);
uint8_t TWIUSI_start(uint8_t addr);
uint8_t TWIUSI_write(uint8_t byte);
uint8_t TWIUSI_read(uint8_t *byte, bool last);
uint8_t TWIUSI_stop(void);

#endif