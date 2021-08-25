/*
 * We are going to power the dot laser and
 * the line laser with separate digital pins
 * so that we can activate them via bluetooth.
 * 
 * There are 9 digital pins, D2 through D10
 * 
 * We will also be using digital pins for the
 * HC-SR04 compatible ultrasonic sensor trigger pin
 * and echo pin.
 * 
 * 
 */

int dotLaserPin=2;
int lineLaserPin=3;

void setup() {
  pinMode(dotLaserPin, OUTPUT);
  pinMode(lineLaserPin, OUTPUT);
}

void loop() {
  digitalWrite(dotLaserPin, HIGH);
  digitalWrite(lineLaserPin, HIGH);
}
