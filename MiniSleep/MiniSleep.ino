#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>
#include <RTC.h>

const uint32_t    MAX_USART   = 60000;
volatile uint8_t  using_usart = 0;
volatile uint32_t last_usart  = 0;

const uint8_t EE_OSCCAL = 0x00;

void setup() {
  OSCCAL = EEPROM.read(EE_OSCCAL);
  
  pinMode(13,OUTPUT);
  
  ADCSRA = 0;             // disable ADC
  power_all_disable ();   // Turn off modules
  power_timer2_enable (); // Turn on timer2
  power_twi_enable ();    // Turn on TWI
  power_usart0_enable (); // Turn on Serial
  
  RTC.begin();
  
  // Interrupt on PD0 (RXD)
  cli();
  PCICR  |= (1 << PCIE2);    // set PCIE2 to enable PCMSK2 scan
  PCMSK2 |= (1 << PCINT16);  // set PCINT16 to trigger an interrupt on state change 
  sei();
  
  Serial.begin(9600);
} 

void loop() {
  menu();
  if(using_usart && (RTC.millis()-last_usart) > MAX_USART) {
    using_usart = 0;
  }
  sleep();
  RTC.sync(); // Needed to refresh the async registers
}

void menu() {
  while(Serial.available()>0) {
    uint8_t c = Serial.read();
    switch(c) {
      case 'h':
        menu_help();
        break;
      case 's':
        while(Serial.available()<19);
        menu_set();
        break;
      case 'p':
        printRTC();
        break;
      default:
        Serial.print(F("Unknown command: "));
        Serial.println(c);
        break;
    }
  }
}

void menu_help() {
  Serial.println(F(" MiniSleep v1.0"));
  Serial.println();
  Serial.println(F(" Commands:"));
  Serial.println(F("   h - Show this help."));
  Serial.println(F("   s - Set the rtc time."));
  Serial.println(F("   p - Print the rtc time."));
}

uint16_t readN4() {
  uint16_t n;
  n = Serial.read()-'0';
  n = n*10 + Serial.read()-'0';
  n = n*10 + Serial.read()-'0';
  n = n*10 + Serial.read()-'0';
  return n;
}

uint8_t readN2() {
  uint16_t n;
  n = Serial.read()-'0';
  n = n*10 + Serial.read()-'0';
  return n;
}

void menu_set() {
  uint16_t tmp16;
  uint8_t  tmp8;
  
  RTC.yy = readN4()-2000;
  Serial.read(); // Ignore '-'
  RTC.mm = readN2();
  Serial.read(); // Ignore '-'
  RTC.dd = readN2();
  Serial.read(); // Ignore ' '
  RTC.hh = readN2();
  Serial.read(); // Ignore ':'
  RTC.mi = readN2();
  Serial.read(); // Ignore ':'
  RTC.ss = readN2();
}

void printRTC() {
  char buf[32];
  sprintf_P(buf,PSTR("%04d-%02d-%02d %02d:%02d:%02d.%03d"),2000+RTC.yy,RTC.mm,RTC.dd,RTC.hh,RTC.mi,RTC.ss,RTC.ff());
  Serial.println(buf);
}

void sleep() {
  if(using_usart) {
    set_sleep_mode (SLEEP_MODE_IDLE);
  } else {
    Serial.flush();
    set_sleep_mode (SLEEP_MODE_PWR_SAVE);
  }
  
  cli();                // timed sequence follows
  sleep_enable();
  sleep_bod_disable();  // turn off brown-out in software
  sei();                // guarantees next instruction executed
  sleep_cpu ();         // sleep within 3 clock cycles of above
}

ISR(PCINT2_vect) {
  using_usart = 1;
  last_usart  = RTC.millis();
}

