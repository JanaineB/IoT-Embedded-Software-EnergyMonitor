// LIBS
#include <Arduino.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <EmonLiteESP.h>
/*---------------- DATABASE-------------------*/
#define FIREBASE_HOST "yourFB-host.firebaseio.com"
#define FIREBASE_AUTH "yourFBauth"
//Your Wi-Fi SSID and Password
#define WIFI_SSID "netvirtua260"
#define WIFI_PASSWORD "3123270860"
/*------ESP8266 DIFINES FOR MEASUREMENT------------*/
// Analog GPIO on the ESP8266
#define CURRENT_PIN             0
// If using a node ESP8266 board it will be 1.0V, if using a NodeMCU there
// is a voltage divider in place, so use 3.3V instead.
#define REFERENCE_VOLTAGE       3.3
// Precision of the ADC measure in bits. Arduinos and ESP8266 use 10bits ADCs, but the
// ADS1115 is a 16bits ADC
#define ADC_BITS                10
// Number of decimal positions for the current output
#define CURRENT_PRECISION       3
// This is basically the volts per amper ratio of your current measurement sensor.
// If your sensor has a voltage output it will be written in the sensor enclosure,
// something like "30V 1A", otherwise it will depend on the burden resistor you are
// using.
#define CURRENT_RATIO           23   // based on 30ohm burden resistor and 2000 windings in SCT-013-000 //29
// This version of the library only calculate aparent power, so it asumes a fixes
// main voltage
#define MAIN_VOLTAGE           127
// Number of samples each time you measure
#define SAMPLES_X_MEASUREMENT   1024

EmonLiteESP power;

unsigned int currentCallback() {
  // If using the ADC GPIO in the ESP8266
  return analogRead(CURRENT_PIN);
}

void setup() {
  Serial.begin(115200);
  //Conexão com o WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("\n connecting");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  power.initCurrent(currentCallback, ADC_BITS, REFERENCE_VOLTAGE, CURRENT_RATIO);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

}
String place = "Sala";
String hardware = "Lampadas";


void loop() {
  // handle error
  if (Firebase.failed()) {
    Serial.print("setting /number failed:");
    Serial.println(Firebase.error());
    return;
  }
  
  float current = power.getCurrent(SAMPLES_X_MEASUREMENT);
  int power = int (current * MAIN_VOLTAGE);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& timeObject = jsonBuffer.createObject();
  timeObject["place"] = place;
  timeObject["hardware"] = hardware;
  timeObject["current"] = current;
  timeObject["power"] = power;
  JsonObject& tempTime = timeObject.createNestedObject("timestamp");
  tempTime[".sv"] = "timestamp";
  
  if(current <= 0){
    Serial.print("NO MEASUREMENT DETECTED - Object: " ); Serial.print(timeObject);
  }else Firebase.push("sensor/", timeObject);
 
  delay(60000);
}
