#include <SoftwareSerial.h>

void setup() {
  ModemInit();

}

void loop() {
  ModemSend( 1, "Test From Arduino");

}
