#include "EspMQTTClient.h"
#include <ArduinoJson.h>
#include "ConnectDataCasa.h"

#define DATA_INTERVAL 1000       // Intervalo para adquirir novos dados do sensor (milisegundos).
                                 // Os dados serão publidados depois de serem adquiridos valores equivalentes a janela do filtro
#define AVAILABLE_INTERVAL 5000  // Intervalo para enviar sinais de available (milisegundos)
#define LED_INTERVAL_MQTT 1000        // Intervalo para piscar o LED quando conectado no broker
#define JANELA_FILTRO 10         // Número de amostras do filtro para realizar a média

byte TRIG_PIN = 14;
byte ECHO_PIN = 13;


unsigned long dataIntevalPrevTime = 0;      // will store last time data was send
unsigned long availableIntevalPrevTime = 0; // will store last time "available" was send


void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input
  
  // Optional functionalities of EspMQTTClient
  //client.enableMQTTPersistence();
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
  //client.setKeepAlive(8);
  WiFi.mode(WIFI_STA);
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{ 
  availableSignal();
}

void availableSignal(){
  client.publish(topic_name + "/available", "online"); 
}

float readSensor(){
  const float SOUND_VELOCITY = 0.034;
  const float CM_TO_INCH     = 0.393701;

  long duration;
  float distanceCm;

  // Clears the trigPin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;

  return distanceCm;
}

void metodoPublisher(){
  static unsigned int amostras = 0;  //variável para realizar o filtro de média
  static float acumulador = 0;       //variável para acumular a média
  
  acumulador += readSensor();

  //realização de média
  if (amostras >= JANELA_FILTRO){
    StaticJsonDocument<300> jsonDoc;
  
    jsonDoc["RSSI"] = WiFi.RSSI();
    jsonDoc["nivel"] = acumulador/JANELA_FILTRO;
  
    String payload = "";
    serializeJson(jsonDoc, payload);

    client.publish(topic_name, payload); 
    amostras = 0; 
    acumulador = 0;
  }
  amostras++;  
}

void blinkLed(){
  static unsigned long ledWifiPrevTime = 0;
  static unsigned long ledMqttPrevTime = 0;
  unsigned long time_ms = millis();
  bool ledStatus = false;
  
  if( (WiFi.status() == WL_CONNECTED)){
    if(client.isMqttConnected()){
      if( (time_ms-ledMqttPrevTime) >= LED_INTERVAL_MQTT){
        ledStatus = !digitalRead(LED_BUILTIN);
        digitalWrite(LED_BUILTIN, ledStatus);
        ledMqttPrevTime = time_ms;
      }      
    }
    else{
      digitalWrite(LED_BUILTIN, LOW); //liga led
    }
  }
  else{
    digitalWrite(LED_BUILTIN, HIGH); //desliga led
  }
}

void loop()
{
  unsigned long time_ms = millis();

  client.loop();

  if (time_ms - dataIntevalPrevTime >= DATA_INTERVAL) {
    client.executeDelayed(1 * 100, metodoPublisher);  
    dataIntevalPrevTime = time_ms;
  }

  if (time_ms - availableIntevalPrevTime >= AVAILABLE_INTERVAL) {
    client.executeDelayed(1 * 500, availableSignal);
    availableIntevalPrevTime = time_ms;
  }

  blinkLed();

}
