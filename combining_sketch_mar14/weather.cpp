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
  WiFi.mode(WIFI_STA);
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

int getMinutesToWait() {
   if(WiFi.status()== WL_CONNECTED){
              
      sensorReadings = httpGETRequest(serverName);
      Serial.println(sensorReadings);
      JSONVar myObject = JSON.parse(sensorReadings);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
      }

      // Get an array of pops
      int pop[48]{};
      for (int i = 0; i < myObject["hourly"].length(); i++) {
        pop[i] = (int) (double(myObject["hourly"][i]["pop"]) * 100);
      }

      // Pops is 48 ints of percentages.
      // We now need to figure out whether to water or wait. Return 0 to water now, or else the number of minutes to wait. 

      // First step is to figure out when it is most likely to rain next, if at all.
      int first = -1;
      for (int i = 0; i < 48; i++) {
        if (pop[i] > 50) {
          
        }
      }

      
  } else {
      Serial.println("WiFi Disconnected");
  }
}
