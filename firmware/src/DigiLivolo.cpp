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

#include <Arduino.h>
#include <DLUSB.h>
#include <DLTransmitter.h>
#include <stdint.h>

/* Pin currently set at compile time in DLTransmitter.h. When it defined there, pin from the constructor
 * parameter are ignored. To change pin number either edit #define DL_STATIC_PIN line in DLTransmitter.h
 * as well or comment out that define and set pin from here. */
DLTransmitter dltransmitter(PIN_B5);

dlusb_packet_t in_buf, out_buf; // Input & outpus USB packet buffers

/// @brief Populates dlusb_packet_t struct with RDY packet which are sent
///        to the host from setup() on device powerup/reset to signal the
///        host software that the device are ready.
/// @param packet[out] pointer to dlusb_packet_t struct
void mk_rdy_packet(dlusb_packet_t* packet) {
  packet->report_id = REPORT_ID;
  packet->cmd_id = CMD_RDY;

  // Just some unused "magic" data to test connection below.
  packet->remote_id = 0xABCD;
  packet->btn_id = 0xEF;
}


/// @brief Wrapper for a member function DLUSB.refresh() to be called from a non-member class
void DLUSB_refresh_wrapper() {
  DLUSB.refresh();
}

void setup() {
  DLUSB.begin();
  DLUSB.refresh();

  /* Fill & send RDY packet to indicate device has been booted and ready to
   * accept commands. */
  mk_rdy_packet(&out_buf);
  DLUSB.write(&out_buf);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  DLUSB.refresh();
}

void loop() {
  DLUSB.refresh();

  // Read data from host if available.
  if (DLUSB.available()) {
    // Turn on the LED to indicate received packet.
    digitalWrite(LED_BUILTIN, HIGH);

    if (DLUSB.read(&in_buf)) {
      DLUSB.refresh();

      if (in_buf.cmd_id == CMD_SWITCH || in_buf.cmd_id == CMD_SWITCH_OLD) {
        // Transmit Livolo code
        if (in_buf.cmd_id == CMD_SWITCH) // New method
          dltransmitter.sendButton(in_buf.remote_id, in_buf.btn_id, true, &DLUSB_refresh_wrapper);
        else if (in_buf.cmd_id == CMD_SWITCH_OLD) // Old method
          dltransmitter.sendButton(in_buf.remote_id, in_buf.btn_id);

        DLUSB.refresh();

        /* Send back same packet so that the host software can acknowledge it was
         * processed by the device. */
        DLUSB.write(&in_buf);
      }
      else {
        memcpy(&out_buf, &in_buf, sizeof(in_buf));
        out_buf.cmd_id = CMD_ERR_UNKNOWN;
        DLUSB.write(&out_buf);
      }
      
      /* Sleep for 100ms after receiving packet & transmitting the keycode. We
       * don't need to send codes more often. Makes LED blink noticable. */
      DLUSB.delay(100);
    }
    digitalWrite(LED_BUILTIN, LOW); // LED off
  }

  // Sleep for 50ms between checking for packets from the host.
  DLUSB.delay(50);
}
