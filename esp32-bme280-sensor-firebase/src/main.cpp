#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "gabosz"
#define WIFI_PASSWORD "11112222"

#define API_KEY "AIzaSyDVB7yYwlxqBU92zOviE1zvpOfTZiyfPkI"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "EMAIL"
#define USER_PASSWORD "PASSWORD"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://fir-esp32-test-eadba-default-rtdb.europe-west1.firebasedatabase.app"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
String latestDataPath;
// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String presPath = "/pressure";
String timePath = "/timestamp";
// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "hu.pool.ntp.org";
/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

//sda scl pins: 21 22

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
float temperature;
float pressure;

unsigned long delayTime;
// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 900000;

void printValues();

// Initialize the sensor
void initBME280()
{
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

void initFirebase() {
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;
  Serial.println("Firebase Data Object Ready");
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Reconnect WiFi");
  fbdo.setResponseSize(4096);
  Serial.println("Firebase Response Size");
  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Serial.println("Firebase Token Status Callback");
  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;
  Serial.println("Firebase Max Token Generation Retry");
  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);
  Serial.println("Firebase Begin");
  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
  latestDataPath = databasePath + "/latest";
}

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("BME280 sensor update firebase database"));

  Serial.println("Init BME280");
  initBME280();

  Serial.println("Init WiFi");
  initWiFi();

  Serial.println("Init NTP");
  configTime(0, 0, ntpServer);

  Serial.println("Init Firebase");
  initFirebase();

  
  
  delayTime = 1000;

  Serial.println();
}


void loop() { 
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath = databasePath + "/" + String(timestamp);

    json.set(tempPath.c_str(), String(bme.readTemperature()));
    json.set(presPath.c_str(), String(bme.readPressure()/100.0F));
    json.set(humPath.c_str(), String(bme.readHumidity()));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    parentPath = latestDataPath;
    json.set(tempPath.c_str(), String(bme.readTemperature()));
    json.set(presPath.c_str(), String(bme.readPressure()/100.0F));
    json.set(humPath.c_str(), String(bme.readHumidity()));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");


  Serial.println();
}
