#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <string.h>


//relay
#define relay D3 //PIN D3 (nodemcu) ==== pin IN (relay)

//ini wiring DHT
#define dht_dpin 2                   //PIN GPIO2 = PIN D4(nodemcu) ==== pin kedua dari kiri (DHT11)
#define DHTTYPE DHT11               //Tipe DHT

//ini wiring ultrasonic
#define triggerPin  D8 //pin triger  ke D8 ==== pin trig (ultrasonic)
#define echoPin     D7 //pin echo  ke D7 ==== pin echo (ultrasonic)

 
const char* ssid = "VillaRahayu2";
const char* password =  "bandung1";
const char* mqtt_server = "180.250.135.100"; 
const int mqtt_port = 8833;
     

DHT dht(dht_dpin, DHTTYPE);

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
 

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  dht.begin();

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
 
}
 
void loop() {
  if (!client.connected()){
    reconnect();
  }

  

  //DHT11
  float h = dht.readHumidity(); //membaca kelembaban
  float t = dht.readTemperature();  // Membaca Temperatur Dalam Celcius (Default)1
  client.publish("sensor/humi_hidro2", String(h).c_str());
  client.publish("sensor/temp_hidro2", String(t).c_str());
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" 'C "); 
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" % ");
  
  //Ultrasonic
  long duration, jarakair_hidro;

  delayMicroseconds(2); 
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarakair_hidro = (duration/2) / 29.1;
  client.publish("sensor/jarakair_hidro2", String(jarakair_hidro).c_str());
  Serial.println("jarak :");
  Serial.print(jarakair_hidro);
  Serial.println(" cm");
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
//  if(t <= 18) {
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
      client.publish("sensor/relay_hidro2",String(r).c_str());
      Serial.print("Status Pompa : ");
      Serial.println(r);
    

  Serial.print("Humi :");
  Serial.println(humi);

  Serial.print("Temp :");
  Serial.println(temp);
}
