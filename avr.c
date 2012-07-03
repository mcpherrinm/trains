#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>

// Output is on PD3


// Because tracks can be reversed, the code on the other end needs to cope with
// either direction.  That means looking at pin changes, not values.
ISR(TIMER1_COMPA_vect) {
  static char outbits[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
                           0,    1, 0, 1, 0, 1, 0, 1, 0,
                           0,    1, 0, 1, 0, 1, 0, 1, 0,
                           0,    0, 0, 0, 0, 0, 0, 0, 0, 1};
  static int idx = 0;
  if(outbits[idx]) {
    PORTD |= (1 << 3);
    PORTD &= ~(1 << 2);
  } else {
    PORTD &= ~(1 << 3);
    PORTD |= (1 << 2);
  }
  idx = (idx + 1 ) % sizeof(outbits);
}

int len = 0;
char packet[8];

#define ERROR synced = false; syncing = 0; return;
// Input is on PB0/PCINT0
ISR(PCINT0_vect) {
  int time = TCNT0;
  TCNT0 = 0;

  static bool gothalf = 0;
  static int periodA;

  static bool synced = false;
  static int syncing = 0;
  if(!synced) {
    if(time > 52 && time < 64) {
      syncing++;
      return;
    } else if(syncing > 16 && time > 95 ) {
      synced = true;
      // Put us halfway through data start
      gothalf = true;
      periodA = time;
    } else {
      ERROR
    }
  }

  if(!gothalf) {
    periodA = time;
    gothalf = true;
    return;
  }
  gothalf = false;

  bool bit;
  /* Combo of periodA and time (period B) encodes a bit. */
  if(periodA >= 52 && periodA <= 64 && abs(periodA - time) <= 6) {
    bit = true;
  } else if(periodA >= 95 && time >= 95) {
    bit = false;
  } else {
    ERROR
  }

  static char bits = 0;

  if(!bits) {
    if(bit) {
      // Packet End Bit!
      /* Do something. Reset things. */
      /* check that shit with errors */
      return;
    } else {
      // Expect another data byte
      packet[len++] = 0;
      bits++;
      return;
    }
  }

  // If it's a 1-bit, set it.
  if(bit) {
    packet[len] |= (1 << (8 - bits));
  }

  if(++bits > 8) {
    // Got a whole byte.
    bits = 0;
    len++;
  }
}


int main(void) {
  DDRD    = (1 << 3) | (1 << 2);
  DDRC    = 1;
  PORTC   = 1;

  // Timer0: Counting up?

  // Timer1: 58us, as per '1' time of NMRA DCC
  OCR1A   = 928;
  TCCR1A  = 0;
  TCCR1B  = (1 << CS10); // no clock scale
  TCCR1B |= (1 << WGM12); // CTC mode, TOP = OCR1A
  TIMSK1 |= (1 << OCIE1A);

  // Pin change interrupt:
  PCICR   = _BV(PCIE0);
  PCMSK0  = _BV(PCINT0);

  sei();
  for(;;) { }
  return 0;

}
