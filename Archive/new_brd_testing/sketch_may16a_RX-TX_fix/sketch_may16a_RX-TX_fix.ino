#include <SoftwareSerial.h>

#define TMC2209_RX 5   // Arduino Nano D5
#define TMC2209_TX 6   // Arduino Nano D6

SoftwareSerial TMCSerial(TMC2209_RX, TMC2209_TX); // RX, TX

void setup() {
  Serial.begin(115200);        // Arduino's main serial (for your PC/IDE)
  TMCSerial.begin(115200);     // Set baudrate for TMC2209 as needed

  Serial.println("TMC2209 test starting...");
}

void loop() {
  // Example: send data to TMC2209
  // TMCSerial.write(0x00); // replace with actual command as needed

  // Example: read data from TMC2209 and send to PC
  if (TMCSerial.available()) {
    int inByte = TMCSerial.read();
    Serial.print("From TMC2209: ");
    Serial.println(inByte, HEX);
  }

  delay(100);
}