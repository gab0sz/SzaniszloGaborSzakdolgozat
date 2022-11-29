#include <Arduino.h>
#include <FastLED.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "gabosz"
#define WIFI_PASSWORD "11112222"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDVB7yYwlxqBU92zOviE1zvpOfTZiyfPkI"

// Insert Authorized Username and Corresponding Password
#define USER_EMAIL "EMAIL"
#define USER_PASSWORD "PASSWORD"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://fir-esp32-test-eadba-default-rtdb.europe-west1.firebasedatabase.app"

// Define Firebase objects
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
FirebaseData fbdo;
// Variables to save database paths
String listenerPath;
String parentPath;
String uid;

#define DATA_PIN 12
#define NUM_LEDS 3
#define LED_TYPE WS2812B


CRGB leds[NUM_LEDS];
int color = 0;
int r = 0;
int g = 0;
int b = 0;


void initFastLED() {
  FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
}

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print('.');
    //delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Callback function that runs on database changes
void streamCallback(FirebaseStream data){
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println();

  // Get the path that triggered the function
  String streamPath = String(data.dataPath());

  // if the data returned is an integer, there was a change on the GPIO state on the following path /{gpio_number}
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string){
    if (data.dataPath() == "/rgb"){
      String rgb = data.stringData();
      
      String red = rgb.substring(1, 3);
      String green = rgb.substring(3,5);
      String blue = rgb.substring(5,7);
      r = strtol(red.c_str(), NULL, 16);
      g = strtol(green.c_str(), NULL, 16);
      b = strtol(blue.c_str(), NULL, 16);
      Serial.println(red);
      Serial.println(green);
      Serial.println(blue);
      Serial.println(r);
      Serial.println(g);
      Serial.println(b);
      FastLED.clear();
      //leds[0] = CRGB(r, g, b);
    }
    
  }
  
  /* When it first runs, it is triggered on the root (/) path and returns a JSON with all keys
  and values of that path. So, we can get all values from the database and updated the GPIO states*/
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json){
    FirebaseJson json = data.to<FirebaseJson>();

    // To iterate all values in Json object
    size_t count = json.iteratorBegin();
    Serial.println("\n---------");
    for (size_t i = 0; i < count; i++){
        
        FirebaseJson::IteratorValue value = json.valueAt(i);
        Serial.println(value.key + " : " + value.value);
        if(value.key == "rgb"){
          
          String rgb = value.value;
      
          String red = rgb.substring(2, 4);
          String green = rgb.substring(4,6);
          String blue = rgb.substring(6,8);
          r = strtol(red.c_str(), NULL, 16);
          g = strtol(green.c_str(), NULL, 16);
          b = strtol(blue.c_str(), NULL, 16);
          Serial.println(red);
          Serial.println(green);
          Serial.println(blue);
          Serial.println(r);
          Serial.println(g);
          Serial.println(b);
          FastLED.clear();
          //leds[0] = CRGB(r, g, b);
        }
        
        
        Serial.printf("Name: %s, Value: %s, Type: %s\n", value.key.c_str(), value.value.c_str(), value.type == FirebaseJson::JSON_OBJECT ? "object" : "array");
    }
    Serial.println();
    json.iteratorEnd(); // required for free the used memory in iteration (node data collection)
  }
  
  //This is the size of stream payload received (current and max value)
  //Max payload size is the payload size under the stream path since the stream connected
  //and read once and will not update until stream reconnection takes place.
  //This max value will be zero as no payload received in case of ESP8266 which
  //BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

void streamTimeoutCallback(bool timeout){
  if (timeout)
    Serial.println("stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup(){
  Serial.begin(9600);
  initWiFi();
  
  initFastLED();
  // Initialize Outputs
  
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);
  while ((auth.token.uid) == "") {
    Serial.print(';');
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  listenerPath = "/UsersData/" + uid + "/rgbled/";
  parentPath = "/UsersData/" + uid + "/rgbled/";
  // Streaming (whenever data changes on a path)
  // Begin stream on a database path --> board1/outputs/digital
  if (!Firebase.RTDB.beginStream(&stream, listenerPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

  // Assign a calback function to run when it detects changes on the database
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  //delay(2000);
}

void loop(){
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
  leds[0] = CRGB(g, r, b);
  FastLED.show();
}
