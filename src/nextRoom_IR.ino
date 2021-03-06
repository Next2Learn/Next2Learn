/*
 *  This file is a sample application, based
 *  on the IoT Prototyping Framework (IoTPF)

    This application and IoTPF is free software: you can redistribute it     and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IoTPF is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU v3 General Public License for more details.

    Released under GNU v3

    You should have received a copy of the GNU General Public License
    along with IoTPF.  If not, see <http://www.gnu.org/licenses/>.
 */

 /**
    Internet of Things Prototyping Framework IoTPF
    @Version 1.0
    @author Dionysios Satikidid
    
    Next2Learn 2018/2019
    @version 1.0
    @author Dennis Ebner, Ilker Porsuk, Andreas Greiß
*/

#include "Adafruit_VL53L0.h"

// address we will assign if dual sensor is present
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

// set the pins to shutdown
#define SHT_LOX1 7
#define SHT_LOX2 6
#define debuggLED 3
#define blueLED 4
#define redLED  5
#define iteration 20
#define taster 15


int dist1=0;
int dist2=0;
int startDist1=0;
int startDist2=0;
int closedDoorDist=0;
int people=0;

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;

/*
    Reset all sensors by setting all of their XSHUT pins low for delay(10), then set all XSHUT high to bring out of reset
    Keep sensor #1 awake by keeping XSHUT pin high
    Put all other sensors into shutdown by pulling XSHUT pins low
    Initialize sensor #1 with lox.begin(new_i2c_address) Pick any number but 0x29 and it must be under 0x7F. Going with 0x30 to 0x3F is probably OK.
    Keep sensor #1 awake, and now bring sensor #2 out of reset by setting its XSHUT pin high.
    Initialize sensor #2 with lox.begin(new_i2c_address) Pick any number but 0x29 and whatever you set the first sensor to
 */

void setID() {
  // all reset
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  delay(10);
  // all unreset
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  // activating LOX1 and reseting LOX2
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(debuggLED, HIGH);
  delay(1000);
  digitalWrite(debuggLED, LOW);
  delay(1000);
  digitalWrite(debuggLED, HIGH);


  // initing LOX1
  if(!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));

//    while(1);
  }
  delay(10);

  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);
  //initing LOX2
  if(!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
//    while(1);
  }
}

void toggleLED(int LED) {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);

}

void read_dual_sensors() {

  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!
  dist1=measure1.RangeMilliMeter;
  dist2=measure2.RangeMilliMeter;

}

int init_dual_sensors() {
  Serial.println("get the distance: ");
  int startDist1tmp=1;
  int startDist2tmp=1;

  read_dual_sensors();

  while(startDist1!=startDist1tmp &&startDist2!=startDist2tmp){
    for (int i=0;i<=10;i++){
    toggleLED(debuggLED);
    toggleLED(blueLED);
    toggleLED(redLED);
    }
    read_dual_sensors();
    startDist1tmp=dist1;
    startDist2tmp=dist2;
    read_dual_sensors();
    startDist1=dist1;
    startDist2=dist2;


    Serial.print("dist1= ");
    Serial.println(startDist1);
    Serial.print("dist2= ");
    Serial.println(startDist2);
    digitalWrite(debuggLED,HIGH);
    delay(1000);
    digitalWrite(debuggLED,LOW);
  }
}





typedef enum STATE{WAIT,OPENDOOR,IN,OUT} STATE_TYPE;

STATE_TYPE actState = WAIT;
STATE_TYPE nextState = WAIT;

int tmp1=0;
int tmp2=0;
int val;


void setup() {

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  pinMode(debuggLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  digitalWrite(debuggLED, HIGH);

  Serial.begin(115200);

  // wait until serial port opens for native USB devices
  while (! Serial) { delay(1); }
  Serial.println("Shutdown pins inited...");

  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);

  Serial.println("Both in reset mode...(pins are low)");

  Serial.println("Starting...");
  setID();
  init_dual_sensors();
  Particle.publish("taster",String(analogRead(A5)));
  while (closedDoorDist<=800){
    if(analogRead(A5)>=1000){
      read_dual_sensors();
      closedDoorDist=dist2;
      break;
    }
    toggleLED(redLED);
    delay(1000);
    Particle.publish("taster",String(analogRead(A5)));
  }
  digitalWrite(redLED, LOW);
  Particle.publish("closedDoorDist",String(closedDoorDist));


}


void loop() {

  switch(actState) {

        case WAIT:
        digitalWrite(blueLED, LOW);
        digitalWrite(redLED, LOW);
        digitalWrite(debuggLED, LOW);
        read_dual_sensors();
        if(dist2>=closedDoorDist+100){
          nextState=OPENDOOR;
          break;
        }
        else {
          nextState=WAIT;
          break;
        }

        case OPENDOOR:
        digitalWrite(blueLED, LOW);
        digitalWrite(redLED, LOW);
        digitalWrite(debuggLED, LOW);
        read_dual_sensors();
        if (startDist2-100>dist2) {
              nextState = IN;
              break;
            }
        else if (startDist1-100>dist1){   //black&white
                nextState = OUT;
                break;
            }
        else if(dist2<=closedDoorDist+40){
              nextState=WAIT;
            }
        break;

        case IN:
//        digitalWrite(blueLED, HIGH);
        for (int i=0;i<iteration;i++){
//          delay(20);
          read_dual_sensors();
          if(startDist1-100>dist1){
              people++;
              Particle.publish("people",String(people));
              toggleLED(debuggLED);
              nextState=WAIT;
              break;
          }
        }
        nextState=WAIT;
        break;

        case OUT:
//        digitalWrite(redLED, HIGH);
        for(int i=0;i<iteration;i++){
//          delay(20);
          read_dual_sensors();
          if (startDist2-100>dist2){
              if(people>=1){
              people--;
            }
            else{people=0;}
              Particle.publish("people",String(people));
              toggleLED(debuggLED);
              nextState=WAIT;
              break;
          }
        }
        nextState=WAIT;
        break;

        }
        actState=nextState;
}
