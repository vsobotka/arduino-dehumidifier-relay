#include <avr/io.h>
#include <avr/interrupt.h>

volatile int seconds = 0;

void setup() {
    Serial.begin(9600);
    // initialize Timer1
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
 
    /* set compare match register to desired timer count:
     *  (# timer counts + 1) = (target time) / (timer resolution)
     *  (# timer counts + 1) = (1s) / (16 MHz = 6.4e-5)
     *  (# timer counts + 1) = 15624
     */
    OCR1A = 15624;
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler:
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    sei();          // enable global interrupts
}

void loop() {

}

ISR(TIMER1_COMPA_vect)
{
    seconds++;
    Serial.print("Timer interrupt: ");
    Serial.println(seconds);
    if (seconds == 10)
    {
        seconds = 0;
    }
}
