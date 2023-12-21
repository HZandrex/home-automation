// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

// Replace with your network credentials
const char* ssid     = "";
const char* password = "";
//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.0.186/api/v1/data";

ESP8266WebServer server(8080);

const int portPin = 0;
ESP8266WebServer serverPort(80);

void setup() {
  Serial.begin(115200);
  pinMode(portPin, OUTPUT);
  digitalWrite(portPin, LOW);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  serverPort.on("/", HTTP_GET, []() {
    digitalWrite(portPin, HIGH);
    delay(500);
    digitalWrite(portPin, LOW);
    serverPort.send(200, "text/plain", "{status: true}");
  });

  server.on("/", HTTP_GET, []() {
    // Send an HTTP GET request to the API endpoint
    String sensorReadings = httpGETRequest(serverName);
    
    // Parse the API response
    const size_t capacity = JSON_OBJECT_SIZE(6) + JSON_ARRAY_SIZE(1) + 20 * JSON_OBJECT_SIZE(6) + 600; // Adjust the capacity as needed
    DynamicJsonDocument jsonBuffer(capacity);
    DeserializationError error = deserializeJson(jsonBuffer, sensorReadings);
    
    String responseText;
    
    if (error) {
      responseText = "Failed to parse API response";
    } else {
      // Access the specific value from the API response JSON
      String apiValue = jsonBuffer["active_power_w"].as<String>(); // Replace "active_power_w" with the actual key for the value you want
      responseText = "<!DOCTYPE html>\n";
      responseText += "<html>\n<head>\n";
      responseText += "<meta http-equiv='refresh' content='3'>\n"; // Refresh the page every 3 seconds
      responseText += "<style>\n";
      responseText += "body { background-color: white; font-size: 65px; font-family: Trebuchet MS;}\n";
      responseText += "</style>\n";
      responseText += "</head>\n<body>\n";
      responseText += "<h1>Huidig verbruik:</h1>\n";
      responseText += "<h1>" + apiValue + "</h1>\n";
      responseText += "</body>\n</html>";
    }
    
    server.send(200, "text/html", responseText);
  });

  server.begin();
  Serial.println("HTTP server started on port 8080");
  serverPort.begin();
  Serial.println("HTTP server started on port 80");
}

void loop() {
  server.handleClient();
  serverPort.handleClient();
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
