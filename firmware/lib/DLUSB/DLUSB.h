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
#include <stdint.h>
#include "usbdrv.h"

#include <util/delay.h> /* for _delay_ms() */

/* Buffer size for USB TX & RX buffers. This is not the bytes, it's a count of dlusb_packet_t
 * structures it holds. I.e. how many packets it can store before processing. */
#define RING_BUFFER_SIZE 16

#define REPORT_ID 0x4c

#define CMD_SWITCH 0x01 // IN,OUT send Livolo keycode command or send ACK to the host
#define CMD_RDY 0x10 // OUT, device ready command
#define CMD_FAIL_BIT (uint8_t)(1 << 7) // Not used

typedef struct dlusb_packet {
  uint8_t report_id;
  uint8_t cmd_id;
  uint16_t remote_id;
  uint8_t btn_id;
} dlusb_packet_t;

struct ring_buffer {
  dlusb_packet_t buffer[RING_BUFFER_SIZE];
  int head;
  int tail;
};

/// @brief Class for interfacing with USB
class DLUSBDevice {
private:
  ring_buffer* _rx_buffer;
  ring_buffer* _tx_buffer;

public:
  DLUSBDevice(ring_buffer* rx_buffer, ring_buffer* tx_buffer);

  void begin();

  void refresh();
  void delay(long milliseconds);

  int available();
  int tx_remaining();

  bool read(dlusb_packet_t* packet);
  bool write(dlusb_packet_t* packet);
};

extern DLUSBDevice DLUSB;

#endif // __DLUSB_h__
