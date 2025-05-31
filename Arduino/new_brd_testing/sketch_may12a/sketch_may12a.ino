// Define pins
const int LED1 = 9;
const int LED2 = 8;
const int LED3 = 2;

const int SW_DN = A1;
const int SW_UP = A2;
const int BTN_A = A3;
const int BTN_B = A4;
const int BTN_C = A5;
const int POT   = A6;

// State tracking for downtrigger detection
bool prev_SW_DN = HIGH;
bool prev_SW_UP = HIGH;
bool prev_BTN_A = HIGH;
bool prev_BTN_B = HIGH;
bool prev_BTN_C = HIGH;

const in t DELAY = 100; // Delay in milliseconds

void setup() {
	// LED pins
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);

	// Input pins with internal pull-up
	pinMode(SW_DN, INPUT_PULLUP);
	pinMode(SW_UP, INPUT_PULLUP);
	pinMode(BTN_A, INPUT_PULLUP);
	pinMode(BTN_B, INPUT_PULLUP);
	pinMode(BTN_C, INPUT_PULLUP);

	Serial.begin(9600);
}

void loop() {
	// --- LED twinkle ---
	digitalWrite(LED1, HIGH);
	delay(DELAY);
	digitalWrite(LED1, LOW);

	digitalWrite(LED2, HIGH);
	delay(DELAY);
	digitalWrite(LED2, LOW);

	// Inverted logic for LED3 (since cathode is to +5V)
	digitalWrite(LED3, LOW);
	delay(DELAY);
	digitalWrite(LED3, HIGH);

	// --- Downtrigger detection ---

	bool curr_SW_DN = digitalRead(SW_DN);
	bool curr_SW_UP = digitalRead(SW_UP);
	bool curr_BTN_A = digitalRead(BTN_A);
	bool curr_BTN_B = digitalRead(BTN_B);
	bool curr_BTN_C = digitalRead(BTN_C);

	if (prev_SW_DN == HIGH && curr_SW_DN == LOW) {
		Serial.println("SW_DN");
	}
	if (prev_SW_UP == HIGH && curr_SW_UP == LOW) {
		Serial.println("SW_UP");
	}
	if (prev_BTN_A == HIGH && curr_BTN_A == LOW) {
		Serial.println("A");
	}
	if (prev_BTN_B == HIGH && curr_BTN_B == LOW) {
		Serial.println("B");
	}
	if (prev_BTN_C == HIGH && curr_BTN_C == LOW) {
		Serial.println("C");
	}

	prev_SW_DN = curr_SW_DN;
	prev_SW_UP = curr_SW_UP;
	prev_BTN_A = curr_BTN_A;
	prev_BTN_B = curr_BTN_B;
	prev_BTN_C = curr_BTN_C;

	// --- Analog read loop if SW_UP is held LOW ---
	if (curr_SW_UP == LOW) {
		int val = analogRead(POT);
		int percent = map(val, 0, 1023, 0, 100);
		Serial.println(val);
		delay(1);
	}
}