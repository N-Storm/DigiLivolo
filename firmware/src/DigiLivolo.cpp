#include <Arduino.h>
#include <DLUSB.h>
#include <livolo.h>
#include <stdint.h>

#define LIVOLO_REMOTE_ID 8525

Livolo livolo(PIN_B5); // transmitter connected to pin #8
int input_buf;

void setup() {
  DLUSB.begin();
  DLUSB.refresh();
  DLUSB.write(0xFF);
  DLUSB.refresh();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void get_input() {
  // when there are no characters to read
  while (true) {
    if(DLUSB.available()){
      //something to read
      input_buf = DLUSB.read();
      // DLUSB.write(input_buf);
      break;
    }
    // refresh the usb port
    DLUSB.refresh();
    DLUSB.delay(10);
  }
}

void loop() {
  //DLUSB.refresh();
  
  get_input();

  if (input_buf > 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    livolo.sendButton(LIVOLO_REMOTE_ID, input_buf);
    DLUSB.refresh();
    DLUSB.write(input_buf);
    DLUSB.delay(100);
    input_buf = 0;
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    DLUSB.refresh();
    DLUSB.write(0xFA);
  }

  DLUSB.delay(10);
}