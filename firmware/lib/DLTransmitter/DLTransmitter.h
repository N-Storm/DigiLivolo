#ifndef __DLTRansmitter_h__
#define __DLTRansmitter_h__

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "Livolo.h"

class DLTransmitter : public Livolo
{
public:
  DLTransmitter(uint8_t pin);
  void sendButton(uint16_t remoteID, uint8_t keycode, bool use_timer, void (*idleCallback_ptr)(void) = NULL);
  using Livolo::sendButton;
private:
  uint8_t txPin;
  void timer1_start();
  void timer1_stop();
};

#define DL_TIMER_PLL 11

#if defined(__AVR_ATtinyX5__) && defined(TIMER_TO_USE_FOR_MILLIS)
  #ifdef __cplusplus
    extern "C" {
  #endif 
  #include <UserTimer.h>
  #ifdef __cplusplus
    } // extern "C"
  #endif
  #if defined(TIMER_TO_USE_FOR_USER) && TIMER_TO_USE_FOR_USER == 1
    #define DL_NATIVE_CORE
    #define DL_TIMER DL_TIMER_PLL
    #define OCR_HALFBIT  82
    #define OCR_FULLBIT 164
    #define OCR_START   255
  #else
    #if defined(TIMER_TO_USE_FOR_USER) && TIMER_TO_USE_FOR_USER == 0
      #define DL_TIMER 0
      #error "Timer 0 on DigiStump core are not supported yet. Please change TIMER_TO_USE_FOR_MILLIS to 0 in Digistump core to make Timer 1 available."
    #else
      #error "Unsupported/unknnown ATTiny85 core or unknown configuration."
    #endif
  #endif
#else
  #define DL_TIMER 1
  #warn "Not ATTiny85 or unsupported core. Using Timer1, may break things if it's used by this core."
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

inline void timer1_update(uint8_t ocr, uint8_t ocr_aux);

#endif // __DLTRansmitter_h__