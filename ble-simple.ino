#include "BLESerial.h"

BLESerial bleSerial; //Bluetooth - create peripheral instance

void setup() {
  bleSerial.setLocalName("E73-2G4M04S"); /* set custom name */
  bleSerial.begin();                     /* Start Bluetooth UART */
}

static uint32_t rnd;

void loop() {     
  bleSerial.poll();
  delay(1500);

  bleSerial.print("E73 #");
  bleSerial.print(rnd++);
  bleSerial.print(" = 0x");
  bleSerial.print(random(0x1000, 0xFFFF), HEX);
}
