/*
 * 
 * There are 9 digital pins, D2 through D10
 * 
 * We will be using digital pins for the
 * HC-SR04 compatible ultrasonic sensor trigger pin
 * and echo pin.
 * 
 * 
 */

int triggerPin=2;
int echoPin=3;

long duration;
int distance;

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);//Serial Communication startgs at 9600 rate
  Serial.println("Sensor Test");
  
}

void loop() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin,LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * .034 /2; //speed of sound wave divided by 2 (go and back)
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}
