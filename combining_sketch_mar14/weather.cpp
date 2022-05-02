#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* ssid = "Wifi-Password-OpenToAll";
const char* password = "OpenToAll";
const char* serverName = "https://api.openweathermap.org/data/2.5/onecall?lat=39.8912&lon=-74.9218&exclude=current,minutely,daily,alerts&appid=97316b5f6caa17ae73f2715a11583ab5";
unsigned long lastTime = 0;
unsigned long elapsedTime = 0;
String sensorReadings;
float sensorReadingsArr[3];

void wifiSetup() {
  WiFi.mode(WIFI_STA)
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  int i = 0;
  while(WiFi.status() != WL_CONNECTED) {
    i++;
    delay(500);
    Serial.print(".");
    if (i >= 10) {
      Serial.println("\nConnection Timeout.");
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

int[] getPops() {
   if(WiFi.status()== WL_CONNECTED){
              
      sensorReadings = httpGETRequest(serverName);
      Serial.println(sensorReadings);
      JSONVar myObject = JSON.parse(sensorReadings);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
      }

      // Get an array of pops
      int pop[myObject["hourly"].length()]{};
      for (int i = 0; i < myObject["hourly"].length(); i++) {
        pop[i] = static_cast<int>(stoi(myObject["hourly"][i]["pop"]) * 100);
      }
      
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
