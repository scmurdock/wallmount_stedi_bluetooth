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
BLEUnsignedIntCharacteristic distanceSensorCharacteristic("1212", BLERead | BLENotify);

int triggerPin=2;
int echoPin=3;

volatile unsigned long pulseInTimeBegin = micros();//microseconds
volatile unsigned long pulseInTimeEnd = micros();//microseconds
volatile bool newPulseDurationAvailable = false;// volatile ensures the variableis stored in main memory, not in registers, ensuring that it is accessible - this variable is a semaphore used to signal to the main thread that some new data is ready
volatile bool awaitingEchoFromSensor = false;//this is a semaphore variable used to tell us we are still waiting for a response from the HC SR04, so not to send it another request

long duration;
int distance=0;

void distanceSensorInterrupt()
{
  if (digitalRead(echoPin) == HIGH){//the echoPin is the one that sends the HIGH/LOW at the beginning and end of the time duration
      pulseInTimeBegin = micros();//this is the start of the timer, so we save the time
  }
  else{
    pulseInTimeEnd = micros(); // went from HIGH to LOW, indicating the end of the duration
    newPulseDurationAvailable = true;
    Serial.println("New duration made available from interrupt");
    awaitingEchoFromSensor = false;//we got our answer!
  }
}

void setup(){
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(echoPin), distanceSensorInterrupt, CHANGE);//attach the function distanceSensorInterrupt to the echoPin so that it gets called whenever the value changes

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
  
  if (Serial){
    Serial.println("Bluetooth STEDI device active, waiting for connections ...");  
  }
}

void loop(){

  //getDistance();

  if (newPulseDurationAvailable){//this variable is a semaphore used to signal to the main thread that some new data is ready
    newPulseDurationAvailable = false;//reset the flag so that on the next loop we don't re-read the same data twice
    unsigned long pulseDuration = pulseInTimeEnd - pulseInTimeBegin;
    distance = pulseDuration * .034 /2; //speed of sound wave divided by 2 (go and back)
    Serial.println("Received signal from interrupt value of distance received: "+distance);
  }

  if (!awaitingEchoFromSensor){//send request for an echo using trigger pin
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin,LOW);
    Serial.println("Sent trigger to trigger pin, awaiting response...");
    awaitingEchoFromSensor=true;//we now are waiting for an answer
  }

  BLEDevice central = BLE.central();

  if (central)
  {
    if(Serial){
      Serial.println("Connected to central: ");
      Serial.println(central.address());
    }
    digitalWrite(LED_BUILTIN, HIGH);


    while (central.connected()) {
      distanceSensorCharacteristic.writeValue(distance);
      delay(200);            
    }    
  }
  digitalWrite(LED_BUILTIN, LOW);

  if (Serial){
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

int getDistance(){

  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin,LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * .034 /2; //speed of sound wave divided by 2 (go and back)
  
  if (Serial){
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }
  return distance;  
}
