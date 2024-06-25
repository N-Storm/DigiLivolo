/* Name: DLUSB.c
 * Part of the DigiLivolo firmware.
 * https://github.com/N-Storm/DigiLivolo/ 
 * Copyright (c) 2024 GitHub user N-Storm.
 * 
 * Based on DLUSB library from Digistump Arduino: https://github.com/ArminJo/DigistumpArduino
 * Based on V-USB Arduino Examples by Philip J. Lindsay
 * Modification for the Digispark by Erik Kettenburg, Digistump LLC
 * VID/PID changed to pair owned by Digistump LLC, code modified to use pinchange int for attiny85
 * Original notice below:
 * Based on project: hid-data, example how to use HID for data transfer
 * (Uses modified descriptor and usbFunctionSetup from it.)
 * Original author: Christian Starkjohann
 * Arduino modifications by: Philip J. Lindsay
 * Creation Date: 2008-04-11
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v3 or later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <Arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  // for sei()
#include <util/delay.h>     // for _delay_ms()
#include <avr/eeprom.h>
#include <avr/pgmspace.h>   // required by usbdrv.h
#include "usbdrv.h"
#include "oddebug.h"        // This is also an example for using debug macros

#include "defs.h"
#include "DLUSB.h"

/* Ring buffer implementation nicked from HardwareSerial.cpp
 * TODO: Don't nick it. :) */
ring_buffer rx_buffer = { { 0, 0, 0, 0 }, 0, 0 };
ring_buffer tx_buffer = { { 0, 0, 0, 0 }, 0, 0 };

/// @brief Stores packet in the ring buffer.
/// @param[in] packet stored packet struct
/// @param[out] buffer pointer to a buffer struct
/// @return true if success, false if buffer is full
inline bool store_packet(dlusb_packet_t* packet, ring_buffer* buffer)
{
  uint8_t newhead = (buffer->head + 1) % RING_BUFFER_SIZE;

  /* if we should be storing the received data into the location
   * just before the tail (meaning that the head would advance to the
   * current location of the tail), we're about to overflow the buffer
   * and so we don't write to the buffer or advance the head. */
  if (newhead != buffer->tail && newhead <= RING_BUFFER_SIZE) {
    memcpy(&buffer->buffer[buffer->head], packet, sizeof(dlusb_packet_t));
    buffer->head = newhead;
    return true;
  }
  return false;
}

DLUSBDevice::DLUSBDevice(ring_buffer* rx_buffer, ring_buffer* tx_buffer) {
  _rx_buffer = rx_buffer;
  _tx_buffer = tx_buffer;
}

/// @brief Initializes USB
void DLUSBDevice::begin() {
  cli();

  usbInit();

  usbDeviceDisconnect();
  uchar   i;
  i = 0;
  while (--i) { // fake USB disconnect for > 250 ms
    _delay_ms(10);
  }
  usbDeviceConnect();

  sei();
}

/// @brief Calls usbPoll() to process low-level USB stuff
void DLUSBDevice::refresh() {
  usbPoll();
}

/// @brief Wait a specified number of milliseconds (roughly), refreshing in the background
/// @param[in] ms delay in milliseconds
void DLUSBDevice::delay(long ms) {
  unsigned long last = millis();
  while (ms > 0) {
    unsigned long now = millis();
    ms -= now - last;
    last = now;
    refresh();
  }
}

int DLUSBDevice::available() {
  return (RING_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % RING_BUFFER_SIZE;
}

int DLUSBDevice::tx_remaining() {
  return RING_BUFFER_SIZE - (RING_BUFFER_SIZE + _tx_buffer->head - _tx_buffer->tail) % RING_BUFFER_SIZE;
}

/// @brief Returns next packet from rx_buffer
/// @param packet[out] pointer to a struct where packet will be copied to
/// @return true if success, false if there are no new packets in ring buffer available
bool DLUSBDevice::read(dlusb_packet_t* packet) {
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return false;
  }
  else {
    memcpy(packet, &_rx_buffer->buffer[_rx_buffer->tail], sizeof(dlusb_packet_t));
    _rx_buffer->tail = (_rx_buffer->tail + 1) % RING_BUFFER_SIZE;
    return true;
  }
}

/// @brief Stores packet to tx_buffer
/// @param packet[in] pointer to a struct of packet to store
/// @return true if success, false if buffer is full
bool DLUSBDevice::write(dlusb_packet_t* packet) {
  return store_packet(packet, _tx_buffer);
}

// TODO: Handle this better?
int tx_available() {
  return (RING_BUFFER_SIZE + tx_buffer.head - tx_buffer.tail) % RING_BUFFER_SIZE;
}

bool tx_read(dlusb_packet_t* packet) {
  // if the head isn't ahead of the tail, we don't have any characters
  if (tx_buffer.head == tx_buffer.tail) {
    return false;
  }
  else {
    memcpy(packet, &tx_buffer.buffer[tx_buffer.tail], sizeof(dlusb_packet_t));
    tx_buffer.tail = (tx_buffer.tail + 1) % RING_BUFFER_SIZE;
    return true;
  }
}


/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif 
  PROGMEM const uchar usbHidReportDescriptor[25] = {    /* USB report descriptor */
    0x05, 0x84,                    // USAGE_PAGE (Power Device)
    0x09, 0x6b,                    // USAGE (SwitchOn/Off)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x6b,                    //   USAGE (SwitchOn/Off)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x85, REPORT_ID,               //   REPORT_ID (76)
    0x09, 0x52,                    //   USAGE (ToggleControl)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x07,                    //   REPORT_COUNT (7)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
  };

  /* ------------------------------------------------------------------------- */

  usbMsgLen_t usbFunctionSetup(uchar data[8])
  {
    usbRequest_t* rq = (usbRequest_t*)((void*)data);

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {    // HID class request
      // Host requests USB HID REPORT. Data: HOST <- DEVICE
      if (rq->bRequest == USBRQ_HID_GET_REPORT) {  // wValue: ReportType (highbyte), ReportID (lowbyte)
        // Since we have only one report type, we can ignore the report-ID
        static dlusb_packet_t packet[1];  // Buffer must stay valid when usbFunctionSetup returns
        if (tx_available()) {
          if (tx_read(&packet[0])) {
            usbMsgPtr = (unsigned char*)packet; // Tell the driver which data to return
            return sizeof(dlusb_packet_t); // Tell the driver to send packet
          }
          else
            return 0;
        }
        else {
          // Drop through to return 0 (which will stall the request?)
        }
      }
      // Host sets USB HID REPORT. Data: HOST -> DEVICE
      else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
        // Since we have only one report type, we can ignore the report-ID
        return USB_NO_MSG;  // use usbFunctionWrite() to receive data from host
      }
    }
    else {
      // Ignore vendor type requests, we don't use any
    }
    return 0;
  }

  // Called when hosts sends data to device, i.e. device receives HID report
  uchar  usbFunctionWrite(uchar* data, uchar len)
  {
    // Type cast incoming data to dlusb_packet_t struct
    dlusb_packet_t* p = (dlusb_packet_t*)data;
    if (p->report_id == REPORT_ID && (p->cmd_id == CMD_SWITCH || p->cmd_id == CMD_SWITCH_OLD) )
      if (!store_packet(p, &rx_buffer))
        return 0xff; // Return FAIL code

    return 1;
  }

#ifdef __cplusplus
} // extern "C"
#endif

DLUSBDevice DLUSB = DLUSBDevice(&rx_buffer, &tx_buffer);
