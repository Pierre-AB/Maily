/* 
 *  FEATURES:
 *  First measurement
 *  Lowpower mode (to be improved / SLEEP_MODE_PWR_DOWN or C byte code)
 *  Improve code with volatile variable.
 *  SMS sending.
 *  Mail counting.
 *  Location feature is working through SMS, nevertheless on battery it is not working. I believe this is due to lack of power: recharge battery ? or manage power differently, meaning: code or power source ? 
 *  In order to run autonomously the arduino, it is necessary to load the sketch on the board with the battery attached.
 *  Is GSM initialization each time consume less energy than once for all?
*/

#include <MKRGSM.h>
#include <ArduinoLowPower.h>
#include "arduino_secrets.h"


//variables for distance measuring
const int trigPin = 6;
const int echoPin = 7;
const int reedPin = 5;
int clapNumber = 0;
int mailNumber = 0;
long firstDuration;
int firstDistance;
long duration;
int distance;

volatile byte reedState = LOW;

//GSM variables
const char PINNUMBER[] = SECRET_PINNUMBER;
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;
const char CONTACTNUM[] = SECRET_CONTACTNUM;
String txtMsg;

//location Variable
String GSMLatitude = "0.000000";
String GSMLongitude = "0.000000";

//Initialize the library instance (GPRS is needed to get coordinates)
GPRS gprs;
GSM gsmAccess;
GSM_SMS sms;
GSMLocation location;


void getLocation() {
  
  unsigned long timeout = millis();
  while (millis() - timeout < 45000) {
    if (location.available() && location.accuracy() < 300 && location.accuracy() !=0){
      GSMLatitude = String(location.latitude(), 6);
      GSMLongitude = String(location.longitude(), 6);
     break;
    }
  }
}

void connectNetwork() {
  
bool connected = false;

gprs.setTimeout(10000);
gsmAccess.setTimeout(10000);

while (!connected){
    
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) && gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY){
      connected = true;
    } else {
      Serial.println("Not Connected");
      delay(1000);
    }
  }
  
Serial.println("Network reached");
  
}

void sendSMS() {
 
     sms.beginSMS(CONTACTNUM);
     sms.print(mailNumber);
     //sms.print("\nhttps://www.google.com/maps/place/" + GSMLatitude + "," + GSMLongitude);
     sms.endSMS();            
  
}

void setup() {
  
Serial.begin(9600);
    
//Energy savings by writing all Digital IO status
for(int i=0; i<15; i++){
   if(i != 2 || i !=5 ){
    pinMode(i, OUTPUT);}
  }
  pinMode(reedPin, INPUT);
  pinMode(echoPin,INPUT);
  
  connectNetwork();
  //location.begin();
  Serial.println("Location acquired");

// first distance calculation
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(70);
digitalWrite(trigPin, LOW);
firstDuration = pulseIn(echoPin, HIGH);
firstDistance = firstDuration*0.034/2;

Serial.println("First Distance: ");
Serial.println(firstDistance);

LowPower.attachInterruptWakeup(reedPin, mailChecking, CHANGE);

  
}


void loop() {
   
   clapNumber ++;
    
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(70);
      digitalWrite(trigPin, LOW);
      
      duration = pulseIn(echoPin, HIGH);
      distance = duration*0.034/2;
      
        if (distance < firstDistance*0.75){
            mailNumber ++;
            //getLocation();
            sendSMS();    
        }

        //____DISTANCE CHECKING____
        
  Serial.println("Distance: ");
  Serial.println(distance);
  Serial.println("Number of mails: ");
  Serial.println(mailNumber);
  Serial.println("clap Number: ");
  Serial.println(clapNumber);
  
  LowPower.sleep();

}

void mailChecking() {
reedState = !reedState;

}



//void boxMeasurement() {
//digitalWrite(trigPin, LOW);
//delayMicroseconds(2);
//digitalWrite(trigPin, HIGH);
//delayMicroseconds(70);
//digitalWrite(trigPin, LOW);
//  }
