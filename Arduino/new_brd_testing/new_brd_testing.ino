// ===================================================================
// new_brd_testing.ino
// Bring-up / bench test sketch for the finalized Mainboard.
// Not the real firmware -- just exercises each I/O one at a time.
//
// Pin map, per user-confirmed final schematic (Arduino Nano sheet):
//   D2  EN        (to TMC2209 EN, active LOW)
//   D3  SW        (pushbutton to GND, needs INPUT_PULLUP)
//   D4  LED_IR    (IR emitter, also usable as a plain digital output)
//   D5  SOL_PWM   (solenoid driver base drive)
//   D6  RX        (TMC2209 UART RX leg, direct wire)
//   D7  TX        (TMC2209 UART TX leg, through 1k R12)
//   A0  POTI
//
// ASSUMPTION -- please confirm: this schematic does not show STEP/DIR
// wired to any Nano pin (D8-D13 are unconnected in the sheet you sent).
// Mode 3 below therefore drives the motor purely over UART using the
// TMC2209's VACTUAL velocity register, not step/dir pulses. If STEP/DIR
// actually exist on pins I haven't seen, mode 3 needs to be rewritten
// around AccelStepper + step pulses instead -- tell me the pins and
// I'll redo it.
// ===================================================================

#include <SoftwareSerial.h>

#define PIN_EN       2
#define PIN_SW       3
#define PIN_LED_IR   4
#define PIN_SOL_PWM  5
#define PIN_TMC_RX   6   // direct leg
#define PIN_TMC_TX   7   // 1k leg
#define PIN_POTI     A0

#define IR_SEND_PIN  PIN_LED_IR   // must be defined before IRremote.hpp is included
#include <IRremote.hpp>

#include <TMCStepper.h>
#define R_SENSE         0.11f
#define DRIVER_ADDRESS  0b00      // MS1/MS2 = GND

// ---- select which test runs ----
#define TEST_MODE 4   // 1 = I/O test, 2 = IR/shutter test, 3 = motor test, 4 = solenoid test

// ---- globals (instantiated regardless of mode, cheap) ----
SoftwareSerial tmcSerial(PIN_TMC_RX, PIN_TMC_TX);   // RX, TX
TMC2209Stepper driver(&tmcSerial, R_SENSE, DRIVER_ADDRESS);
IRsend irLED;   // pin comes from the IR_SEND_PIN macro defined above, this IRremote version takes no constructor arg

bool ledState = false;
bool lastSwState = HIGH;   // INPUT_PULLUP idle = HIGH
unsigned long shutterCount = 0;

void fireShutter() {
  digitalWrite(PIN_LED_IR, HIGH);
  for (int i = 0; i < 3; i++) {
    irLED.sendSony(0xB4B8F, 20);   // same command FilmScanner::takePhoto() uses
    delay(40);
  }
  digitalWrite(PIN_LED_IR, LOW);
  shutterCount++;
  Serial.print("shutter fired, count=");
  Serial.println(shutterCount);
}

void initStepperUART() {
  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, LOW);   // enable, active low
  tmcSerial.begin(115200);
  driver.begin();
  driver.blank_time(24);
  driver.rms_current(500);     // motor is rated 500mA per Notes -- do not raise without checking
  driver.microsteps(0);
  driver.pwm_autoscale(true);
  driver.ihold(8);             // explicit standstill current scale (0-31), don't rely on rms_current()'s default hold multiplier
  driver.irun(16);             // explicit run current scale
  driver.iholddelay(10);
  driver.freewheel(0);         // 0 = apply IHOLD at standstill, 1 = coast/no current
  driver.toff(4);              // MUST be last CHOPCONF-touching call -- microsteps()/others were clobbering this back to 0,
                                // and TOFF=0 is the driver's actual output-stage-disable state, which is what was happening
  driver.VACTUAL(0);

  int result = driver.test_connection();
  if (result == 0) {
    Serial.println("TMC2209 UART: success");
  } else if (result == 2) {
    Serial.println("TMC2209 UART: driver VM = 0V (check +20V rail)");
  } else {
    Serial.println("TMC2209 UART: FAILED -- check RX/TX wiring, R12, MS1/MS2");
  }

  // test_connection() just checks whether IOIN() read back exactly 0x0 and
  // assumes that means VM=0V -- but IOIN also reflects STEP/DIR/MS1/MS2 pin
  // states, and on this board those are grounded/floating-low, so an all-zero
  // IOIN can happen with VM fully present. GSTAT's uv_cp bit is the actual
  // charge-pump-undervoltage flag and is a real VM check, unlike the heuristic
  // above -- read both raw so you can tell which one it actually is.
  uint32_t ioin = driver.IOIN();
  uint8_t gstat = driver.GSTAT();
  Serial.print("IOIN=0x");
  Serial.println(ioin, HEX);
  Serial.print("GSTAT=0b");
  Serial.println(gstat, BIN);
  Serial.print("  reset=");
  Serial.println(gstat & 0x1);
  Serial.print("  drv_err=");
  Serial.println((gstat >> 1) & 0x1);
  Serial.print("  uv_cp (real VM undervoltage flag)=");
  Serial.println((gstat >> 2) & 0x1);

  // hold-torque specific diagnostics
  Serial.print("toff="); Serial.println(driver.toff());               // 0 = output stage fully disabled, must be nonzero
  Serial.print("ihold="); Serial.println(driver.ihold());
  Serial.print("irun="); Serial.println(driver.irun());
  Serial.print("cs_actual="); Serial.println(driver.cs_actual());     // current scale the driver is ACTUALLY applying right now
  Serial.print("stst (standstill)="); Serial.println(driver.stst());
  Serial.print("ot (overtemp shutdown)="); Serial.println(driver.ot());
  Serial.print("otpw (overtemp prewarn)="); Serial.println(driver.otpw());
  Serial.print("s2ga (short to gnd, coil A)="); Serial.println(driver.s2ga());
  Serial.print("s2gb (short to gnd, coil B)="); Serial.println(driver.s2gb());
  Serial.print("ola (open load, coil A -- unreliable at standstill, informational only)="); Serial.println(driver.ola());
  Serial.print("olb (open load, coil B -- unreliable at standstill, informational only)="); Serial.println(driver.olb());
}

void setup() {
  Serial.begin(9600);
  Serial.print("new_brd_testing, mode=");
  Serial.println(TEST_MODE);

  pinMode(PIN_SW, INPUT_PULLUP);
  pinMode(PIN_LED_IR, OUTPUT);
  digitalWrite(PIN_LED_IR, LOW);
  pinMode(PIN_SOL_PWM, OUTPUT);
  digitalWrite(PIN_SOL_PWM, LOW);
  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, HIGH);   // driver disabled by default; mode 3 enables it

  if (TEST_MODE == 3) {
    initStepperUART();
  }
}

// ---------------- mode 1: I/O test, prints @ 2Hz ----------------
void loop_mode1() {
  ledState = !ledState;
  digitalWrite(PIN_LED_IR, ledState ? HIGH : LOW);

  int swRaw = digitalRead(PIN_SW);              // 1 = HIGH (idle), 0 = LOW (pressed)
  float potiVal = analogRead(PIN_POTI) / 1023.0; // 0.00 - 1.00

  Serial.print('[');
  Serial.print(ledState ? 1 : 0);
  Serial.print('|');
  Serial.print(swRaw);
  Serial.print('|');
  Serial.print(potiVal, 2);
  Serial.println(']');

  delay(500);   // 2 Hz
}

// ---------------- mode 2: IR / shutter test ----------------
void loop_mode2() {
  int swRaw = digitalRead(PIN_SW);   // LOW = pressed (INPUT_PULLUP)
  if (swRaw == LOW && lastSwState == HIGH) {
    fireShutter();
    delay(50);   // crude debounce
  }
  lastSwState = swRaw;
}

// ---------------- mode 3: motor test (UART VACTUAL, no STEP/DIR wired) ----------------
// VACTUAL units are ~0.715 Hz per LSB (v[Hz] = VACTUAL * fCLK/2^24, fCLK ~12MHz internal).
// At microsteps(0) (full step) that's full-steps/sec directly. 300 =~ 215Hz =~ 1.07 rev/s --
// a NEMA17 can actually follow this from a dead stop with no ramp. The previous value (30000)
// was ~21kHz =~ 6400 RPM commanded instantly from standstill -- the motor just can't do that,
// which alone would look exactly like "holds torque but never rotates."
void printMotionDiag() {
  Serial.print("  cs_actual="); Serial.print(driver.cs_actual());
  Serial.print("  stst=");      Serial.print(driver.stst());
  Serial.print("  drv_err=");   Serial.println((driver.GSTAT() >> 1) & 0x1);
}

void loop_mode3() {
  const int32_t TEST_VELOCITY = 300;   // ~1 rev/s at full step -- sane starting point, raise gradually once this actually turns

  Serial.print("forward");
  driver.VACTUAL(TEST_VELOCITY);
  printMotionDiag();
  delay(1000);

  Serial.print("reverse");
  driver.VACTUAL(-TEST_VELOCITY);
  printMotionDiag();
  delay(1000);
}

// ---------------- mode 4: solenoid test ----------------
void loop_mode4() {
  Serial.println("solenoid ON");
  digitalWrite(PIN_SOL_PWM, HIGH);
  delay(500);

  Serial.println("solenoid OFF");
  digitalWrite(PIN_SOL_PWM, LOW);
  delay(1500);
}

void loop() {
  if (TEST_MODE == 1) loop_mode1();
  else if (TEST_MODE == 2) loop_mode2();
  else if (TEST_MODE == 3) loop_mode3();
  else if (TEST_MODE == 4) loop_mode4();
}
