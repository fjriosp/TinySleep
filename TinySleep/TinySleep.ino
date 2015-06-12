#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include <TinyWireM.h>
#include <TWISerial.h>

#define TIMEOUT(t,v) if(millis()>=(t)) return (v);

// TWISerial
const uint8_t TWI_SERIAL_ADDR = 0x08;

// Pins
const uint8_t PIN_HR    = 1;
const uint8_t PIN_ALARM = 3;
const uint8_t PIN_HRPWR = 4;

// Globals
volatile uint8_t sleeping  = 0;
volatile uint8_t hr_beats  = 0;

TWI_SERIAL TWISerial = TWI_SERIAL(TWI_SERIAL_ADDR);

// HR
const uint8_t HR_NBEATS = 5;

const uint8_t HR_OFF     = 0;
const uint8_t HR_WARM    = 1;
const uint8_t HR_MEASURE = 2;

uint8_t hr_status;
unsigned long hr_ontime = 0;
unsigned long hr_start;
unsigned long hr_time;

uint16_t hr = 0;

// LowPower
unsigned long real_millis;
unsigned long last_millis;

// Mem
extern uint8_t __data_start;
extern uint8_t __data_end;
extern uint8_t __bss_start;
extern uint8_t __bss_end;
extern uint8_t __heap_start;
extern uint8_t *    __brkval;

const uint8_t STACK_CANARY=0xAA;

void setup() {
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
  
  // Start I2C
  TinyWireM.begin();

  hr_warm();
}

void loop() {
  menu_loop();
  hr_loop();
  sleep(1);
}

//##############
//# Interrupts #
//##############
ISR(WDT_vect) {
  // WakeUp!
  sleeping++;
}

unsigned long cnt1=0;

ISR(PCINT0_vect) {
  cnt1++;
  if(digitalRead(PIN_HR))
    hr_beats++;
  digitalWrite(PIN_ALARM, digitalRead(PIN_HR));
}

//########
//# Menu #
//########
void menu_loop() {
  TWISerial.setEnabled(1);
  uint8_t n = TWISerial.available();
  
  if(TWISerial.isError()) {
    TWISerial.setEnabled(0);
    n = 0;
  }
  
  while(n > 0) {
    char cmd = TWISerial.read();
    n--;
    switch(cmd) {
      case 'h':
        menu_help();
        break;
      case 'd':
        dump_eeprom();
        break;
      case 'c':
        show_cpu_stats();
        break;
      case 'm':
        show_mem_stats();
        break;
      case 't':
        power_test();
        break;
      default:
        TWISerial.print(F("CNT1: "));
        TWISerial.println(cnt1);
        TWISerial.print(F("Unknown command: "));
        TWISerial.println(cmd);
    }
  }
  
  TWISerial.flush();
}

void menu_help() {
  TWISerial.println();
  TWISerial.println();
  TWISerial.println(F("TinySleep 1.0"));
  TWISerial.println();
  TWISerial.println(F("Commands:"));
  TWISerial.println(F(" h Show this help"));
  TWISerial.println(F(" d Dump EEPROM"));
  TWISerial.println(F(" c Show cpu stats"));
  TWISerial.println(F(" m Show mem stats"));
  TWISerial.println(F(" t Power test"));
  TWISerial.println();
  TWISerial.flush();
}

void power_test() {
  digitalWrite(PIN_HRPWR, LOW);
  
  TWISerial.println();
  
  TWISerial.println(F("1. All off"));
  TWISerial.flush();
  while(TWISerial.available()==0) {
    sleep(1);
  }
  TWISerial.read();
  
  TWISerial.println(F("2. CPU running"));
  TWISerial.flush();
  while(TWISerial.available()==0);
  TWISerial.read();
  
  digitalWrite(PIN_HRPWR, HIGH);
  TWISerial.println(F("3. HR running"));
  TWISerial.flush();
  while(TWISerial.available()==0) {
    sleep(1);
  }
  TWISerial.read();
  
  digitalWrite(PIN_HRPWR, HIGH);
  TWISerial.println(F("3. ALL running"));
  TWISerial.flush();
  while(TWISerial.available()==0);
  TWISerial.read();
  
  TWISerial.println(F("End of test."));
  TWISerial.flush();
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

void show_cpu_stats() {
  unsigned long total    = rmillis();
  unsigned long running  = millis();
  unsigned long sleeping = total - running;
  
  TWISerial.print(F("Total:    "));
  TWISerial.println(total);
  TWISerial.print(F("HR:       "));
  TWISerial.print(hr_ontime);
  TWISerial.print(F("("));
  TWISerial.print((hr_ontime*100)/total);
  TWISerial.println(F(")"));
  TWISerial.print(F("Running:  "));
  TWISerial.print(running);
  TWISerial.print(F("("));
  TWISerial.print((running*100)/total);
  TWISerial.println(F(")"));
  TWISerial.print(F("Sleeping: "));
  TWISerial.print(sleeping);
  TWISerial.print(F("("));
  TWISerial.print((sleeping*100)/total);
  TWISerial.println(F(")"));
  TWISerial.flush();
}

void show_mem_stats() {
  uint16_t total_size = RAMEND-((uint16_t)&__data_start)+1;
  uint16_t data_size  = ((uint16_t)&__data_end)-((uint16_t)&__data_start);
  uint16_t bss_size   = ((uint16_t)&__bss_end)-((uint16_t)&__bss_start);
  uint16_t heap_size  = ((uint16_t)__brkval) == 0 ? 0 : (((uint16_t)__brkval)-((uint16_t)&__heap_start));
  uint16_t stack_size = RAMEND-SP;
  uint16_t total_used = data_size+bss_size+heap_size+stack_size;
  TWISerial.print(F("Total:  "));
  TWISerial.println(total_size);
  TWISerial.print(F("Data:   "));
  TWISerial.println(data_size);
  TWISerial.print(F("BSS:    "));
  TWISerial.println(bss_size);
  TWISerial.print(F("Heap:   "));
  TWISerial.println(heap_size);
  TWISerial.print(F("Stack:  "));
  TWISerial.println(stack_size);
  TWISerial.print(F("Free:   "));
  TWISerial.println(RAMEND-total_used);
  TWISerial.print(F("Unused: "));
  TWISerial.println(mem_unused());
  TWISerial.flush();
}

//#############
//# HeartRate #
//#############
void hr_loop() {
  switch(hr_status) {
    case HR_OFF:
      // Take the HR each minute
      if(rmillis() - hr_start >= 60000) {
        hr_warm();
      }
      break;
    case HR_WARM:
      // Ignore the first 2 seconds
      if(rmillis() - hr_start >= 2000) {
        if(hr_begin_measure()!=0) {
          hr_off();
        }
      }
      break;
    case HR_MEASURE:
      // Listen beats in 5 seconds
      if(rmillis() - hr_time >= 5000) {
        if(hr_end_measure()!=0) {
          hr_off();
        }
      }
      break;
  }
}

uint8_t hr_warm() {
  TWISerial.println(F("hr_warm()"));
  
  // Enable HR Interrupt
  GIMSK |= _BV(PCIE);   // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT1); // Use PB1 as interrupt pin
  
  hr_start = rmillis();
  digitalWrite(PIN_HRPWR, HIGH);
  hr_status = HR_WARM;
  return 0;
}

uint8_t hr_begin_measure() {
  TWISerial.println(F("hr_begin_measure()"));
  // Wait Rise edge
  unsigned long t = millis() + 2000;
  while(digitalRead(PIN_HR)==1) TIMEOUT(t,1);
  while(digitalRead(PIN_HR)==0) TIMEOUT(t,1);
  // Start to count beats
  hr_time = rmillis();
  hr_beats = 0;
  hr_status = HR_MEASURE;
  return 0;
}

uint8_t hr_end_measure() {
  TWISerial.println(F("hr_end_measure()"));
  // Wait Rise edge
  unsigned long t = millis() + 2000;
  while(digitalRead(PIN_HR)==1) TIMEOUT(t,1);
  while(digitalRead(PIN_HR)==0) TIMEOUT(t,1);
  // Calc HeartRate
  hr = (((unsigned long)hr_beats)*60000)/(rmillis()-hr_time);
  
  TWISerial.print(F("HR: "));
  TWISerial.println(hr);
  TWISerial.flush();
  
  return hr_off();
}

uint8_t hr_off() {
  TWISerial.println(F("hr_off()"));
  digitalWrite(PIN_HRPWR, LOW);
  digitalWrite(PIN_ALARM, LOW);
  
  // Disable HR Interrupt
  PCMSK &= ~_BV(PCINT1); // Disable PB1 as interrupt pin
  
  hr_ontime += rmillis() - hr_start;
  hr_status = HR_OFF;
  return 0;
}

//#############
//# Low Power #
//#############

unsigned long rmillis() {
  unsigned long t = millis();
  real_millis += t - last_millis;
  last_millis = t;
  return real_millis;
}

void sleep(int nsec) {
  // Prevent false wdt reset
  wdt_reset();
  // Configure the wdt
  wdt_interrupt_enable();
  
  // Set sleeping flag
  sleeping = 1;
  
  // Set sleep to full power down.  Only external interrupts or 
  // the watchdog timer can wake the CPU!
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  unsigned long t = millis();
  while(sleeping <= nsec) {
    // Enable sleep and enter sleep mode.
    sleep_mode();
    
    // CPU is now asleep and program execution completely halts!
    // Once awake, execution will resume at this point.
    // If the flag sleeping is 1, we will go to sleep again.
  }
  real_millis += (nsec*1000) - (millis() - t); // real_millis += sleeped - awake
  
  wdt_interrupt_disable();
  
  // When awake, disable sleep mode and turn on all devices.
  sleep_disable();
}

void wdt_interrupt_enable() {
  // Configure WDT
  cli();                         // Disable Interrupts
  WDTCR |= _BV(WDCE) | _BV(WDE); // Enable the WD Change Bit
  WDTCR =  _BV(WDIE) |           // Enable WDT Interrupt
           _BV(WDP1) | _BV(WDP2);// Set Timeout to ~1 seconds
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

