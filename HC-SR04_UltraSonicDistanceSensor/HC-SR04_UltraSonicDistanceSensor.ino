// Define pins
const int echoPin = 3;  // ECHO connected to pin 3
const int trigPin = 4;  // TRIG connected to pin 4

// Variable to store duration of the echo
long duration;
// Variable to store distance
int distance;

void setup() {
  // Initialize serial communication at 9600 baud
  Serial.begin(9600);

  // Set trigPin as OUTPUT and echoPin as INPUT
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Trigger the sensor by sending a 10 microsecond HIGH pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance: sound speed is ~343 m/s
  // distance (mm) = duration * 0.034 / 2
  distance = duration * 0.34 / 2;

  // Print distance on Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  // Small delay between measurements
  delay(100);
}
