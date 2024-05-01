/* Name: DLUSB.h
 * Part of the DigiLivolo firmware.
 * https://github.com/N-Storm/DigiLivolo/ 
 * Copyright (c) 2024 GitHub user N-Storm.
 * 
 * Based on DigiUSB library from Digistump Arduino: https://github.com/ArminJo/DigistumpArduino
 * Based on Obdev's AVRUSB code.
 *
 * License: GNU GPL v3 or later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef __DLUSB_h__
#define __DLUSB_h__

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdint.h>
#include "usbdrv.h"
#include "defs.h"

#include <util/delay.h> /* for _delay_ms() */

/* Buffer size for USB TX & RX buffers. This is not the bytes, it's a count of dlusb_packet_t
 * structures it holds. I.e. how many packets it can store before processing. */
#define RING_BUFFER_SIZE 16

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
