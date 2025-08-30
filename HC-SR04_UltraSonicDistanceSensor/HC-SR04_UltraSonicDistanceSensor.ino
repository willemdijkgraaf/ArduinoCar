/*
  HC-SR04 with TaskScheduler
  TRIG on D3, ECHO on D4 (as requested)
  - Measures frequently (every 60 ms)
  - Reports to Serial every 500 ms
  - No delay() calls; cooperative scheduling
*/

#include <TaskScheduler.h>

// --------- Pins ----------
const uint8_t ECHO_PIN = 3;
const uint8_t TRIG_PIN = 4;

// --------- Scheduler & Tasks ----------
Scheduler runner;

void taskMeasureEcho();   // takes a measurement
void taskReportSerial();  // prints the latest value

// Run measurement ~16.7 Hz; reporting ~2 Hz
Task tMeasure(60, TASK_FOREVER, &taskMeasureEcho);
Task tReport(500, TASK_FOREVER, &taskReportSerial);

// --------- State / Filtering ----------
volatile unsigned long lastDurationUs = 0;  // last echo duration (μs)
float lastDistanceCm = NAN;                 // last computed distance (cm)
float filteredCm = NAN;                     // smoothed distance
const float alpha = 0.35f;                  // EWMA smoothing factor

// Optional: define max range timeout (30 ms ~ ~5m round-trip; HC-SR04 spec ~4m)
const unsigned long ECHO_TIMEOUT_US = 30000UL;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  runner.init();
  runner.addTask(tMeasure);
  runner.addTask(tReport);

  tMeasure.enable();
  tReport.enable();

  Serial.println(F("HC-SR04 with TaskScheduler started."));
}

void loop() {
  runner.execute();
}

void taskMeasureEcho() {
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

void taskReportSerial() {
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
