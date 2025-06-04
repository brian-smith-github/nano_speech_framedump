// Calculations for timer 1 with period 2000ms
// 2000ms corresponds to a CTC frequency of 0.5 Hz
// For prescaler 1024, OCR1A = 16000000/(2*1024*0.5) - 1 = 15624


#include <Arduino.h>
#include <SPI.h>

const int AUDIO_IN = A7;

bool led_state = false;
int prev,x,y;

//-----------------------------------------------------------------------------------
void setup() {
  Serial.begin(230400);
  pinMode(LED_BUILTIN, OUTPUT);  // Set pin 12 (LED) as output
  
  // set up timer0 interrupt at 6.4KHz....
  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 38; // = (16*10^6) / (6400*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);


  pinMode(AUDIO_IN, INPUT);
  //analogReference(EXTERNAL);
  analogRead(AUDIO_IN); // initialise ADC to read audio input
  
  sei();  // Enable back the interrupts  
}

//-------------------------------------------------------------------------------------
void loop() {
  // Do nothing in the loop function
}


//-------------------------------------------------------------------------------------
// The Interrupt-service routine (ISR).
// With the settings above, this ISR will trigger 6400 times/sec...
ISR(TIMER0_COMPA_vect) {
  TCNT0 = 0;  // Reset the counter
  // read the ADC and dump level to serial...
  int i = ADCL;
  i += ADCH << 8;
  x=i-prev; prev=i;  y=x;
  //bitSet(ADCSRA, ADIF); // clear the flag
  Serial.write(y);
  
  bitSet(ADCSRA, ADSC); // start next ADC conversion
  
  
}
