#include "DigiUSB.h"
#include <livolo.h>
#include <stdint.h>

#define LIVOLO_REMOTE_ID 8525

Livolo livolo(PIN_B5); // transmitter connected to pin #8
int input_buf;

void setup() {
  DigiUSB.begin();
  DigiUSB.refresh();
  DigiUSB.write(0xFF);
  DigiUSB.refresh();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void get_input() {
  // when there are no characters to read
  while (true) {
    if(DigiUSB.available()){
      //something to read
      input_buf = DigiUSB.read();
      // DigiUSB.write(input_buf);
      break;
    }
    // refresh the usb port
    DigiUSB.refresh();
    DigiUSB.delay(10);
  }
}


void loop() {
 //wait for response

  //DigiUSB.refresh();
  
  get_input();

  if (input_buf > 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    livolo.sendButton(LIVOLO_REMOTE_ID, input_buf);
    DigiUSB.refresh();
    DigiUSB.write(input_buf);
    DigiUSB.delay(100);
    input_buf = 0;
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    DigiUSB.refresh();
    DigiUSB.write(0xFA);
  }

  DigiUSB.delay(10);
}