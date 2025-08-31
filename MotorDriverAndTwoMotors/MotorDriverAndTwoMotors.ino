#include <TaskScheduler.h>
#include <Servo.h>

// --------- Pins ----------
// distance sensor
const uint8_t ECHO_PIN = 3;
const uint8_t TRIG_PIN = 4;
// motor right
const uint8_t ENA = 10;   // Right motor enable (PWM)
const uint8_t IN1 = 9;   // Right motor dir A
const uint8_t IN2 = 8;   // Right motor dir B
// motor left
const uint8_t ENB = 5;  // Left motor enable (PWM)
const uint8_t IN3 = 7;   // Left motor dir A
const uint8_t IN4 = 6;   // Left motor dir B
// servos
const uint8_t SERVO_LEFTRIGHT = 11;
const uint8_t SERVO_UPDOWN = 12;

// --------- State / Filtering ----------
volatile unsigned long lastDurationUs = 0;  // last echo duration (μs)
float lastDistanceCm = NAN;                 // last computed distance (cm)
float filteredCm = NAN;                     // smoothed distance
const float alpha = 0.35f;                  // EWMA smoothing factor

// Optional: define max range timeout (30 ms ~ ~5m round-trip; HC-SR04 spec ~4m)
const unsigned long ECHO_TIMEOUT_US = 30000UL;

// --------- Scheduler & Tasks ----------
Scheduler runner;
void taskMeasureDistance();   // takes a measurement
// void taskReportState();  // prints the latest value
void taskChangeSpeed(); // changes speed and forward/backwards direction
void taskMoveLeftRight();
void taskMoveUpDown();
// Create tasks: (interval in milliseconds, iterations, callback)
Task tMeasureDistance(20, TASK_FOREVER, &taskMeasureDistance);
// Task tReportState(1000, TASK_FOREVER, &taskReportState);
Task tChangeSpeed(40, TASK_FOREVER, &taskChangeSpeed);
Task tMoveLeftRight(50, TASK_FOREVER, &taskMoveLeftRight);
Task tMoveUpDown(60, TASK_FOREVER, &taskMoveUpDown);

// Servo objects
Servo servoLeftRight;
Servo servoUpDown;

// Servo movement variables
int angleLR = 120;
int stepLR  = 4;
int angleUD = 100;
int stepUD  = 2;

// ------ Influence speed & direction ---------
uint8_t speed = 255;
bool shallDriveForward = true;
int direction = 0; // 0 = straight forward, -127 = spin left, +127 = spin right, -1..-126 = turn left, 1 ..126 turn right

void setup() {
  setupMotorDriver();
  setupServos();
  setupDistanceSensor();
  setupTaskScheduler();

  // Setup serial port (USB/RS232 out)
  //Serial.begin(9600);
}

void loop() {
  runner.execute();
}

void setupServos(){
  // Attach servos
  servoLeftRight.attach(SERVO_LEFTRIGHT);
  servoUpDown.attach(SERVO_UPDOWN);

  // Initial positions
  servoLeftRight.write(angleLR);
  servoUpDown.write(angleUD);
}

void setupMotorDriver() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void setupDistanceSensor() {
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
}

void setupTaskScheduler() {
  runner.init();
  
  runner.addTask(tMeasureDistance);
  // runner.addTask(tReportState);
  runner.addTask(tChangeSpeed);
  runner.addTask(tMoveLeftRight);
  runner.addTask(tMoveUpDown);

  tMoveLeftRight.enable();
  tMoveUpDown.enable();
  tMeasureDistance.enable();
  // tReportState.enable();
  tChangeSpeed.enable();
}

// Task function for left-right servo
void taskMoveLeftRight() {
  angleLR += stepLR;
  if (angleLR >= 170 || angleLR <= 70) {
    stepLR = -stepLR; // reverse direction
  }
  servoLeftRight.write(angleLR);
}

// Task function for up-down servo
void taskMoveUpDown() {
  angleUD += stepUD;
  if (angleUD >= 140 || angleUD <= 90) {
    stepUD = -stepUD; // reverse direction
  }
  servoUpDown.write(angleUD);
}

void taskChangeSpeed() {
  int speedMotorA = speed; //- direction;
  int speedMotorB = speed; //+ direction;
  shallDriveForward = true;
  // something on our path?
  if (filteredCm > 50 || filteredCm < 20){
    speedMotorA = 0;
    speedMotorB = 0;
  }
  // something right in fron of us? If yes, drive backwards at max speed
  if (filteredCm <= 5) {
    speedMotorA = 255;
    speedMotorB = 255;
    shallDriveForward = false;
  }
  
  digitalWrite(IN1, !shallDriveForward);
  digitalWrite(IN2, shallDriveForward);
  analogWrite(ENA, speedMotorA); // full speed left motor
  
  digitalWrite(IN3, shallDriveForward);
  digitalWrite(IN4, !shallDriveForward);
  analogWrite(ENB, speedMotorB); // full speed right motor
}

void taskMeasureDistance() {
  // Trigger a 10 μs pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse width with a short timeout to avoid long blocking
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  lastDurationUs = duration;

  // Convert to distance (cm). Speed of sound ~0.0343 cm/μs, divide by 2 (round trip)
  if (duration == 0) {
    // No echo within timeout → out of range / invalid reading
    lastDistanceCm = NAN;
  } else {
    lastDistanceCm = (duration * 0.0343f) * 0.5f;
  }

  // Exponential smoothing for stability
  if (isnan(lastDistanceCm)) {
    // If invalid reading, gently decay toward NaN but keep previous filtered value
    // (Do nothing; we’ll just report the last good filtered value)
  } else if (isnan(filteredCm)) {
    filteredCm = lastDistanceCm;
  } else {
    filteredCm = alpha * lastDistanceCm + (1.0f - alpha) * filteredCm;
  }
}

void taskReportState() {
  Serial.print(F("Raw: "));
  if (isnan(lastDistanceCm)) {
    Serial.print(F("—"));
  } else {
    Serial.print(lastDistanceCm, 1);
    Serial.print(F(" cm"));
  }

  Serial.print(F("   |   Smoothed: "));
  if (isnan(filteredCm)) {
    Serial.println(F("—"));
  } else {
    Serial.print(filteredCm, 1);
    Serial.println(F(" cm"));
  }
}