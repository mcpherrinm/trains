#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
/*

Full step sequence:
| Coils |          Signals          |
|-----------------------------------|
| A | B | EnA In1 In2 | EnB In3 In4 |
|-----------------------------------|
| + |   |  1   1   0  |  0   X   X  |
|   | + |  0   X   X  |  1   1   0  |
| - |   |  1   0   1  |  0   X   X  |
|   | - |  0   X   X  |  1   0   1  |

Waveforms:
EnA -_-_-_-_
In1 --__--__
In2 __--__--  = !In1
EnB _-_-_-_-  = !EnA
In3 --__--__  = In1
In4 __--__--  = In2
*/

/* Once I find a hardware not-gate I can halve pin count */
/* All are on port D*/
/* And these values are basically a comment for now */
static const int PIN_EnA = 2;
static const int PIN_EnB = 3;
static const int PIN_In1 = 4;
static const int PIN_In2 = 5;
static const int PIN_In3 = 6;
static const int PIN_In4 = 7;

/*
1 ^ 1 = 0
1 ^ 0 = 1
0 ^ 1 = 1
0 ^ 0 = 0
*/

ISR(TIMER0_COMPA_vect) {
  if(!(PIND & _BV(PD0))) {
    // only run when button held
    return;
  }
  static bool flippy = false;
  flippy = !flippy;
  if(flippy) {  // alternating times
    /*flip all bits*/
    PORTD ^= 0b11111100;
  } else {
    /* just flip enable bits */
    PORTD ^= 0b00001100;
  }
}

void setup(void) {
  /*bit 1 is for serial */
  DDRD = 0b11111110;
  DDRC = 0; // In particular, PD0 is unset, ie, input
  PORTC = 1; // Use built-in pull-up resistor.

  /* On: EnA, In1, In3.
     Off: Enb, In2, In4.*/
  PORTD = 0b01010100;

  /* Set up timer for stepper */
  TCCR0A = 1 << WGM01; // CTC mode
  TCCR0B = _BV(CS02) | _BV(CS00); // clock/1024
  OCR0A = 255;
  TIMSK0 = _BV(OCIE0A); //Output compare intterupt enable 0a

  /* and hang out */
  sei();
}

void loop(void) { }

int main(void) {
  setup();
  while(true){loop();}
}
