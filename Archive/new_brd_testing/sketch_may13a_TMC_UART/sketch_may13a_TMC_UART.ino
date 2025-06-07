#include <TMCStepper.h>

// Define pins
#define EN_PIN     2
#define DIR_PIN    3
#define STEP_PIN   4
#define SERIAL_PORT Serial

// TMC2209 Settings
#define R_SENSE    0.11f   // Sense resistor in Ohms
#define DRIVER_ADDRESS 0b00  // TMC2209 default address (MS1/MS2 = GND)

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

void setup() {
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW);  // Enable driver (LOW = enabled)

    SERIAL_PORT.begin(115200);  // UART to TMC2209
    Serial.begin(9600);         // USB serial to PC for debug

    delay(500);

    driver.begin();             // Start TMC driver
    driver.toff(4);             // Enable driver
    driver.blank_time(24);
    driver.rms_current(600);    // Set motor RMS current (mA)
    driver.microsteps(16);      // Set microstepping
    driver.pwm_autoscale(true); // Needed for stealthChop

    Serial.println("TMC2209 UART test initialized.");
}

void loop() {
    static uint32_t lastTime = 0;
    if (millis() - lastTime > 1000) {
        lastTime = millis();
        Serial.print("IFCNT: ");
        Serial.println(driver.IFCNT());
    }
}