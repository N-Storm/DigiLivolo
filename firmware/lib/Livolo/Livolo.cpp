/*
  Livolo.cpp - Library for Livolo wireless switches.
  Created by Sergey Chernov, October 25, 2013.
  Released into the public domain.

  01/12/2013 - code optimization, thanks Maarten! http://forum.arduino.cc/index.php?topic=153525.msg1489857#msg1489857
*/

#include "Arduino.h"
#include "Livolo.h"

volatile dl_buffer_u dl_buf = { 0 };

Livolo::Livolo(byte pin)
{
  pinMode(pin, OUTPUT);
  txPin = pin;
}

// Sequence remoteID, 16 bits followed by keycode, 7 bits
// seq = (remoteID << 7) + (keycode & 0x7F)
// keycodes #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
// real remote IDs: 6400; 19303
// tested "virtual" remote IDs: 10550; 8500; 7400
// other IDs could work too, as long as they do not exceed 16 bit
// known issue: not all 16 bit remote ID are valid
// have not tested other buttons, but as there is dimmer control, some keycodes could be strictly system
// use: sendButton(remoteID, keycode), see example blink.ino; 

void Livolo::sendButton(unsigned int remoteID, byte keycode) {

  for (pulse = 0; pulse <= 180; pulse = pulse + 1) { // how many times to transmit a command
    sendPulse(1); // Start
    high = true; // first pulse is always high

    for (i = 15; i <= 15; i--) { // transmit remoteID bits 15..0
      // byte txPulse=bitRead(remoteID, i-1); // read bits from remote ID
      byte txPulse = (remoteID >> i) & 0x01;
      selectPulse(txPulse);
    }

    for (i = 6; i <= 6; i--) { // transmit keycode bits 6..0 (!)
      // byte txPulse=bitRead(keycode, i-1); // read bits from keycode
      byte txPulse = (keycode >> i) & 0x01;
      selectPulse(txPulse);
    }
  }
  digitalWrite(txPin, LOW);
}

// build transmit sequence so that every high pulse is followed by low and vice versa
void Livolo::selectPulse(byte inBit) {
  switch (inBit) {
  case 0:
    for (byte ii = 1; ii < 3; ii++) {
      if (high == true) {   // if current pulse should be high, send High Zero
        sendPulse(2);
      }
      else {              // else send Low Zero
        sendPulse(4);
      }
      high = !high; // invert next pulse
    }
    break;
  case 1:                // if current pulse should be high, send High One
    if (high == true) {
      sendPulse(3);
    }
    else {             // else send Low One
      sendPulse(5);
    }
    high = !high; // invert next pulse
    break;
  }
}

// transmit pulses
// slightly corrected pulse length, use old (commented out) values if these not working for you
void Livolo::sendPulse(byte txPulse) {

  switch (txPulse) { // transmit pulse
  case 1: // Start
    digitalWrite(txPin, HIGH);
    delayMicroseconds(500); // 550
    // digitalWrite(txPin, LOW); 
    break;
  case 2: // "High Zero"
    digitalWrite(txPin, LOW);
    delayMicroseconds(100); // 110
    digitalWrite(txPin, HIGH);
    break;
  case 3: // "High One"
    digitalWrite(txPin, LOW);
    delayMicroseconds(300); // 303
    digitalWrite(txPin, HIGH);
    break;
  case 4: // "Low Zero"
    digitalWrite(txPin, HIGH);
    delayMicroseconds(100); // 110
    digitalWrite(txPin, LOW);
    break;
  case 5: // "Low One"
    digitalWrite(txPin, HIGH);
    delayMicroseconds(300); // 290
    digitalWrite(txPin, LOW);
    break;
  }
}

// New transmitter with Timer1
// CLK = 64MHz / 128 = 500000 Hz = 2 us period.
// 20000 Hz (64000000/((25)*128))
// 10000 Hz (100 us) (64000000/(50*128))
// 2000 Hz (500 us) (64000000/(250*128))
// 100 us (zero) = 50 ticks
// 300 us (one) = 150 ticks
// 500 us (start) = 250 ticks

void Livolo::sendButton(unsigned int remoteID, byte keycode, bool use_timer) {
  if (use_timer == true) {
    memset((void *)&dl_buf, 0, sizeof(dl_buf));
    /* if (dl_buf.buf != 0) // Still sending previous ?
      dl_buf.buf = 0;
      return; */

    // Set MSB to 1 (it's not transmitted as keycode are 7-bit, but we use it as a end-of-transmit mark)
    keycode |= (1 << 7);
        
    uint8_t tempbuf[3];
    // reversed bit order
    tempbuf[2] = __builtin_avr_insert_bits(0x70123456, keycode, 0);
    tempbuf[1] = __builtin_avr_insert_bits(0x01234567, (uint8_t)(remoteID & 0xFF), 0);
    tempbuf[0] = __builtin_avr_insert_bits(0x01234567, (uint8_t)(remoteID >> 8), 0);

    for (uint8_t z = 0; z <= 2; z++) {
      dl_buf.bytes[2] = tempbuf[2];
      dl_buf.bytes[1] = tempbuf[1];
      dl_buf.bytes[0] = tempbuf[0];

      // digitalWrite(txPin, HIGH);
      PORTB |= 1 << txPin;
      timer1_start();
      // while (dl_buf.buf > 1);
      while (dl_buf.buf != 0);
      timer1_stop();
    }

    delayMicroseconds(500);
    PORTB &= ~(1 << txPin);
    // digitalWrite(txPin, LOW);

    timer1_stop();
    PLLCSR &= ~(1 << PCKE);
    OCR1C = 0xFF;
    // Reset interrupt flags
    TIFR = (1 << OCF1A | 1 << OCF1B | 1 << TOV1);

    // Run timer in "old" mode
    TCCR1 = (1 << CTC1 | 1 << PWM1A | 1 << CS10 | 1 << CS11 | 1 << CS12);
    TIMSK = 1 << TOIE1;
  }
  else
    sendButton(remoteID, keycode);
}

/// @brief Initializes and starts Timer 1
void timer1_start() {
  cli();

  // Stop Timer 1
  // TCCR1 &= ~(1 << CS10 | 1 << CS11 | 1 << CS12 | 1 << CS13);

  // Clear registers
  TCCR1 = 0;
  GTCCR = 0;
  TCNT1 = 0;

  // Reset interrupt flags
  TIFR = (1 << OCF1B | 1 << OCF1A);

  // Enable asynchronous mode on Timer1 (clock source - PLL)
  PLLCSR |= (1 << PCKE);

  // CTC
  TCCR1 |= (1 << CTC1);

  OCR1C = OCR_500US; // 500 uS
  // interrupt COMPA
  OCR1A = OCR_500US; // 500 uS
  // Output Compare Match A Interrupt Enable
  TIMSK |= (1 << OCIE1A);
  // Prescaler 128, start timer
  TCCR1 |= (1 << CS13);
  sei();
}

/// @brief Changes time to next interrups by updating output compare registers
/// @param ocr[in] OCR value (used values are defined in OCR_ macros)
inline void timer1_update(uint8_t ocr, uint8_t ocr_aux = 0) {
  OCR1A = ocr;
  OCR1C = ocr;
  if (ocr_aux > 0) {
    OCR1B = ocr_aux;
    TIFR = 1 << OCF1B;
    TIMSK |= (1 << OCIE1B);
  }
  else {
    TIMSK &= ~(1 << OCIE1B);
    // OCR1B = 0xFF;
  }
}

/// @brief Stops Timer 1
void timer1_stop() {
  noInterrupts();

  // Stop Timer 1
  TCCR1 &= ~(1 << CS10 | 1 << CS11 | 1 << CS12 | 1 << CS13);

  // Output Compare Match A/B Interrupt Disable
  TIMSK &= ~(1 << OCIE1A | 1 << OCIE1B);

  OCR1C = 0xFF;
  OCR1A = 0xFF;
  OCR1B = 0xFF;
  
  TCNT1 = 0;

  interrupts();
}

ISR(TIMER1_COMPA_vect, ISR_NOBLOCK) {
  PINB = 1 << PINB5;

  if ((dl_buf.bytes[0] & 0x01) == 0) {
    timer1_update(OCR_200US, OCR_100US);
  }
  else {
    timer1_update(OCR_300US);
  }
  
  dl_buf.buf >>= 1;
}

ISR(TIMER1_COMPB_vect) {
  PINB = 1 << PINB5;
}
