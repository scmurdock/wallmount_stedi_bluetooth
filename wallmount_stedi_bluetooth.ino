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
#include <NewPing.h> //this library has built in interrupts, and median filtering

#define TRIGGER_PIN 2 //this is the trigger pin on the HC-SR04
#define ECHO_PIN 3 // this is the echo pin on the HC-SR04
#define MAX_DISTANCE 100 //the wall mount device should not ever exceed 1 meter in height when attached to the wall, this is not a supported height
#define DOT_LASER_PIN 2 // this is the dot laser pin on the arduino board -- we may want to turn this on and off based off of bluetooth signals
#define LINE_LASER_PIN 3 // this is the line laser pin on the arduino board -- we may want to turn this on and off based off of bluetooth signals

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

BLEService wallMountService("stedibalancev300bluetoothlenerg");
BLEUnsignedIntCharacteristic distanceSensorCharacteristic("1212", BLERead | BLENotify);

int triggerPin=2;
int echoPin=3;

volatile unsigned long pulseInTimeBegin = micros();//Returns the number of microseconds since the Arduino board began running the current program
volatile unsigned long pulseInTimeEnd = micros();//Returns the number of microseconds since the Arduino board began running the current program
volatile bool newPulseDurationAvailable = false;// volatile ensures the variable is stored in main memory, not in registers, ensuring that it is accessible - this variable is a semaphore used to signal to the main thread that some new data is ready
volatile bool awaitingEchoFromSensor = false;//this is a semaphore variable used to tell us we are still waiting for a response from the HC SR04, so not to send it another request

long duration;
int distance=0;

enum us_sensor_phase { 
  trigger, // Phase, in which the US sensor is triggered to send pings.
  sending, // Phase, in which we wait until all pings have been sent by the US sensor.
  receive  // Phase, in which we await the echo of the US sensor pings.
};

enum us_sensor_phase ultrasonicPhase = trigger; // The current phase of the sensor
unsigned long ultrasonicPhaseTime = 0;          // Remembers when the phase started.


void setup(){
  pinMode(DOT_LASER_PIN, OUTPUT);
  pinMode(LINE_LASER_PIN, OUTPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);

  while(!Serial);
  if(!BLE.begin())
  {
    if (Serial){
      Serial.println("Starting BLE failed!");
      while(1);
    }
  }

  BLE.setLocalName("STEDIWallMountV3.00");
  BLE.setAdvertisedService(wallMountService);
  wallMountService.addCharacteristic(distanceSensorCharacteristic);
  BLE.addService(wallMountService);

  BLE.advertise();
  
  Serial.println("Bluetooth STEDI device active, waiting for connections ...");  
}

void loop(){
  digitalWrite(DOT_LASER_PIN, HIGH);// we have hard-coded the dot laser to be on constantly 
  digitalWrite(LINE_LASER_PIN, HIGH);//we have hard-coded the line laser to be on constantly

  Serial.println("****Variable Values****");
  Serial.print("pulseInTimeBegin: ");
  Serial.println(pulseInTimeBegin);
  Serial.print("pulseInTimeEnd: ");
  Serial.println(pulseInTimeEnd);
  Serial.print("newpulseDurationAvailable: ");
  Serial.println(newPulseDurationAvailable);
  Serial.print("awaitingEchoFromSensor: ");
  Serial.println(awaitingEchoFromSensor);
  Serial.print("duration: ");
  Serial.println(duration);
  Serial.print("distance: ");
  Serial.println(distance);
  Serial.println("*****End Variable Values****");

  distance = sonar.ping_cm();

  BLEDevice central = BLE.central();

  if (central)
  {
    Serial.println("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);


    if (central.connected()) {
      distanceSensorCharacteristic.writeValue(distance);
    }    
  }

  else{
    digitalWrite(LED_BUILTIN, LOW);

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }


  
  delay(50);            
}
