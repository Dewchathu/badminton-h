#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Dew T"
#define WIFI_PASSWORD "12345678"

// Insert Firebase project API Key
#define API_KEY "AIzaSyD0wYYVfaBvSaktiXMQ0tCc2vWdxmAS2k8"

// Insert RTDB URL
#define DATABASE_URL "https://badminton-h-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define SENSOR_PIN D2
#define WIFI_LED D3
#define SENSOR_LED D4

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

// NTP settings
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);

void setup() {
  pinMode(SENSOR_LED,OUTPUT);
  pinMode(WIFI_LED,OUTPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(WIFI_LED,LOW);
    Serial.print(".");
    delay(300);
  }
  digitalWrite(WIFI_LED,HIGH);
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize NTP
  timeClient.begin();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long-running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK) {
    // Update the time from NTP server
    timeClient.update();

    if (!detectWave()) {

      digitalWrite(SENSOR_LED,HIGH);
      delay(500);
      digitalWrite(SENSOR_LED,LOW);
      // Get the current time in milliseconds
      unsigned long currentMillis = timeClient.getEpochTime();

      // Format the timestamp as a string
      String timestamp = String(currentMillis);

      Serial.println("Timestamp: " + timestamp);

      if (Firebase.RTDB.setString(&fbdo, "ir_test/unit7/timestamp", timestamp)) {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
      } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
    digitalWrite(SENSOR_LED,LOW);

    // Add other tasks or delay as needed
    delay(100);
  }
}

bool detectWave() {
  return digitalRead(SENSOR_PIN);
}
