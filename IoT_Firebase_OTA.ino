/*Developed by Marco Andrade & Jesus Gomez @ MYADTECH.MX
 *Please feel free to modify this code according to your needs.
 *Keep it free and enjoy it!
 *If need custom solution email us: contacto@myadtech.mx
 *Feel free to tell us a requierment and we will make it Open Source!
*/

#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define FIREBASE_HOST "project.firebaseio.com" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "secret-key"

void SetupPins();
bool IsConnected();
void InitializeWiFi();
void InitializeFirebase();
bool IsConnected();
void SendToServer();
void printResult(FirebaseData);

const char* ssid     = "SSID";
const char* password = "SSID_KEY";

//If you want to add hardware End Unique Identifier
const char* hardware_eui = "";

FirebaseJson json1;
FirebaseData firebaseData;

//Path to write data to
String path = "/sensors/";

//Path to read data from
String sensorPath = "/sensors/temperature";

String jsonStr = "";
FirebaseJsonData jsonObj;

int temperature;
bool sentToServer = false;
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  InitializeWiFi();
  InitializeFirebase();
  SendToServer();

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
}

void loop() {
  ArduinoOTA.handle();
  // put your main code here, to run repeatedly:
  
}

void SetupPins(){
  //Example pins
  pinMode(2,INPUT);
  pinMode(5,OUTPUT);
}

void InitializeWiFi(){
  Serial.println(F("Starting WiFi"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to:\t");
  Serial.println(ssid); 
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
 
  Serial.println();
  Serial.print("Connected to:\t");
  Serial.println(WiFi.SSID()); 
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

}

void InitializeFirebase(){
  //Set device path in Firebase, this allows to handle multiple devices by changing hardware_eui
  path = path + hardware_eui;
  //Start Firebase on device path
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}
 
bool IsConnected(){
  if(WiFi.status()== WL_CONNECTED){   
    return true;
  }else{
    Serial.println("Error in WiFi connection");   
    return false;
  }
}

void SendToServer(){
  String jsonStr = ""; 
  FirebaseJsonData jsonObj;

  //Save data to JSON format
  json1.set("temperature", temperature);

  //Over-write data in realtime
  Firebase.updateNode(firebaseData, path, json1);
  Serial.println("Data sent to server");
}

void printResult(FirebaseData &data)
{
    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        Serial.println();
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == JSON_OBJECT ? "object" : "array");
            if (type == JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        Serial.println();
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == JSON_STRING ||
                     jsonData.typeNum == JSON_NULL ||
                     jsonData.typeNum == JSON_OBJECT ||
                     jsonData.typeNum == JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
}
