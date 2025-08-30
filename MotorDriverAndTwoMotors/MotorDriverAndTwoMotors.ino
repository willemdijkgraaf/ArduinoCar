// Pin definitions
const uint8_t ENA = 10;   // Right motor enable (PWM)
const uint8_t IN1 = 9;   // Right motor dir A
const uint8_t IN2 = 8;   // Right motor dir B

const uint8_t ENB = 5;  // Left motor enable (PWM)
const uint8_t IN3 = 7;   // Left motor dir A
const uint8_t IN4 = 6;   // Left motor dir B

uint8_t speed = 120;
bool shallDriveForward = true;
int8_t direction = -40; // 0 = straight forward, -127 = spin left, +127 = spin right, -1..-126 = turn left, 1 ..126 turn right

void setup() {
  // Set all pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  // Nothing needed here, keeps running forward
  analogWrite(ENA, speed - direction); // full speed left motor
  analogWrite(ENB, speed + direction); // full speed right motor

  // Drive both motors forward at full speed

  digitalWrite(IN1, !shallDriveForward);
  digitalWrite(IN2, shallDriveForward);
  digitalWrite(IN3, shallDriveForward);
  digitalWrite(IN4, !shallDriveForward);

}
