#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include <TinyWireM.h>

// I2C_SERIAL
const uint8_t I2C_SERIAL = 0x08;

const uint8_t I2C_SERIAL_PRINT     = 'p';
const uint8_t I2C_SERIAL_AVAILABLE = 'a';
const uint8_t I2C_SERIAL_READ      = 'r';

// Pins
const uint8_t PIN_HR    = 1;
const uint8_t PIN_ALARM = 3;
const uint8_t PIN_HRPWR = 4;

// Globals
volatile uint8_t sleeping  = 0;
volatile uint8_t hr_level  = 0;

void setup() {
  wdt_interrupt_enable();
  
  // Calibrate clock
  OSCCAL = 0x54;
  
  // Disable unused HW
  power_adc_disable();
  
  // Configure IO
  digitalWrite(PIN_ALARM, LOW);
  digitalWrite(PIN_HRPWR, LOW);
  
  pinMode(PIN_HR, INPUT);
  pinMode(PIN_ALARM, OUTPUT);
  pinMode(PIN_HRPWR, OUTPUT);
  
  // Enable HR Interrupt
  GIMSK |= _BV(PCIE);   // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT1); // Use PB1 as interrupt pin
  
  // Start I2C
  TinyWireM.begin();

  i2c_serial_println("");
  i2c_serial_println("");
  i2c_serial_println("TinySleep 1.0");
  i2c_serial_println("");
  i2c_serial_println("Commands:");
  i2c_serial_println(" d  Dump EEPROM");
  i2c_serial_println("");
}

void loop() {
  uint8_t n = i2c_serial_available();
  
  while(n > 0) {
    char cmd = i2c_serial_read();
    n--;
    switch(cmd) {
      case 'd':
        dump_eeprom();
      break;
      default:
        i2c_serial_println("Unknown command: " + String(cmd));
    }
  }

  sleep();
  
  uint8_t hr = hr_measure();
  i2c_serial_println("HR: " + String(hr));
}

//##############
//# Interrupts #
//##############

ISR(WDT_vect) {
  // WakeUp!
  sleeping = 0;
}

ISR(PCINT0_vect) {
  hr_level = digitalRead(PIN_HR);
}

//#############
//# TinySleep #
//#############

uint8_t hr_measure() {
  digitalWrite(PIN_HRPWR, HIGH);
  digitalWrite(PIN_ALARM, HIGH);
  
  // Wait level up
  while(hr_level);
  // Wait level down
  while(!hr_level);
  // Wait level up
  while(hr_level);
  
  unsigned long time = millis();
  
  for(int i=0; i<10; i++) {
    // Wait level down
    while(!hr_level);
    // Wait level up
    while(hr_level);
    
    time = millis()-time;
    i2c_serial_println("T: " + String(time) + "(" + String(60000/time) + ")");
    time = millis();
  }
  
  digitalWrite(PIN_HRPWR, LOW);
  digitalWrite(PIN_ALARM, LOW);
  return 0;
}

void dump_eeprom() {
  String line;
  int addr = 0;
  
  // 32 lines of 16 bytes = 512 bytes
  for(int j=0; j<32; j++) {
    line = asHex(addr,4) + ": ";
    for(int i=0; i<16; i++){
      line = line + asHex(i,2);
      addr++;
    }
    i2c_serial_println(line);
  }
}

inline String asHex(int data, uint8_t len) {
  String hex = String(data,HEX);
  while(hex.length() < len) {
    hex = "0" + hex;
  }
  hex.toUpperCase();
  return hex;
}

//#############
//# Low Power #
//#############

void sleep() {
  // Set sleeping flag
  sleeping = 1;
  
  wdt_reset();
  wdt_interrupt_enable();
  
  // Set sleep to full power down.  Only external interrupts or 
  // the watchdog timer can wake the CPU!
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  while(sleeping) {
    // Enable sleep and enter sleep mode.
    sleep_mode();
    
    // CPU is now asleep and program execution completely halts!
    // Once awake, execution will resume at this point.
    // If the flag sleeping is 1, we will go to sleep again.
  }
  
  wdt_interrupt_disable();
  
  // When awake, disable sleep mode and turn on all devices.
  sleep_disable();
}

void wdt_interrupt_enable() {
  // Configure WDT
  cli();                         // Disable Interrupts
  WDTCR |= _BV(WDCE) | _BV(WDE); // Enable the WD Change Bit
  WDTCR =  _BV(WDIE) |           // Enable WDT Interrupt
           _BV(WDP0) | _BV(WDP3);// Set Timeout to ~8 seconds
  WDTCR |= _BV(WDIE);            // Enable watchdog as interrupt
  sei();                         // Enable Interrupts
}

void wdt_interrupt_disable() {
  // Configure WDT
  cli();                         // Disable Interrupts
  WDTCR |= _BV(WDCE) | _BV(WDE); // Enable the WD Change Bit
  WDTCR =  0;                    // Disable WDT
  sei();                         // Enable Interrupts
}

//##############
//# i2c Serial #
//##############

void i2c_serial_println(String data) {
  i2c_serial_print(data+"\n");
}

void i2c_serial_print(String data) {
  int n=data.length();
  int i=0;
  while(i < n) {
    TinyWireM.beginTransmission(I2C_SERIAL);
    TinyWireM.write(I2C_SERIAL_PRINT);
    uint8_t len = min(n-i,10);
    TinyWireM.write(len);
    for(int j=len; j>0; j--) {
      TinyWireM.write(data[i]);
      i++;
    }
    TinyWireM.endTransmission();
    delay(5);
  }
}

uint8_t i2c_serial_available() {
  TinyWireM.beginTransmission(I2C_SERIAL);
  TinyWireM.write(I2C_SERIAL_AVAILABLE);
  TinyWireM.endTransmission();
  delay(5);
  TinyWireM.requestFrom(I2C_SERIAL,1);
  uint8_t res = TinyWireM.receive();
  return res;
}

uint8_t i2c_serial_read() {
  TinyWireM.beginTransmission(I2C_SERIAL);
  TinyWireM.write(I2C_SERIAL_READ);
  TinyWireM.endTransmission();
  delay(5);
  TinyWireM.requestFrom(I2C_SERIAL,1);
  uint8_t res = TinyWireM.receive();
  return res;
}
