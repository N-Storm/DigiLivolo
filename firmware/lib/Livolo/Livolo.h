/*
  Morse.h - Library for Livolo wireless switches.
  Created by Sergey Chernov, October 25, 2013.
  Released into the public domain.
*/

#ifndef Livolo_h
#define Livolo_h

#include "Arduino.h"
#include <stdint.h>

#define DL_TIMER_PLL 11

#if defined(__AVR_ATtinyX5__) && defined(TIMER_TO_USE_FOR_MILLIS)
  #include <UserTimer.h>
  #if defined(TIMER_TO_USE_FOR_USER) && TIMER_TO_USE_FOR_USER == 1
    #define DL_TIMER DL_TIMER_PLL
    #define OCR_HALFBIT  82
    #define OCR_FULLBIT 164
    #define OCR_START   255
  #else
    #if defined(TIMER_TO_USE_FOR_USER) && TIMER_TO_USE_FOR_USER == 0
      #define DL_TIMER 0
      #error "Timer 0 on DigiStump core are not supported yet. Please change TIMER_TO_USE_FOR_MILLIS to 0 in Digistump core to make Timer 1 available."
    #else
      #error "Unknown ATTiny85 core or unknown configuration."
    #endif
  #endif
#else
  #define DL_TIMER 1
#endif

#if DL_TIMER != DL_TIMER_PLL
  #define OCR_START_US 530ULL
  #define OCR_BIT_US 320ULL
  #define DL_TIMER_PRESCALER 64

  #define OCR_START_I ((OCR_START_US * F_CPU + (1000000 * DL_TIMER_PRESCALER / 2)) / (1000000 * DL_TIMER_PRESCALER))
  #define OCR_START OCR_START_I <= 255 ? OCR_START_I : 255
  #define OCR_FULLBIT_I ((OCR_BIT_US * F_CPU + (1000000 * DL_TIMER_PRESCALER / 2)) / (1000000 * DL_TIMER_PRESCALER))
  #define OCR_FULLBIT OCR_FULLBIT_I % 2 == 1 ? (OCR_FULLBIT_I+1) : (OCR_FULLBIT_I)
  #define OCR_HALFBIT (OCR_FULLBIT)/2
#endif

#if defined(__AVR__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
typedef union {
  __uint24 buf;
  uint8_t bytes[3];
} dl_buffer_u;
#else
typedef union {
  uint32_t buf;
  uint8_t bytes[4];
} dl_buffer_u;
#endif

void timer1_start();
void timer1_stop();
inline void timer1_update(uint8_t ocr, uint8_t ocr_aux);

class Livolo
{
public:
  Livolo(byte pin);
  void sendButton(unsigned int remoteID, byte keycode);
  void sendButton(unsigned int remoteID, byte keycode, bool use_timer);
private:
  byte txPin;
  byte i; // just a counter
  byte pulse; // counter for command repeat
  boolean high; // pulse "sign"
  void selectPulse(byte inBit);
  void sendPulse(byte txPulse);
};

#endif
