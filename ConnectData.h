const String topic_name = "/medidor_nivel";


EspMQTTClient client(
  "SSID",
  "WIFI_PASSWORD",
  "IP_BROKER_ADDRES",  // MQTT Broker server ip
  "BROKER_LOGIN",   // Can be omitted if not needed
  "BROKER_PASSWORD",   // Can be omitted if not needed
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);
