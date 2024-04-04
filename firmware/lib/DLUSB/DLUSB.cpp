/* Name: DLUSB.c
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
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 692 2008-11-07 15:07:40Z cs $
 */

#include <Arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

#include "DLUSB.h"

 // Ring buffer implementation nicked from HardwareSerial.cpp
 // TODO: Don't nick it. :)
ring_buffer rx_buffer = { { 0 }, 0, 0 };
ring_buffer tx_buffer = { { 0 }, 0, 0 };

inline uint8_t store_packet(dlusb_packet_t* packet_ptr, ring_buffer* the_buffer)
{
  uint8_t i = (the_buffer->head + 1) % RING_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != the_buffer->tail) {
    memcpy(&(the_buffer->buffer[the_buffer->head]), packet_ptr, sizeof(dlusb_packet_t));
    // the_buffer->buffer[the_buffer->head] = packet;
    the_buffer->head = i;
    return 1;
  }
  return 0;
}

DLUSBDevice::DLUSBDevice(ring_buffer* rx_buffer,
  ring_buffer* tx_buffer) {
  _rx_buffer = rx_buffer;
  _tx_buffer = tx_buffer;
}

void DLUSBDevice::begin() {
  cli();

  usbInit();

  usbDeviceDisconnect();
  uchar   i;
  i = 0;
  while (--i) {             /* fake USB disconnect for > 250 ms */
    _delay_ms(10);
  }
  usbDeviceConnect();

  sei();
}

void DLUSBDevice::refresh() {
  usbPoll();
}

// wait a specified number of milliseconds (roughly), refreshing in the background
void DLUSBDevice::delay(long milli) {
  unsigned long last = millis();
  while (milli > 0) {
    unsigned long now = millis();
    milli -= now - last;
    last = now;
    refresh();
  }
}

int DLUSBDevice::available() {
  /*
   */
  return (RING_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % RING_BUFFER_SIZE;
}

int DLUSBDevice::tx_remaining() {
  return RING_BUFFER_SIZE - (RING_BUFFER_SIZE + _tx_buffer->head - _tx_buffer->tail) % RING_BUFFER_SIZE;
}

dlusb_packet_t* DLUSBDevice::read() {
  /*
   */
   // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return NULL;
  }
  else {
    dlusb_packet_t* packet_ptr = &(_rx_buffer->buffer[_rx_buffer->tail]);
    _rx_buffer->tail = (_rx_buffer->tail + 1) % RING_BUFFER_SIZE;
    return packet_ptr;
  }
}

size_t DLUSBDevice::write(dlusb_packet_t* packet_ptr) {
  return 1;
  // return store_packet(packet_ptr, _tx_buffer);
}


// TODO: Handle this better?
int tx_available() {
  /*
   */
  return (RING_BUFFER_SIZE + tx_buffer.head - tx_buffer.tail) % RING_BUFFER_SIZE;
}

dlusb_packet_t* tx_read() {
  /*
   */
   // if the head isn't ahead of the tail, we don't have any characters
  if (tx_buffer.head == tx_buffer.tail) {
    return NULL;
  }
  else {
    dlusb_packet_t* packet_ptr = &(tx_buffer.buffer[tx_buffer.tail]);
    tx_buffer.tail = (tx_buffer.tail + 1) % RING_BUFFER_SIZE;
    return packet_ptr;
  }
}




/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif 
  PROGMEM const uchar usbHidReportDescriptor[38] = {    /* USB report descriptor */
    0x05, 0x84,                    // USAGE_PAGE (Power Device)
    0x09, 0x6b,                    // USAGE (SwitchOn/Off)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x6b,                    //   USAGE (SwitchOn/Off)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x85, 0x4c,                    //   REPORT_ID (76)
    0x09, 0x52,                    //   USAGE (ToggleControl)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
  };

  /* ------------------------------------------------------------------------- */

  usbMsgLen_t usbFunctionSetup(uchar data[8])
  {
    usbRequest_t* rq = (usbRequest_t*)((void*)data);

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {    /* HID class request */
      if (rq->bRequest == USBRQ_HID_GET_REPORT) {  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
        /* since we have only one report type, we can ignore the report-ID */
        static dlusb_packet_t* packet_ptr;  /* buffer must stay valid when usbFunctionSetup returns */
        if (tx_available()) {
          packet_ptr = tx_read();
          if (packet_ptr) {
            usbMsgPtr = (unsigned char*)packet_ptr; /* tell the driver which data to return */
            return sizeof(dlusb_packet_t); /* tell the driver to send 1 byte */
          }
          else
            return 0;
        }
        else {
          // Drop through to return 0 (which will stall the request?)
        }
      }
      else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
        /* since we have only one report type, we can ignore the report-ID */
        return USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */

        // TODO: Check race issues?
    //	  store_packet(rq->wValue.bytes[0] * 8, &rx_buffer);
    //	  store_packet(rq->wValue.bytes[0], &rx_buffer);
    //	  store_packet(rq->wIndex.bytes[0], &rx_buffer);
    //	  store_packet(rq->wIndex.bytes[1], &rx_buffer);
    //        store_packet(0xAA, &rx_buffer);

      }
    }
    else {
      /* ignore vendor type requests, we don't use any */
    }
    return 0;
  }

  uchar   usbFunctionWrite(uchar* data, uchar len)
  {
    /*
        uchar usbReportId = data[0];
        if (usbReportId == 1 && len == 2)
          store_packet((uchar)data[1], &rx_buffer);
        else if (usbReportId == 2 && len == 4) {
          store_packet((uchar)data[2], &rx_buffer);
          store_packet((uchar)data[1], &rx_buffer);
          store_packet((uchar)data[3], &rx_buffer);
        }
    */
    //    if(len == 2)
    //        store_packet((uchar)data[1], &rx_buffer);

    return 1;
  }

#ifdef __cplusplus
} // extern "C"
#endif

DLUSBDevice DLUSB = DLUSBDevice(&rx_buffer, &tx_buffer);


