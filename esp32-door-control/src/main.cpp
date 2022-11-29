#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <MFRC522.h>
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
int sendTime = 0;
bool isSent = false;
/*declare inputs - 3.3V - red
                   GND - black
                   SS - green
                   SCK - orange
                   MOSI - yellow
                   MISO - blue
                   IRQ -nothing
                   RST - brown
*/

//const int SS = 5;
const int SC = 18;
//const int MOSI = 23;
//const int MISO = 19;
const int RST = 27;

MFRC522 rfid(SS, RST);

// Declare outputs
const int relayPin = 16;
const int motionSensor = 17;
int rfidState = 0;

byte allowedCard[] = { 0x83, 0x18, 0x0B, 0x1E };


//initialize the RFID reader
void initRFID() {
  SPI.begin();
  rfid.PCD_Init();
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
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer){
    if (data.dataPath() == "/rfid"){
      rfidState = data.intData();
      Serial.println("RFID state: " + String(rfidState));
    }
    else if (data.dataPath() == "/status"){
      int state = data.intData();
      Serial.print("STATE: ");
      Serial.println(state);
      digitalWrite(relayPin, state);
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
        if(value.key == "rfid"){
          rfidState = value.value.toInt();
          Serial.println("RFID state: " + String(rfidState));
        }
        else if(value.key == "status"){
          int state = value.value.toInt();
          Serial.print("STATE: ");
          Serial.println(state);
          digitalWrite(relayPin, state);
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
  initRFID();
  // Initialize Outputs
  pinMode(relayPin, OUTPUT);
  pinMode(13, OUTPUT);
  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  
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
  listenerPath = "/UsersData/" + uid + "/door/";
  parentPath = "/UsersData/" + uid + "/door/";
  // Streaming (whenever data changes on a path)
  // Begin stream on a database path --> board1/outputs/digital
  if (!Firebase.RTDB.beginStream(&stream, listenerPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

  // Assign a calback function to run when it detects changes on the database
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  //delay(2000);
}

void loop(){
  digitalWrite(13, digitalRead(motionSensor));
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
  if (rfidState == 1){
    if (rfid.PICC_IsNewCardPresent()) { // new tag is available
      if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
        Serial.print("RFID/NFC Tag Type: ");
        Serial.println(rfid.PICC_GetTypeName(piccType));
        bool isAllowed = true;
        // print UID in Serial Monitor in the hex format
        Serial.print("UID:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);

          if (rfid.uid.uidByte[i] != allowedCard[i])
          {
            Serial.println("Access denied");
            isAllowed = false;
            break;
          }
        }
        Serial.println();
        if (isAllowed)
        {
          Serial.println("Access granted");
          //digitalWrite(relayPin, HIGH);
          json.set("/rfid", 1);
          json.set("/status", 1);
          
          Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
          sendTime = millis();
          isSent = true;
          // Firebase.RTDB.setString(&config, "/UsersData/" + uid + "/door", "0");
          
        }
        

        rfid.PICC_HaltA(); // halt PICC
        rfid.PCD_StopCrypto1(); // stop encryption on PCD
      }
      
    }
    if (isSent){
        if (millis() - sendTime > 2000){
          sendTime = millis();
          //digitalWrite(relayPin, LOW);
          json.set("/rfid", 1);
          json.set("/status", 0);
          Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
          isSent = false;
        }
      }
  }
}
