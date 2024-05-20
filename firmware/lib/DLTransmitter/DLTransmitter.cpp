/* Part of the DigiLivolo firmware.
 * https://github.com/N-Storm/DigiLivolo/ 
 * Copyright (c) 2024 GitHub user N-Storm.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "DLTransmitter.h"

volatile dl_buffer_u dl_buf = { 0 };
uint8_t *txPin_p;

#ifndef __AVR_ATtinyX5__
  static bool state_buf = false;
#endif

DLTransmitter::DLTransmitter(uint8_t pin) : Livolo(pin)
{
  pinMode(pin, OUTPUT);
  txPin = pin;
  txPin_p = &txPin;
}

// Sequence remoteID, 16 bits followed by keycode, 7 bits
// seq = (remoteID << 7) + (keycode & 0x7F)

void DLTransmitter::sendButton(uint16_t remoteID, uint8_t keycode, bool use_timer, void (*idleCallback_ptr)(void)) {
  if (use_timer == true) {
    memset((void *)&dl_buf, 0, sizeof(dl_buf));

    // Set MSB to 1 (it's not transmitted as keycode are 7-bit, but we use it as a end-of-transmit mark)
    keycode |= (1 << 7);
        
    uint8_t tempbuf[3];
    // reversed bit order
    tempbuf[2] = __builtin_avr_insert_bits(0x70123456, keycode, 0);
    tempbuf[1] = __builtin_avr_insert_bits(0x01234567, (uint8_t)(remoteID & 0xFF), 0);
    tempbuf[0] = __builtin_avr_insert_bits(0x01234567, (uint8_t)(remoteID >> 8), 0);

    // Save timer settings if not using native core
    #ifndef DL_NATIVE_CORE
      uint8_t tccr1_saved, gtccr_saved, tifr_saved, ocr1a_saved, ocr1c_saved;
      tccr1_saved = TCCR1;
      gtccr_saved = GTCCR;
      tifr_saved = TIFR;
      ocr1a_saved = OCR1A;
      ocr1c_saved = OCR1C;
    #endif

    for (uint8_t z = 0; z <= 128; z++) {
      dl_buf.bytes[2] = tempbuf[2];
      dl_buf.bytes[1] = tempbuf[1];
      dl_buf.bytes[0] = tempbuf[0];

      memcpy((void *)dl_buf.bytes, tempbuf, 3);

      #ifdef __AVR_ATtinyX5__
        PORTB |= 1 << txPin;
      #else
        digitalWrite(txPin, HIGH);
      #endif

      timer1_start();
      // while (dl_buf.buf > 1);
      // TODO: Make non-blocking function
      while (dl_buf.buf != 0)
        if (idleCallback_ptr != NULL)
          idleCallback_ptr();

      timer1_stop();
    }

    #ifdef __AVR_ATtinyX5__
      PORTB &= ~(1 << txPin);
    #else
      digitalWrite(txPin, LOW);
    #endif

    timer1_stop();
    #if DL_TIMER == DL_TIMER_PLL
      PLLCSR &= ~(1 << PCKE);
    #endif
    #ifndef DL_NATIVE_CORE      
    #endif

    OCR1C = 0xFF;

    // Reset interrupt flags
    TIFR = (1 << OCF1A | 1 << OCF1B | 1 << TOV1);

    // Restore timer-related registers content
    #if not defined(DL_NATIVE_CORE) && DL_TIMER == 1
      TCCR1 = tccr1_saved;
      GTCCR = gtccr_saved;
      TIFR = tifr_saved;
      OCR1A = ocr1a_saved;
      OCR1C = ocr1c_saved;      
    #endif
  }
  else
    Livolo::sendButton(remoteID, keycode);
}

/// @brief Initializes and starts Timer 1
void DLTransmitter::timer1_start() {
  cli();

  // Stop Timer 1
  // TCCR1 &= ~(1 << CS10 | 1 << CS11 | 1 << CS12 | 1 << CS13);

  // Clear registers
  TCCR1 = 0;
  GTCCR = 0;
  TCNT1 = 0;

  // Reset interrupt flags
  TIFR = (1 << OCF1B | 1 << OCF1A);

  #if DL_TIMER == DL_TIMER_PLL
    // Enable asynchronous mode on Timer1 (clock source - PLL)
    PLLCSR |= (1 << PCKE);
  #endif

  // CTC
  TCCR1 |= (1 << CTC1);

  OCR1C = OCR_START; // 500 uS
  // interrupt COMPA
  OCR1A = OCR_START; // 500 uS
  // Output Compare Match A Interrupt Enable
  TIMSK |= (1 << OCIE1A);
  // Prescaler 128, start timer
  TCCR1 |= (1 << CS13);
  sei();
}

/// @brief Stops Timer 1
void DLTransmitter::timer1_stop() {
  cli();

  // Stop Timer 1
  TCCR1 &= ~(1 << CS10 | 1 << CS11 | 1 << CS12 | 1 << CS13);

  // Output Compare Match A/B Interrupt Disable
  TIMSK &= ~(1 << OCIE1A | 1 << OCIE1B);

  OCR1C = 0xFF;
  OCR1A = 0xFF;
  OCR1B = 0xFF;
  
  TCNT1 = 0;

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

ISR(TIMER1_COMPA_vect) {
  #ifdef __AVR_ATtinyX5__
    PINB = 1 << *txPin_p;
  #else
    state_buf = !state_buf;
    digitalWrite(*txPin_p, state_buf ? HIGH : LOW);
  #endif

  sei();

  // TODO: no need to update OCR1A, OCR1C with a FULLBIT value
  if ((dl_buf.bytes[0] & 0x01) == 0) {
    timer1_update(OCR_FULLBIT, OCR_HALFBIT);
  }
  else {
    timer1_update(OCR_FULLBIT);
  }

  dl_buf.buf >>= 1;
}

ISR(TIMER1_COMPB_vect) {
  #ifdef __AVR_ATtinyX5__
    PINB = 1 << *txPin_p;
  #else
    state_buf = !state_buf;
    digitalWrite(*txPin_p, state_buf ? HIGH : LOW);
  #endif
}
