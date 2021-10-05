#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <RTClib.h>
#include <Wire.h>
#include <string.h>
#include <DFPlayer_Mini_Mp3.h>
#include <SoftwareSerial.h>




#define PIN_BUSY D5
 
RTC_DS3231 rtc;
//wiring DS3231;
//SDA ==> D1
//SCL ==> D2

char t[32];


//wiring yang buat musik
SoftwareSerial mp3Serial(D3, D4); // RX, TX


//relay
#define relay D0 //PIN D0 (nodemcu) ==== pin IN (relay)


//ini wiring DHT
#define DHTPIN D6                   //PIN GPIO2 = PIN D4(nodemcu) ==== pin kedua dari kiri (DHT11)
#define DHTTYPE DHT11               //Tipe DHT
DHT dht (DHTPIN,DHTTYPE);     

//ini wiring ultrasonic
#define triggerPin  D8 //pin triger  ke D8 ==== pin trig (ultrasonic)
#define echoPin     D7 //pin echo  ke D7 ==== pin echo (ultrasonic)

 
const char* ssid = "VillaRahayu2";
const char* password =  "bandung1";
const char* mqtt_server = "180.250.135.100"; 
const int mqtt_port = 8833;


WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //rtc

  Wire.begin(5, 4);   //Setting wire 5 untuk SDA dan 4 untuk SCL
 
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));  //Setting Time
//   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  pinMode(PIN_BUSY, INPUT);
  Serial.begin (9600);
  Serial.println("Setting up software serial");
  mp3Serial.begin (9600);
  Serial.println("Setting up mp3 player");
  mp3_set_serial (mp3Serial);  
  // Delay is required before accessing player. From my experience it's ~1 sec
  delay(1000); 
  mp3_set_volume (90);
  
  //dht
  dht.begin();
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  pinMode(relay, OUTPUT);

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
}


void callback(String topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    Serial.println("");
    messageTemp += (char)payload[i];
    Serial.print(" message temp : ");
    Serial.print(messageTemp);
  }

  //Serial.println("Musik : " + musik);
  if(String(topic)=="topic/jam1"){
    if(messageTemp == "ON"){
       Serial.println("its work");
       mp3_next ();
  } else  {
    Serial.print ("Belum Waktunya");
    delay(10);
    }

  delay(500);
   Serial.println();
 }
}
  

 
void loop() {
  if (!client.connected()){
    reconnect();
  }

  client.loop();
  client.connect("ESP8266Client");
   

  //DHT11
  float h = dht.readHumidity(); //membaca kelembaban
  float t = dht.readTemperature();  // Membaca Temperatur Dalam Celcius (Default)1
  client.publish("sensor/humi_hidro", String(h).c_str());
  client.publish("sensor/temp_hidro", String(t).c_str());
  //Serial.print("Temperature: ");
  //Serial.print(t);
  //Serial.println(" 'C "); 
  //Serial.print("Humidity: ");
  //Serial.print(h);
  //Serial.println(" % ");
  
  //Ultrasonic
  long duration, jarakair_hidro;

  delayMicroseconds(2); 
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarakair_hidro = (duration/2) / 29.1;
  client.publish("sensor/jarakair_hidro", String(jarakair_hidro).c_str());
  //Serial.print("jarak :");
 // Serial.print(jarakair_hidro);
  //Serial.println(" cm");
  client.loop();
  delay(1000);


  //humidity
//  String humi;
//
//  if (h <= 30) {
//    humi = "kering";
//    }
//
//  else if (h > 30 && h <70) {
//    humi = "sedang";
//    } 
//
//  else if (h >= 70) {
//    humi = "basah";
//    }
//
//
////  //temperature
//  String temp;
//
//  if(t < 19) {
//   temp = "dingin";
//    }
//
//   else if(t > 18 && t <= 28) {
//   temp = "normal";
//    }
//
//   else if(t > 28) {
//    temp = "panas";
//    }



    //else if gabungan dari humidity dan temperature

    String r;
    if(t >= 19 && h >= 80) {
      digitalWrite(relay, HIGH);
      r = "ON";
      
      }

    else if (t < 19 && h < 80) {
      digitalWrite(relay, HIGH);
      r = "OFF";
       
      }
      client.publish("sensor/relay_hidro",String(r).c_str());
      Serial.print("Status Pompa : ");
      Serial.println(r);
    



  DateTime now = rtc.now();       
 
  int jam     = now.hour();
  int menit   = now.minute();
  int detik   = now.second(); 
  client.publish("time/hour", String(jam).c_str());
  client.publish("time/minute", String(menit).c_str());

}



  void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("topic/jam1");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
