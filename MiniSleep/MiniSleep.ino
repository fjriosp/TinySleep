#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>
#include <RTC.h>

// Low Power
const uint32_t    MAX_USART   = 10*1000;
volatile uint8_t  using_usart = 0;
volatile uint32_t usart_ttl  = 0;

// EEPROM
const uint8_t EE_OSCCAL = 0x00;

// Temp
const uint32_t TEMP_TIME = 30*60*1000;
uint32_t temp_next   = 0;
uint16_t temp        = 0;

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

const uint8_t STACK_CANARY=0xAA;

void setup() {
  OSCCAL = EEPROM.read(EE_OSCCAL);
  
  pinMode(13,OUTPUT);
  
  ADCSRA = 0;            // disable ADC
  power_all_disable();   // Turn off all modules
  power_timer2_enable(); // Turn on timer2
  power_twi_enable();    // Turn on TWI
  power_usart0_enable(); // Turn on Serial
  
  RTC.begin();
  
  // Interrupt on PD0 (RXD)
  cli();
  PCICR  |= (1 << PCIE2);    // set PCIE2 to enable PCMSK2 scan
  PCMSK2 |= (1 << PCINT16);  // set PCINT16 to trigger an interrupt on state change 
  sei();
  
  Serial.begin(9600);
} 

void loop() {
  temp_loop();
  menu_loop();
  sleep();
  RTC.sync(); // Needed to refresh the async registers
}

//#################
//# Internal Temp #
//#################
void temp_loop() {
  if(RTC.millis() >= temp_next) {
    power_adc_enable();   // Turn on ADC
    // Enable the ADC with 8MHz/128=62.5kHz
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
    // Set the internal reference and mux.
    ADMUX   = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
    
    _delay_ms(1);
    
    // Discard the first read
    rawAnalogReadWithSleep();
    temp = rawAnalogReadWithSleep();
  
    ADMUX  = 0;           // Disable internal ref
    ADCSRA = 0;           // Disable ADC
    power_adc_disable();  // Turn off ADC
    
    temp_next = RTC.millis() + TEMP_TIME;
  }
}

uint16_t rawAnalogReadWithSleep() {
  ADCSRA |= 1<<ADIF; // Clear interrupt flag
  ADCSRA |= 1<<ADIE; // Enable ADC interrupt
  ADCSRA |= 1<<ADSC; // Start conversion

  // Sleep while conversion
  set_sleep_mode( SLEEP_MODE_IDLE );
  sleep_enable();

  // Loop until the conversion is finished
  do {
    RTC.sync();
    sei();          // Ensure interrupts are enabled before sleeping
    sleep_cpu();
    cli();          // Disable interrupts so the while below is performed without interruption
  // Conversion finished?  If not, loop.
  } while(ADCSRA & (1<<ADSC));

  
  sleep_disable();  // No more sleeping
  sei();            // Enable interrupts

  ADCSRA &= ~(1<<ADIE); // Disable ADC interrupt
  
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
    // Read while available or end of command
    char c;
    do {
      c = Serial.read();
      cmdline[cmdlen] = c;
      cmdlen++;
      cmdlen %= 32;
    } while(Serial.available()>0 && c!='\r');

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
        default:
          Serial.print(F("Unknown command: "));
          Serial.println(c);
          break;
      }
      cmdlen = 0;
    }
  }
  if(using_usart && (RTC.millis() > usart_ttl)) {
    using_usart = 0;
    // Interrupt on PD0 (RXD)
    cli();
    PCICR  |= (1 << PCIE2);    // set PCIE2 to enable PCMSK2 scan
    PCMSK2 |= (1 << PCINT16);  // set PCINT16 to trigger an interrupt on state change 
    sei();
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
  sprintf_P(buf,PSTR("A%hhd,%hhd,%hhd"),RTC.am,RTC.ah,RTC.ad);
  Serial.println(buf);
}

void menu_set_adjust() {
  sscanf_P(cmdline+1,PSTR("%hhd,%hhd,%hhd"),&RTC.am,&RTC.ah,&RTC.ad);
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
  temp_next=0;
  Serial.println(temp);
}

//#############
//# Low Power #
//#############
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
  RTC.sync();
  usart_ttl = RTC.millis() + MAX_USART;
  PCMSK2 &= ~(1 << PCINT16);  // disable PCINT16 to trigger an interrupt on state change 
  PCICR  &= ~(1 << PCIE2);    // disable PCIE2 to ignore PCMSK2 scan
}

//#############
//# Mem Trace #
//#############
__attribute__ ((naked,section (".init1")))
void mem_paint(void) {
  uint8_t *p = __brkval==0 ? &__heap_start : __brkval;

  while((uint16_t)p <= SP)
  {
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
