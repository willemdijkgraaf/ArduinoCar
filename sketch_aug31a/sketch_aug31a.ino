// Pin definitions (your wiring)
const uint8_t ENA = 5;   // Left motor enable (PWM)
const uint8_t ENB = 10;  // Right motor enable (PWM)
const uint8_t IN1 = 9;   // Left motor dir A
const uint8_t IN2 = 8;   // Left motor dir B
const uint8_t IN3 = 7;   // Right motor dir A
const uint8_t IN4 = 6;   // Right motor dir B

void setup() {
  // Set all pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Drive both motors forward at full speed
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 255); // full speed left motor
  analogWrite(ENB, 255); // full speed right motor
}

void loop() {
  // Nothing needed — motors keep running forward
}
