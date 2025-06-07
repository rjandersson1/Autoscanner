#define TMC2209_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#include <TMCStepper.h>

constexpr uint8_t RX = 5, TX = 6;
SoftwareSerial SW(RX, TX);

void setup() {
  Serial.begin(115200);
  SW.begin(57600);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  for (uint8_t a = 0; a < 4; a++) {
    TMC2209Stepper drv(&SW, 0.11f, a);
    drv.begin();
    drv.toff(4);                    // wake chopper

    Serial.print("addr ");
    Serial.print(a);
    Serial.println(drv.test_connection());
  }
}
void loop() {}