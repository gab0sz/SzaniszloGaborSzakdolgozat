#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <MQUnifiedsensor.h>
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

#define RatioMQ135CleanAir 3.6

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
String latestDataPath;
String mq135path = "/mq135";
String sa17path = "/sa17";
String parentPath;


int mq135Pin = 34;
int sa17Pin = 35;

MQUnifiedsensor MQ135("ESP32", 3.3, 12, mq135Pin, "MQ135");

void setup() {

  Serial.begin(9600);
  initWiFi();
  initFirebase();
  
  MQ135.setRegressionMethod(1);
  MQ135.setA(110.47);
  MQ135.setB(-2.862);
  MQ135.init();

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the esp will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Connection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Connection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  
  MQ135.serialDebug(true);

}

void loop() {
  MQ135.update();
  
  float mq135reading = MQ135.readSensor();
  if (mq135reading > 1000.0) {
    Serial.println("MQ135");
  }
  int sa17reading = digitalRead(sa17Pin);
  if (sa17reading == HIGH) {
    
    Serial.println("SA17");
  }

  json.set(mq135path.c_str(), String(mq135reading));
  json.set(sa17path.c_str(), String(sa17reading));
  Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
}


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
  parentPath = "/UsersData/" + uid + "/safetysensors";
  
}