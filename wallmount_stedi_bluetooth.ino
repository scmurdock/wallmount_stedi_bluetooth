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

volatile unsigned long pulseInTimeBegin = micros();//Returns the number of microseconds since the Arduino board began running the current program
volatile unsigned long pulseInTimeEnd = micros();//Returns the number of microseconds since the Arduino board began running the current program
volatile bool newPulseDurationAvailable = false;// volatile ensures the variable is stored in main memory, not in registers, ensuring that it is accessible - this variable is a semaphore used to signal to the main thread that some new data is ready
volatile bool awaitingEchoFromSensor = false;//this is a semaphore variable used to tell us we are still waiting for a response from the HC SR04, so not to send it another request

long duration;
int distance=0;


//below code and more copied from https://forum.arduino.cc/t/non-blocking-ultrasonic-sensor-without-interrupts-and-timers/382076/10

enum us_sensor_phase { 
  trigger, // Phase, in which the US sensor is triggered to send pings.
  sending, // Phase, in which we wait until all pings have been sent by the US sensor.
  receive  // Phase, in which we await the echo of the US sensor pings.
};

enum us_sensor_phase ultrasonicPhase = trigger; // The current phase of the sensor
unsigned long ultrasonicPhaseTime = 0;          // Remembers when the phase started.

//
//void distanceSensorInterrupt()
//{
// Serial.println("Interrupt occurred");
////  Serial.print("Value of awaitingEchoFromSensor: ");
////  Serial.println(awaitingEchoFromSensor);
////  Serial.print("Value of newPulseDurationAvailable: ");
////  Serial.println(newPulseDurationAvailable);
////  
////  if (awaitingEchoFromSensor){
//    if (digitalRead(echoPin) == HIGH){//the echoPin is the one that sends the HIGH/LOW at the beginning and end of the time duration
//        pulseInTimeBegin = micros();//this is the start of the timer, so we save the time
//    }
//    else{
//      pulseInTimeEnd = micros(); // went from HIGH to LOW, indicating the end of the duration
//      newPulseDurationAvailable = true;
////      Serial.println("New duration made available from interrupt");
////      awaitingEchoFromSensor = false;//we got our answer!
//    }
////  }
//}

void setup(){
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);

  //attachInterrupt(digitalPinToInterrupt(echoPin), distanceSensorInterrupt, CHANGE);//attach the function distanceSensorInterrupt to the echoPin so that it gets called whenever the value changes

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


  switch(ultrasonicPhase){
    case trigger:
      ultrasonicPhase = sending;
      digitalWrite(triggerPin, LOW);
      delayMicroseconds(10);
      digitalWrite(triggerPin, HIGH);
      delayMicroseconds(20);
      digitalWrite(triggerPin, LOW);
      ultrasonicPhaseTime = micros();
      break;
    case sending:
      // Wait until all pings have been sent by the ultrasonic sensor.
      if((micros() - ultrasonicPhaseTime) > 500UL ) {
      // Exit if pings are not sent fast enough
        Serial.println("Error1: Ping not sent");
        ultrasonicPhase = trigger;
      } else if(digitalRead(echoPin) == HIGH) {
        ultrasonicPhase = receive;
        ultrasonicPhaseTime = micros();
      }
      break;
    case receive:
    // Await the echo of a ping.
      if((micros() - ultrasonicPhaseTime) > 20000UL) {
        // Exit if no echo is received (e.g. because too far away)
        Serial.println("Error2: No echo received");
        ultrasonicPhase = trigger;
      } else if(digitalRead(echoPin) == LOW) {
        duration = (micros() - ultrasonicPhaseTime);
        distance = duration / 58.2;
        ultrasonicPhase = trigger;
      }
      break;
  }

      
    
  

//
//  if (newPulseDurationAvailable){//this variable is a semaphore used to signal to the main thread that some new data is ready
//    newPulseDurationAvailable = false;//reset the flag so that on the next loop we don't re-read the same data twice
//    unsigned long pulseDuration = pulseInTimeEnd - pulseInTimeBegin;
//    distance = pulseDuration * .034 /2; //speed of sound wave divided by 2 (go and back)
//    Serial.print("Received signal from interrupt value of distance received: ");
//    Serial.print(distance);
//    Serial.println(" cm");
//
//    Serial.println("Sending trigger to trigger pin, awaiting response...");
//    digitalWrite(triggerPin, LOW);
//    delayMicroseconds(2);
//    digitalWrite(triggerPin, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(triggerPin,LOW);    
//  }

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

//  if (!awaitingEchoFromSensor){//send request for an echo using trigger pin
//    
//    Serial.println("Sending trigger to trigger pin, awaiting response...");
//    digitalWrite(triggerPin, LOW);
//    delayMicroseconds(2);
//    digitalWrite(triggerPin, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(triggerPin,LOW);
//
//    awaitingEchoFromSensor=true;//we now are waiting for an answer
//  }
  
  delay(200);            
}
