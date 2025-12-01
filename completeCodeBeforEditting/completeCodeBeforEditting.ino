#include <DHT.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <HardwareSerial.h>

// DHT11 Sensor
#define DHTTYPE DHT11
#define DHTPIN 4
DHT dht(DHTPIN, DHTTYPE);

// PM Sensor (SDS011) - Hardware Serial
HardwareSerial pmSerial(2); // RX2=16, TX2=17

// MQ Sensor Analog Pins
#define MQ135_PIN 34    // GPIO34 - General Air Quality
#define MQ7_PIN 35      // GPIO35 - Carbon Monoxide  
#define MQ2_PIN 32      // GPIO32 - Combustible Gases
#define MQ131_PIN 33    // GPIO33 - Ozone

// WiFi Credentials
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

// Firebase Configuration
#define FIREBASE_HOST "your-project.firebaseio.com"
#define FIREBASE_AUTH "Your_Firebase_Secret"

FirebaseData firebaseData;

// Sensor Calibration Parameters (Adjust based on your calibration)
struct SensorCalibration {
  float temp_coeff;
  float hum_coeff;
  float base_line;
  String gas_name;
  String unit;
};

// Calibration for each MQ sensor
SensorCalibration mq135_cal = {0.015, -0.0025, 80.0, "General_Air_Quality", "PPM"};
SensorCalibration mq7_cal = {0.018, -0.0020, 10.0, "Carbon_Monoxide", "PPM"};
SensorCalibration mq2_cal = {0.020, -0.0030, 5.0, "Combustible_Gases", "PPM"};
SensorCalibration mq131_cal = {0.012, -0.0022, 20.0, "Ozone", "PPB"};

// PM Sensor calibration
float pm_humidity_coeff = 0.0025;

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize PM sensor (SDS011)
  pmSerial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  
  // Initialize analog pins for MQ sensors
  analogReadResolution(12); // 12-bit resolution for better precision
  
  // Connect to WiFi
  setupWiFi();
  
  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  Serial.println("EcoSphere Monitor Initialized");
  Serial.println("=============================");
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read environmental parameters
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Check if environmental readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }
  
  // Read all MQ sensors
  int mq135_raw = analogRead(MQ135_PIN);
  int mq7_raw = analogRead(MQ7_PIN);
  int mq2_raw = analogRead(MQ2_PIN);
  int mq131_raw = analogRead(MQ131_PIN);
  
  // Read PM sensor
  PMData pm_data = readPMSensor();
  
  // Apply compensation to all sensors
  float mq135_corrected = compensateMQSensor(mq135_raw, temperature, humidity, mq135_cal);
  float mq7_corrected = compensateMQSensor(mq7_raw, temperature, humidity, mq7_cal);
  float mq2_corrected = compensateMQSensor(mq2_raw, temperature, humidity, mq2_cal);
  float mq131_corrected = compensateMQSensor(mq131_raw, temperature, humidity, mq131_cal);
  
  // Apply compensation to PM data
  PMData pm_corrected = compensatePMSensor(pm_data, humidity);
  
  // Print readings to Serial Monitor
  printSensorReadings(temperature, humidity, mq135_corrected, mq7_corrected, 
                     mq2_corrected, mq131_corrected, pm_corrected);
  
  // Send data to Firebase
  sendToFirebase(temperature, humidity, mq135_corrected, mq7_corrected, 
                mq2_corrected, mq131_corrected, pm_corrected);
  
  // Wait 10 seconds before next reading
  delay(10000);
}

// MQ Sensor Compensation Function
float compensateMQSensor(int raw_value, float temp, float hum, SensorCalibration cal) {
  float temp_ref = 25.0;  // Reference temperature
  float hum_ref = 50.0;   // Reference humidity
  
  // Convert analog reading to voltage (ESP32: 3.3V, 12-bit = 4095)
  float voltage = (raw_value / 4095.0) * 3.3;
  
  // Apply temperature and humidity compensation
  float compensated_voltage = voltage * 
    (1 + cal.temp_coeff * (temp - temp_ref)) * 
    (1 + cal.hum_coeff * (hum - hum_ref));
  
  // Convert back to PPM (simplified - you'll need proper calibration curve)
  float ppm = (compensated_voltage / 3.3) * 1000; // Simplified conversion
  
  return ppm;
}

// PM Sensor Data Structure
struct PMData {
  float pm25;
  float pm10;
  bool valid;
};

// Read PM Sensor (SDS011)
PMData readPMSensor() {
  PMData data = {0, 0, false};
  byte buffer[32];
  int idx = 0;
  
  // Read available data from PM sensor
  while (pmSerial.available() > 0) {
    byte byteIn = pmSerial.read();
    
    // Look for start byte (0xAA)
    if (idx == 0 && byteIn == 0xAA) {
      buffer[idx++] = byteIn;
    } else if (idx > 0) {
      buffer[idx++] = byteIn;
      
      // Complete packet received (10 bytes)
      if (idx == 10) {
        if (buffer[9] == 0xAB) { // Check end byte
          // Extract PM2.5 and PM10 values
          int pm25 = (buffer[3] * 256 + buffer[2]) / 10;
          int pm10 = (buffer[5] * 256 + buffer[4]) / 10;
          
          data.pm25 = pm25;
          data.pm10 = pm10;
          data.valid = true;
        }
        idx = 0;
      }
    }
    delay(2);
  }
  
  return data;
}

// PM Sensor Compensation
PMData compensatePMSensor(PMData raw_data, float humidity) {
  PMData corrected = raw_data;
  
  if (raw_data.valid) {
    corrected.pm25 = raw_data.pm25 / (1 + pm_humidity_coeff * humidity);
    corrected.pm10 = raw_data.pm10 / (1 + pm_humidity_coeff * humidity);
  }
  
  return corrected;
}

// Print all sensor readings to Serial Monitor
void printSensorReadings(float temp, float hum, float mq135, float mq7, 
                        float mq2, float mq131, PMData pm) {
  Serial.println("=== EcoSphere Sensor Readings ===");
  Serial.printf("Environment: %.1f°C, %.1f%% RH\n", temp, hum);
  Serial.printf("MQ-135 (General): %.1f PPM\n", mq135);
  Serial.printf("MQ-7 (CO): %.1f PPM\n", mq7);
  Serial.printf("MQ-2 (Combustible): %.1f PPM\n", mq2);
  Serial.printf("MQ-131 (Ozone): %.1f PPB\n", mq131);
  
  if (pm.valid) {
    Serial.printf("PM2.5: %.1f μg/m³, PM10: %.1f μg/m³\n", pm.pm25, pm.pm10);
  } else {
    Serial.println("PM Sensor: No valid data");
  }
  Serial.println("=================================\n");
}

// Send data to Firebase
void sendToFirebase(float temp, float hum, float mq135, float mq7, 
                   float mq2, float mq131, PMData pm) {
  String timestamp = String(millis());
  
  // Create JSON object for Firebase
  FirebaseJson json;
  
  // Environment data
  json.set("environment/temperature", temp);
  json.set("environment/humidity", hum);
  json.set("environment/timestamp", timestamp);
  
  // Gas sensors data
  json.set("gases/mq135/value", mq135);
  json.set("gases/mq135/unit", "PPM");
  json.set("gases/mq7/value", mq7);
  json.set("gases/mq7/unit", "PPM");
  json.set("gases/mq2/value", mq2);
  json.set("gases/mq2/unit", "PPM");
  json.set("gases/mq131/value", mq131);
  json.set("gases/mq131/unit", "PPB");
  
  // PM data
  if (pm.valid) {
    json.set("particulate_matter/pm25", pm.pm25);
    json.set("particulate_matter/pm10", pm.pm10);
    json.set("particulate_matter/unit", "μg/m³");
  }
  
  // Push to Firebase
  if (Firebase.pushJSON(firebaseData, "/sensor_readings", json)) {
    Serial.println("Data sent to Firebase successfully");
  } else {
    Serial.println("Firebase error: " + firebaseData.errorReason());
  }
}