/*
  Morse.h - Library for Livolo wireless switches.
  Created by Sergey Chernov, October 25, 2013.
  Released into the public domain.
*/

#ifndef Livolo_h
#define Livolo_h

#include "Arduino.h"
#include <stdint.h>

// #define MOD_TIME2

#ifdef MOD_TIME
#define OCR_100US 14
#define OCR_200US 28
#define OCR_300US 42
#define OCR_500US 68
#else
#ifdef MOD_TIME2
#define OCR_100US 52
#define OCR_200US 103
#define OCR_300US 155
#define OCR_500US 255
#else
#define OCR_100US  50
#define OCR_200US 100
#define OCR_300US 150
#define OCR_500US 250
#endif
#endif

typedef union {
  uint32_t buf;
  uint8_t bytes[4];
} dl_buffer_u;

extern volatile dl_buffer_u dl_buf;

void timer1_start();
inline void timer1_update(uint8_t ocr, uint8_t ocr_aux);
void timer1_stop();

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
