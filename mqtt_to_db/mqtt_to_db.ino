#include <WiFi.h>  
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const int DHT_PIN = 26;  
const char* ssid = "Maintenance"; ///  wifi ssid 
const char* password = "1nyaduax";
const char* mqtt_server = "192.168.43.198";// mosquitto server url

DHT sensor_dht(DHT_PIN,DHT22);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

StaticJsonDocument<100> sensor_doc;
char buffer[100];

void setup_wifi() { 
  delay(10);
  Serial.println();
  Serial.print("Wifi terkoneksi ke : ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi berhasil terkoneksi");
  Serial.print("Alamat IP : ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]);
  }}

void reconnect() { 
  while (!client.connected()) {
    Serial.print("Baru melakukan koneksi MQTT ...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }}
}

void setup() {
  //pinMode(2, OUTPUT);     
  Serial.begin(115200);
  sensor_dht.begin();
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 

}

void loop() {
  delay(100);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) { //perintah publish data
    lastMsg = now;
    
    float temp = sensor_dht.readTemperature();
    float hum = sensor_dht.readHumidity();

    // node per data    
    char temp_str[8];
    dtostrf(temp, 1, 2, temp_str);
    client.publish("/topic/temperature", temp_str);
    char hum_str[8];
    dtostrf(hum, 1, 2, hum_str);
    client.publish("/topic/humidity", hum_str);

    // node combined json
    sensor_doc["temperature"] = temp;
    sensor_doc["humidity"] = hum;

    serializeJson(sensor_doc, buffer);
    // Serial.println(buffer);

    client.publish("/topic/dataset", buffer);

    // Serial.print("Temperature: ");
    // Serial.println(temp);
    // Serial.print("Humidity: ");
    // Serial.println(hum);
  }
}