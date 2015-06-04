#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include <TinyWireM.h>
#include <TWISerial.h>

//5.430 github


// TWISerial
const uint8_t TWI_SERIAL_ADDR = 0x08;

// Pins
const uint8_t PIN_HR    = 1;
const uint8_t PIN_ALARM = 3;
const uint8_t PIN_HRPWR = 4;

// Globals
volatile uint8_t sleeping  = 0;
volatile uint8_t hr_level  = 0;

TWI_SERIAL TWISerial = TWI_SERIAL(TWI_SERIAL_ADDR);

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

  TWISerial.println();
  TWISerial.println();
  TWISerial.println(F("TinySleep 1.0"));
  TWISerial.println();
  TWISerial.println(F("Commands:"));
  TWISerial.println(F(" d Dump EEPROM"));
  TWISerial.println();
  TWISerial.flush();
}

void loop() {
  uint8_t n = TWISerial.available();
  
  while(n > 0) {
    char cmd = TWISerial.read();
    n--;
    switch(cmd) {
      case 'd':
        dump_eeprom();
      break;
      default:
        TWISerial.print(F("Unknown command: "));
        TWISerial.println(cmd);
    }
  }

  sleep();
  
  uint8_t hr = hr_measure();
  TWISerial.print(F("HR: "));
  TWISerial.println(hr);
  TWISerial.flush();
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
    TWISerial.print(F("T: "));
    TWISerial.print(time);
    TWISerial.print(60000/time);
    TWISerial.println(F(")"));
    TWISerial.flush();
    time = millis();
  }
  
  digitalWrite(PIN_HRPWR, LOW);
  digitalWrite(PIN_ALARM, LOW);
  return 0;
}

void dump_eeprom() {
  char buf[16];
  int addr = 0;
  
  // 32 lines of 16 bytes = 512 bytes
  for(int j=0; j<32; j++) {
    toHex(buf,addr,4);
    TWISerial.print(buf);
    TWISerial.print(F(": "));
    for(int i=0; i<16; i++){
      toHex(buf,i,2);
      TWISerial.print(buf);
      addr++;
    }
    TWISerial.println();
  }
  TWISerial.flush();
}

void toHex(char* buf, unsigned int n, uint8_t len) {
  buf[len]='\0';
  for(int i=len-1; i>=0; i--) {
    uint8_t v = n & 0x0F;
    if(v < 10)
      buf[i] = '0'+v;
    else
      buf[i] = 'A'+(v-10);
    n >>= 4;
  }
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
