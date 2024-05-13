/*
  Morse.h - Library for Livolo wireless switches.
  Created by Sergey Chernov, October 25, 2013.
  Released into the public domain.
*/

#ifndef Livolo_h
#define Livolo_h

#include "Arduino.h"
#include <stdint.h>
#include <UserTimer.h>

#if defined(TIMER_TO_USE_FOR_USER) && TIMER_TO_USE_FOR_USER == 1
#define DL_TIMER 1
#define OCR_HALFBIT  82
#define OCR_FULLBIT 164
#define OCR_START   255
#else
#define DL_TIMER 0
#define OCR_HALFBIT  42
#define OCR_FULLBIT  84
#define OCR_START   137
#endif

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
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
