#include <Arduino.h>
#include <FastBit.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>
#include <RTC.h>

#define PCINT_D0  pcint_usart
//#define PCINT_D2  empty
//#define PCINT_D10 empty
//#define PCINT_D11 empty
//#define PCINT_A2  empty
//#define PCINT_A3  empty

#include <PCInt.h>

// Neeeded for conditional global variables
void empty(void) {}

// IO
static const uint8_t USART_RX_PIN =  0;
static const uint8_t DHT_PIN      =  2;
static const uint8_t BTN2_PIN     =  2;
static const uint8_t BTN0_PIN     = 10;
static const uint8_t HR_PIN       = 11;
static const uint8_t BTN1_PIN     = A2;
static const uint8_t BTN3_PIN     = A3;

// Comment this to disable DHT22 calibration
//#include <DHT.h>

#ifdef DHT_H
DHT dht(DHT_PIN, DHT22);
static const uint8_t T_LOG_SIZE = 128;
uint8_t  log_i = 0;
uint16_t atemp_log[T_LOG_SIZE];
uint16_t itemp_log[T_LOG_SIZE];
uint16_t etemp_log[T_LOG_SIZE];
#endif

// Low Power
volatile uint8_t  LPSR   = 0; // Low Power Status Register
static const    uint8_t  LPUSRE = 0; // USART Enabled
static const    uint8_t  LPUSRW = 1; // USART WarmUp
static const    uint8_t  LPADCE = 2; // ADC Enabled

// USART
static const uint32_t    MAX_USART  = 10*1000UL;
volatile uint32_t usart_ttl  = 0;

// EEPROM
static const uint8_t EE_OSCCAL = 0x00;
static const uint8_t EE_RTC_AM = 0x01;
static const uint8_t EE_RTC_AH = 0x02;

// Temp
static const uint8_t TEMP_OFFSET = 323;
static const uint8_t TEMP_MULT   = 216;
static const uint8_t TEMP_SHIFT  = 8; // DIV 256

static const uint32_t TEMP_TIME = 60*60*1000UL;
static const uint32_t TEMP_WARM = 100L;
uint32_t temp_next = 0;

// Menu
char cmdline[32];
uint8_t cmdlen = 0;

// Mem
extern uint8_t __data_start;
extern uint8_t __data_end;
extern uint8_t __bss_start;
extern uint8_t __bss_end;
extern uint8_t __heap_start;
extern uint8_t *__brkval;

static const uint8_t STACK_CANARY = 0xAA;

void setup() {
  OSCCAL = EEPROM.read(EE_OSCCAL);
  
  ADCSRA = 0;            // disable ADC
  power_all_disable();   // Turn off all modules
  power_timer2_enable(); // Turn on timer2
  power_twi_enable();    // Turn on TWI
  power_usart0_enable(); // Turn on Serial
  
  RTC.begin();
  RTC.am = EEPROM.read(EE_RTC_AM);
  RTC.ah = EEPROM.read(EE_RTC_AH);
  
  // Pin change interrupts
  cli();
  PCMSK0  = 0;
  bitSet(PCICR,PCIE0);  // set PCIE0 to enable PCMSK0 (PB) scan
  PCMSK1  = 0;
  bitSet(PCICR,PCIE1);  // set PCIE1 to enable PCMSK1 (PC) scan
  PCMSK2  = 0;
  bitSet(PCICR,PCIE2);  // set PCIE2 to enable PCMSK2 (PD) scan
  pcint_enable(USART_RX_PIN);
  sei();
  
  Serial.begin(9600);
  
#ifdef DHT_H
  dht.begin();
#endif
} 

void loop() {
  temp_loop();
  menu_loop();
  sleep();
  RTC.sync(); // Needed to refresh the async registers
}

//################
//# PCInterrupts #
//################

void pcint_usart(void) {
  // D0/PD0 interrupt
  bitSet(LPSR,LPUSRE);
  bitSet(LPSR,LPUSRW);
  RTC.sync();
  usart_ttl = RTC.millis() + MAX_USART;
  pcint_disable(USART_RX_PIN);
}

//#################
//# Internal Temp #
//#################
void temp_loop() {
  if(RTC.millis() > temp_next) {
    // If ADC was off, I need to WarmUp
    if(!bitRead(LPSR,LPADCE)) {
      power_adc_enable();   // Turn on ADC
      // Enable the ADC with 8MHz/128=62.5kHz
      ADCSRA = bit(ADEN) | bit(ADPS2) | bit(ADPS1) | bit(ADPS0);
      // Set the internal reference and mux.
      ADMUX  = bit(REFS1) | bit(REFS0) | bit(MUX3);

      bitSet(LPSR,LPADCE);
      temp_next = RTC.millis() + TEMP_WARM;
    // If ADC was enabled, I can read
    } else {
      // Mean 8 reads
      uint16_t t = 0;
      for(uint8_t i=0; i<8; i++) {
        t += rawAnalogReadWithSleep();
      }
      // Adjust before truncate (DIV 8)
      t = (t - 2) / 8;  
      
      // Now calc the temperature
      int8_t t2 = t - TEMP_OFFSET;  // SUB and truncate to 8bits
      RTC.temp = (t2 * TEMP_MULT) >> TEMP_SHIFT;

#ifdef DHT_H
      atemp_log[log_i] = ADCW;
      itemp_log[log_i] = RTC.temp;
      etemp_log[log_i] = dht.readTemperature(false)*10;
      log_i = (log_i+1) % T_LOG_SIZE;
#endif

      ADMUX  = 0;           // Disable internal ref
      ADCSRA = 0;           // Disable ADC
      power_adc_disable();  // Turn off ADC
    
      bitClear(LPSR,LPADCE);
      temp_next = RTC.millis() + TEMP_TIME;
    }
  }
}

uint16_t rawAnalogReadWithSleep() {
  // Wait until ADC is IDLE
  while(bitRead(ADCSRA,ADSC));
  
  bitSet(ADCSRA,ADIF); // Clear interrupt flag
  bitSet(ADCSRA,ADIE); // Enable ADC interrupt

  // Sleep while conversion
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  // Loop until the conversion is finished
  do {
    RTC.sync();
    sei();          // Ensure interrupts are enabled before sleeping
    sleep_cpu();
    cli();          // Disable interrupts so the while below is performed without interruption
  // Conversion finished?  If not, loop.
  } while(bitRead(ADCSRA,ADSC));

  sleep_disable();  // No more sleeping
  sei();            // Enable interrupts

  bitClear(ADCSRA,ADIE); // Disable ADC interrupt
  
  return ADCW;
}

ISR(ADC_vect) {
  // Ignore
}

//###############
//# Serial Menu #
//###############
void menu_loop() {
  while(Serial.available()>0) {
    usart_ttl = RTC.millis() + MAX_USART;
    char c = 0;
    // USART WarmUp
    if(bitRead(LPSR,LPUSRW)) {
      // Flush serial input
      while(Serial.available()>0) {
        c = Serial.read();
        _delay_us(1500);
      }
      bitClear(LPSR,LPUSRW);
      Serial.println(F("ERROR: WarmUp"));
    }
    // Read while available or end of command
    while(Serial.available()>0 && c!='\r') {
      c = Serial.read();
      cmdline[cmdlen] = c;
      cmdlen++;
      cmdlen %= 32;
    } 

    // If end of command
    if(c == '\r') {
      cmdline[cmdlen] = '\0';
      switch(cmdline[0]) {
        case 'h':
          menu_help();
          break;
        case 's':
          menu_set_rtc();
          break;
        case 'p':
          menu_print_rtc();
          break;
        case 'a':
          menu_print_adjust();
          break;
        case 'A':
          menu_set_adjust();
          break;
        case 'm':
          menu_mem_stats();
          break;
        case 't':
          menu_print_temp();
          break;
#ifdef DHT_H
        case 'T':
          menu_print_temp_log();
          break;
#endif
        default:
          Serial.print(F("ERROR: Unknown command: "));
          Serial.println(cmdline[0]);
          break;
      }
      cmdlen = 0;
    }
  }
  if(bitRead(LPSR,LPUSRE) && (RTC.millis() > usart_ttl)) {
    bitClear(LPSR,LPUSRE);
    pcint_enable(USART_RX_PIN);
  }
}

void menu_help() {
  Serial.println(F(" MiniSleep v1.0"));
  Serial.println();
  Serial.println(F(" Commands:"));
  Serial.println(F("   h - Show this help."));
  Serial.println(F("   s - Set the rtc time."));
  Serial.println(F("   p - Print the rtc time."));
  Serial.println(F("   a - Print the rtc adjust."));
  Serial.println(F("   A - Set the rtc adjust."));
  Serial.println(F("   m - Show mem stats."));
  Serial.println(F("   t - Show temperature."));
#ifdef DHT_H
  Serial.println(F("   T - Show temperature log."));
#endif
}

void menu_set_rtc() {
  uint16_t yy,ff;
  uint8_t  mm,dd,hh,mi,ss;
  sscanf_P(cmdline+1,PSTR("%hu-%hhu-%hhu %hhu:%hhu:%hhu.%hu"),&yy,&mm,&dd,&hh,&mi,&ss,&ff);
  RTC.setTime(yy,mm,dd,hh,mi,ss,ff);
}

void menu_print_rtc() {
  char buf[32];
  sprintf_P(buf,PSTR("%04hd-%02hhd-%02hhd %02hhd:%02hhd:%02hhd.%03hd"),2000+RTC.yy,RTC.mm,RTC.dd,RTC.hh,RTC.mi,RTC.ss,RTC.ff());
  Serial.println(buf);
}

void menu_print_adjust() {
  char buf[16];
  sprintf_P(buf,PSTR("A%hhd,%hhd"),RTC.am,RTC.ah);
  Serial.println(buf);
}

void menu_set_adjust() {
  sscanf_P(cmdline+1,PSTR("%hhd,%hhd"),&RTC.am,&RTC.ah);
  EEPROM.write(EE_RTC_AM,RTC.am);
  EEPROM.write(EE_RTC_AH,RTC.ah);
}

void menu_mem_stats() {
  uint16_t total_size = RAMEND-((uint16_t)&__data_start)+1;
  uint16_t data_size  = ((uint16_t)&__data_end)-((uint16_t)&__data_start);
  uint16_t bss_size   = ((uint16_t)&__bss_end)-((uint16_t)&__bss_start);
  uint16_t heap_size  = ((uint16_t)__brkval) == 0 ? 0 : (((uint16_t)__brkval)-((uint16_t)&__heap_start));
  uint16_t stack_size = RAMEND-SP;
  uint16_t total_used = data_size+bss_size+heap_size+stack_size;
  Serial.print(F("Total:  "));
  Serial.println(total_size);
  Serial.print(F("Data:   "));
  Serial.println(data_size);
  Serial.print(F("BSS:    "));
  Serial.println(bss_size);
  Serial.print(F("Heap:   "));
  Serial.println(heap_size);
  Serial.print(F("Stack:  "));
  Serial.println(stack_size);
  Serial.print(F("Free:   "));
  Serial.println(RAMEND-total_used);
  Serial.print(F("Unused: "));
  Serial.println(mem_unused());
}

void menu_print_temp() {
  temp_next=0;  // Force a new read
  Serial.println(RTC.temp);
}

#ifdef DHT_H
void menu_print_temp_log() {
  char buf[16];
  for(uint8_t i=0; i<T_LOG_SIZE; i++) {
    sprintf_P(buf,PSTR("%3hhu - %4hu (%4hu) <%4hu>"),i,atemp_log[i],itemp_log[i],etemp_log[i]);
    Serial.print(buf);
    if(i == log_i) {
      Serial.print(F(" <--"));
    }
    Serial.println();
  }
}
#endif

//#############
//# Low Power #
//#############
void sleep() {
  Serial.flush(); // Force a flush to prevent TX interrupts
  
  if(LPSR & (bit(LPUSRE) | bit(LPADCE))) {
    set_sleep_mode (SLEEP_MODE_IDLE);
  } else {
    set_sleep_mode (SLEEP_MODE_PWR_SAVE);
  }
  
  cli();                // timed sequence follows
  sleep_enable();
  sleep_bod_disable();  // turn off brown-out in software
  sei();                // guarantees next instruction executed
  sleep_cpu();          // sleep within 3 clock cycles of above
}

//#############
//# Mem Trace #
//#############
__attribute__ ((naked,section (".init1")))
void mem_paint(void) {
  uint8_t *p = __brkval==0 ? &__heap_start : __brkval;

  while((uint16_t)p <= SP) {
      *p = STACK_CANARY;
      p++;
  }
}

uint16_t mem_unused() {
  uint8_t *p = __brkval==0 ? &__heap_start : __brkval;

  uint16_t c = 0;
  uint16_t maxc = 0;
  // Search max contiguous bytes with value STACK_CANARY
  while(p <= (uint8_t*)SP) {
    if(*p == STACK_CANARY) {
      c++;
    } else if(c > maxc) {
      maxc = c;
    }
    p++;
  }

  return maxc;
}

