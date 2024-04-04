#include <Arduino.h>
#include <DLUSB.h>
#include <livolo.h>
#include <stdint.h>

#define LIVOLO_REMOTE_ID 8525

Livolo livolo(PIN_B5); // transmitter connected to pin #8
dlusb_packet_t in_buf, out_buf;

void prep_rdy_packet(dlusb_packet_t* packet) {
  packet->report_id = REPORT_ID;
  packet->cmd_id = CMD_RDY;
  packet->remote_id = 0;
  packet->btn_id = 0;
}

void setup() {
  DLUSB.begin();
  DLUSB.refresh();
  prep_rdy_packet(&out_buf);
  DLUSB.write(&out_buf);  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  DLUSB.refresh();
}

void loop() {
  // Read data from host if available
  if (DLUSB.available()) {
    if (DLUSB.read(&in_buf)) {
      digitalWrite(LED_BUILTIN, HIGH);
      DLUSB.refresh();
      livolo.sendButton(in_buf.remote_id, in_buf.btn_id);
      DLUSB.refresh();
      DLUSB.write(&in_buf);
      DLUSB.delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  DLUSB.delay(50);
}