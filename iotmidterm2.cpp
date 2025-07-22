/* 
 * Project ioTmidterm2
 * Author:Aarav Patel
 * Date: 7/17/25
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Air_Quality_Sensor.h"
#include "Grove_Air_quality_Sensor.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "Adafruit_BME280.h"

//relay
const int relayPin = D7;
unsigned int lastRelay;
int relayState;
unsigned int lastRelay2;
//classes
AirQualitySensor sensor (A0);
TCPClient TheClient; 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 
Adafruit_MQTT_Publish pubFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/airquality");
Adafruit_MQTT_Publish pubFeed3 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisture");
Adafruit_MQTT_Publish pubFeed4 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish pubFeed5 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humid");
Adafruit_MQTT_Publish pubFeed6 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pressure");
Adafruit_MQTT_Subscribe subFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/button");
//publishing
unsigned int lastPublish;
//bme
char number = 164;
Adafruit_BME280 bme;
int hexAddress = 0x76;
float temp, pres, humid, Fahrenheit, inHg;
bool status;
//strings floats ints
unsigned int last, lastTime;
float pubValue,pubValue2,pubValue3,pubValue4,pubValue5,pubValue6,pubValue7;
int subValue;
String dateTime;
String timeOnly;
const int OLED_RESET=-1;
Adafruit_SSD1306 display(OLED_RESET);
const int moisturePin = A5;
float moistureValue;
//functions
void MQTT_connect();
bool MQTT_ping();
 bool displayToggle;
void OLED(void);
SYSTEM_MODE(AUTOMATIC);


SYSTEM_THREAD(ENABLED);


void setup() {
  Serial.begin(9600);
//relay
pinMode(relayPin,OUTPUT);

  //moisture time zone
Time.zone(-6);
Particle.syncTime();

 //oled
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
display.display();
delay(2000);
display.clearDisplay();
  waitFor(Serial.isConnected,10000);

  //bme
  waitFor(Serial.isConnected, 5000);
  status = bme.begin(hexAddress);
  if (status == TRUE) {
    Serial.printf("BME280 at address 0x%02X started successfully!\n",hexAddress);
  }
//wifi
  WiFi.on();
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");
   // Setup MQTT subscription
  mqtt.subscribe(&subFeed);
  while(!Serial);
  //air sensor
  Serial.println("waiting sensor to init");
  delay(20000);
  
  if(sensor.init()){
    Serial.println("Sensor is ready");
  }
  else{
    Serial.println("Sensor Error");
  }
}


void loop() {

  OLED();
  MQTT_connect();
MQTT_ping();
  //bme
  temp = bme.readTemperature();
pres = bme.readPressure();
humid = bme.readHumidity();
Fahrenheit = (temp * 9/5) + 32;
inHg = (pres * 0.00029529983071445);
  Serial.printf("Inches Of Mercury %f\nHumidity %f\n Fahrenheit %f\n",inHg,humid,Fahrenheit);
  delay(1000);
  // air sensor
 int quality = sensor.slope();
 
 Serial.print("Sensor value: ");
 Serial.println(sensor.getValue());

 if (quality == AirQualitySensor::FORCE_SIGNAL){
  Serial.println("High pollution signal active");
 }
 else if(quality == AirQualitySensor::HIGH_POLLUTION){
  Serial.println("high pollution");
 }
 else if(quality == AirQualitySensor::LOW_POLLUTION){
  Serial.println("low pollution");
 }
 else if(quality == AirQualitySensor::FRESH_AIR){
  Serial.println("fresh air");
 }

 delay(1000);




    //moisture sensor
    moistureValue = analogRead(moisturePin);
dateTime = Time.timeStr();
timeOnly = dateTime.substring(11,19);
 if( millis () - lastTime >10000) {
 lastTime = millis () ;


Serial.printf (" Date and time is %s\n", dateTime.c_str () );
 Serial.printf (" Time is %s\n", timeOnly.c_str () );
 Serial.printf("moisture reading is %0.2f",moistureValue);
  }

  //relay
  if (millis() - lastRelay > 10000) {
  lastRelay = millis();

  if (moistureValue > 3200) {
    digitalWrite(relayPin, HIGH);
    Serial.println("RELAY ON");
    delay(500); 
    digitalWrite(relayPin, LOW);
    Serial.println("RELAY OFF");
  }
}
 //manual button




    // this is our 'wait for incoming subscription packets' busy subloop 
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &subFeed) {
      subValue = atoi((char *)subFeed.lastread);
      Serial.printf("publishing %i to Adafruit.io ",subValue);
    }
    if (subValue == 1) {
  if (relayState != 1) {
    digitalWrite(relayPin, HIGH);
    Serial.println("relayState = ON");
    relayState = 1;
  }
}

if (subValue == 0) {
  if (relayState != 0) {
    digitalWrite(relayPin, LOW);
    Serial.println("relayState = OFF");
    relayState = 0;
  }
}
  }
 

   if((millis()-lastPublish > 6000)) {
    if(mqtt.Update()) {
      pubValue = sensor.getValue();
      pubValue3 = moistureValue;
      pubValue4 = Fahrenheit;
      pubValue5 = humid;
      pubValue6 = inHg;
      pubFeed.publish(pubValue);
      pubFeed3.publish(pubValue3);
      pubFeed4.publish(pubValue4);
      pubFeed5.publish(pubValue5);
      pubFeed6.publish(pubValue6);
      Serial.printf("Publishing %0.2f \n",pubValue);
      Serial.printf("Publishing %0.2f \n",pubValue3); 
      Serial.printf("Publishing %0.2f \n",pubValue4); 
      Serial.printf("Publishing %0.2f \n",pubValue5);
      Serial.printf("Publishing %0.2f \n",pubValue6);   
      } 
    lastPublish = millis();
}

  
  
}
void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}

void OLED(void){
  static int lastDisplay;
   int quality = sensor.slope();
 dateTime = Time.timeStr();
timeOnly = dateTime.substring(11,19);

if(millis() - lastDisplay > 5000){
  displayToggle = ! displayToggle;
  lastDisplay = millis();
}
if(displayToggle == true){
  display.setTextSize(0.8);
display.setTextColor(WHITE); 
  display.setCursor(10,0);
  display.clearDisplay();
  display.printf("moisture reading is %0.2f",moistureValue);
  display.printf(" Date and time is %s\n", dateTime.c_str ());
  display.printf(" Time is %s\n", timeOnly.c_str ());


   //air sensor
  display.print("Sensor value: ");
 display.println(sensor.getValue());
 if (quality == AirQualitySensor::FORCE_SIGNAL){
  display.println("High pollution signal active");
 }
 else if(quality == AirQualitySensor::HIGH_POLLUTION){
  display.println("high pollution");
 }
 else if(quality == AirQualitySensor::LOW_POLLUTION){
  display.println("low pollution");
 }
 else if(quality == AirQualitySensor::FRESH_AIR){
  display.println("fresh air");
 }
 display.display();
  display.clearDisplay();
  delay(1000);
}
else{
   //bme
    display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.printf("Fahrenheit %0.1fF\nHumidity %0.1f\nInOfMerc %0.1f\n",Fahrenheit ,inHg,humid);
  display.display();
  display.clearDisplay();
  delay(1000);}

 
}