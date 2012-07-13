#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
/*

Full step sequence:
| Coils |          Signals          |   Waveforms:
|-----------------------------------|   EnA -_-_-_-_
| A | B | EnA In1 In2 | EnB In3 In4 |   In1 --__--__
|-----------------------------------|   In2 __--__--  = !In1
| + |   |  1   1   0  |  0   X   X  |   EnB _-_-_-_-  = !EnA
|   | + |  0   X   X  |  1   1   0  |   In3 --__--__  = In1
| - |   |  1   0   1  |  0   X   X  |   In4 __--__--  = In2   
|   | - |  0   X   X  |  1   0   1  |

Now, reverse sequence:
| Coils |          Signals          |  Waveforms with don't cares chosen:
|-----------------------------------|  EnA _-_-_-_-
| A | B | EnA In1 In2 | EnB In3 In4 |  In1 --__--__
|-----------------------------------|  In2 __--__--
|   | + |  0   X   X  |  1   1   0  |  EnB -_-_-_-_
| + |   |  1   1   0  |  0   X   X  |  In3 --__--__
|   | - |  0   X   X  |  1   0   1  |  In4 __--__--
| - |   |  1   0   1  |  0   X   X  |

Observe carefully: The In* pins have the same form, but we just have to invert
the enable bits.  Very convenient.
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

ISR(TIMER0_COMPA_vect) {
  static bool running = true;
  if(!(PINC & _BV(PC0))) {
    // only run when button held
    // Danger: This leaves coils on.
    // it might be better to just put a switch on both enable lines
    return;
  }

  static bool reversing = false;

  if(PINC & _BV(PC1)) {
    if(!reversing) {
      reversing = !reversing;
      PORTD ^= 0b00001100;
    }
  } else {
    if(reversing) {
      reversing = !reversing;
      PORTD ^= 0b00001100;
    }
  }

  /* Enable toggles twice as fast as others */
  static bool flippy = false;
  flippy = !flippy;
  if(flippy) { 
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
  DDRC = 0;        /* Use port C for input */
  PORTC = 0xFF;  /* And enable pull-up resistors on them */

  /* On: EnA, In1, In3.
     Off: Enb, In2, In4.*/
  PORTD = 0b01010100;
 
  if(PINC & _BV(PC1)) {
    PORTD ^= 0b00001100;
  }

  /* Set up timer for stepper */
  TCCR0A = 1 << WGM01; // CTC mode
  TCCR0B = _BV(CS02) | _BV(CS00); // clock/1024
  OCR0A = 255;
  TIMSK0 = _BV(OCIE0A); //Output compare intterupt enable 0a

  /* Enable interrupts */
  sei();
}

void loop(void) { }

int main(void) {
  setup();
  while(true){loop();}
}
