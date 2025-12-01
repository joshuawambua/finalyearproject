
//This is the code that will power the hardware
//It will enable communication with the firebase
//It will sent sensor data to firebase
//http protocal will be used



#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Firebase Realtime Database URL (replace with your Firebase project link)
String firebaseHost = "https://your-project-id.firebaseio.com/";  
String firebaseAuth = "YOUR_FIREBASE_DATABASE_SECRET";  // optional for secured DB

// Example sensor pins
int gasSensorPin = 34;  // MQ sensor analog pin
int tempSensorPin = 35; // temperature sensor analog pin

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Read sensor values
    int gasValue = analogRead(gasSensorPin);
    int tempValue = analogRead(tempSensorPin);

    // Convert to JSON string
    String jsonData = "{";
    jsonData += "\"gas\": " + String(gasValue) + ",";
    jsonData += "\"temperature\": " + String(tempValue);
    jsonData += "}";

    // Firebase endpoint (for Realtime Database REST API)
    String url = firebaseHost + "/sensors.json";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("Data sent successfully, code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }

  delay(5000); // send data every 5 seconds
}
