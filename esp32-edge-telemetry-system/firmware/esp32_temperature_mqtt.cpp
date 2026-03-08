#include <WiFi.h>          
#include <PubSubClient.h>  
#include "SPIFFS.h"        

/***************** USER CONFIGURATION *****************/

// WiFi credentials
const char* ssid = "Rjee";
const char* password = "rjee4631";

// MQTT broker settings
const char* mqtt_server = "192.168.18.41";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "hospital/pharmacy/temp";

// Deep sleep duration
#define uS_TO_S_FACTOR 1000000ULL                
#define SLEEP_TIME 60   // seconds

/********************************************************/

WiFiClient espClient;
PubSubClient mqttClient(espClient);

/***************** FUNCTION DECLARATIONS *****************/

void connectWiFi();
void connectMQTT();
float readTemperature();
void saveToQueue(String payload);
void flushQueue();
int countQueueItems();

/********************************************************/

void setup() {

  Serial.begin(115200);

  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed!");
    return;
  }

  // Connect WiFi
  connectWiFi();

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);

  // Read sensor
  float temperature = readTemperature();

  // Create JSON payload
  String payload = "{";
  payload += "\"device_id\":\"ESP32_01\",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"timestamp\":" + String((uint64_t)time(NULL));
  payload += "}";

  Serial.println("Generated Payload:");
  Serial.println(payload);

  // If WiFi connected
  if (WiFi.status() == WL_CONNECTED) {

    connectMQTT();

    if (mqttClient.connected()) {

      // Send stored offline data first
      flushQueue();

      // Send current payload
      if (mqttClient.publish(mqtt_topic, payload.c_str())) {

        Serial.println("Current payload sent successfully!");

      } else {

        Serial.println("Publish failed. Saving locally.");
        saveToQueue(payload);

      }

    } else {

      Serial.println("MQTT not connected. Saving locally.");
      saveToQueue(payload);

    }

  } else {

    Serial.println("WiFi not connected. Saving locally.");
    saveToQueue(payload);

  }
  Serial.printf("Total: %d bytes\n", SPIFFS.totalBytes());
  Serial.printf("Used: %d bytes\n", SPIFFS.usedBytes());
  delay(200);
  // Deep Sleep
  Serial.println("Going to deep sleep...");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

/********************************************************/

void loop() {
}

/********************************************************/

void connectWiFi() {

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int retry = 0;

  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("\nWiFi Connection Failed!");

  }
}

/********************************************************/

void connectMQTT() {

  Serial.println("Connecting to MQTT...");

  int retry = 0;

  while (!mqttClient.connected() && retry < 5) {

    if (mqttClient.connect("ESP32_Client_01")) {

      Serial.println("MQTT Connected!");

    } else {

      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying...");
      delay(2000);
      retry++;

    }
  }
}

/********************************************************/

float readTemperature() {

  // Simulated temperature
  float temp = random(200, 300) / 10.0;  // 20.0 – 30.0 °C
  return temp;

}

/********************************************************/

void saveToQueue(String payload) {

  File file = SPIFFS.open("/queue.txt", FILE_APPEND);

  if (!file) {
    Serial.println("Failed to open queue file!");
    return;
  }

  file.println(payload);
  file.close();

  Serial.println("Payload saved to flash queue.");

  int total = countQueueItems();

  Serial.print("Total records stored in SPIFFS queue: ");
  Serial.println(total);
}

/********************************************************/

void flushQueue() {

  File file = SPIFFS.open("/queue.txt", FILE_READ);

  if (!file) {

    Serial.println("No queued data found.");
    return;

  }

  int total = countQueueItems();

  Serial.print("Stored records before flush: ");
  Serial.println(total);

  Serial.println("Flushing stored data...");

  while (file.available()) {

    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {

      mqttClient.publish(mqtt_topic, line.c_str());
      Serial.println("Sent stored payload:");
      Serial.println(line);

      delay(200);
    }
  }

  file.close();

  // Clear queue after sending
  SPIFFS.remove("/queue.txt");

  Serial.println("Queue cleared.");
}

/********************************************************/

int countQueueItems() {

  File file = SPIFFS.open("/queue.txt", FILE_READ);

  if (!file) {
    return 0;
  }

  int count = 0; 

  while (file.available()) {

    String line = file.readStringUntil('\n');

    if (line.length() > 5) {
      count++;
    }

  }

  file.close();

  return count;
}