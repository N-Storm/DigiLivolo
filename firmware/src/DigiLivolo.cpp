#include <Arduino.h>
#include <DLUSB.h>
#include <Livolo.h>
#include <stdint.h>

Livolo livolo(PIN_B5); // Transmitter connected to pin #5
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
  // Read data from host if available.
  if (DLUSB.available()) {
    // Turn on the LED to indicate received packet.
    digitalWrite(LED_BUILTIN, HIGH);

    if (DLUSB.read(&in_buf)) {
      DLUSB.refresh();
      // Transmit Livolo code
      livolo.sendButton(in_buf.remote_id, in_buf.btn_id);
      DLUSB.refresh();
      /* Send back same packet so that the host software can acknowledge it was
       * processed by the device. */
      DLUSB.write(&in_buf);
      /* Sleep for 100ms after receiving packet & transmitting the keycode. We
       * don't need to send codes more often. Makes LED blink noticable. */
      DLUSB.delay(100);
      digitalWrite(LED_BUILTIN, LOW); // LED off
    }
  }

  // Sleep for 50ms between checking for packets from the host.
  DLUSB.delay(50);
}
