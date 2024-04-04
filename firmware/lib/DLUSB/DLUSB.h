/*
 * Based on DigiUSB library from Digistump Arduino: https://github.com/ArminJo/DigistumpArduino
 * Based on Obdev's AVRUSB code and under the same license.
 *
 * TODO: Make a proper file header. :-)
 */
#ifndef __DLUSB_h__
#define __DLUSB_h__

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usbdrv.h"
#include "Print.h"

typedef uint8_t byte;

#include <util/delay.h>     /* for _delay_ms() */

#define RING_BUFFER_SIZE 128

struct ring_buffer {
  unsigned char buffer[RING_BUFFER_SIZE];
  int head;
  int tail;
};

class DLUSBDevice : public Print {
 private:
  ring_buffer *_rx_buffer;
  ring_buffer *_tx_buffer;

 public:
  DLUSBDevice (ring_buffer *rx_buffer, ring_buffer *tx_buffer);

  void begin();
    
  // TODO: Deprecate update
  void update();

  void refresh();
  void delay(long milliseconds);

  int available();
  int tx_remaining();
  
  int read();
  virtual size_t write(byte c);
  using Print::write;

};

extern DLUSBDevice DLUSB;

#endif // __DLUSB_h__
