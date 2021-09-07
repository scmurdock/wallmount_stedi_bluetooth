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
#include <ArduinoBLE.h>

BLEService wallMountService("stedibalancev300bluetoothlenerg");
BLEUnsignedCharCharacteristic distanceSensorCharacteristic("1212", BLERead | BLENotify);

int triggerPin=2;
int echoPin=3;

long duration;
int distance;

void setup(){
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  while(!Serial);

  if(!BLE.begin())
  {
    Serial.println("Starting BLE failed!");
    while(1);
  }

  BLE.setLocalName("STEDIWallMountV3.00");
  BLE.setAdvertisedService(wallMountService);
  wallMountService.addCharacteristic(distanceSensorCharacteristic);
  BLE.addService(wallMountService);

  BLE.advertise();
  Serial.println("Bluetooth STEDI device active, waiting for connections ...");  
}

void loop(){

  getDistance();

  BLEDevice central = BLE.central();

  if (central)
  {
    Serial.println("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);


    while (central.connected()) {
      distance = getDistance();
      distanceSensorCharacteristic.writeValue(distance);
      delay(200);            
    }    
  }
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());
}

int getDistance(){

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
  return distance;  
}
