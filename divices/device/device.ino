#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Dew T"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyD0wYYVfaBvSaktiXMQ0tCc2vWdxmAS2k8"
#define DATABASE_URL "https://badminton-h-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define SENSOR_PIN D2
#define WIFI_LED D3
#define SENSOR_LED D4

#define USER_EMAIL "admin@gmail.com"
#define USER_PASSWORD "admin123"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);

void setup() {
  pinMode(SENSOR_LED, OUTPUT);
  pinMode(WIFI_LED, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(WIFI_LED, LOW);
    Serial.print(".");
    delay(300);
  }
  
  digitalWrite(WIFI_LED, HIGH);
  Serial.println("\nConnected with IP: " + WiFi.localIP().toString());

  timeClient.begin();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
 

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void loop() {
  if (Firebase.ready()) {
    timeClient.update();

    if (!detectWave()) {
      blinkSensorLED();

      unsigned long currentMillis = timeClient.getEpochTime();
      String timestamp = String(currentMillis);

      String path = "ir_test/unit7";
      if (Firebase.RTDB.setString(&fbdo, path.c_str(), timestamp.c_str())) {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
      } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
    
    digitalWrite(SENSOR_LED, LOW);
    delay(100);
  }
}

bool detectWave() {
  return digitalRead(SENSOR_PIN);
}

void blinkSensorLED() {
  digitalWrite(SENSOR_LED, HIGH);
  delay(500);
  digitalWrite(SENSOR_LED, LOW);
}
