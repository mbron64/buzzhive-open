/**
 * Buzzhive Base Station Firmware
 * 
 * Receives data from hive sensors via LoRa, runs ML inference,
 * and uploads results to cloud via WiFi.
 * 
 * Hardware:
 * - ESP32 (any variant)
 * - SX1276 LoRa Module
 * - WiFi connection
 * 
 * License: Apache 2.0
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "config.h"
#include "xgboost_inference.h"

// ============================================================================
// Configuration - CHANGE THESE FOR YOUR SETUP
// ============================================================================

// WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// API endpoint for cloud upload
const char* API_ENDPOINT = "https://api.buzzhive.io/v1/telemetry";
const char* API_KEY = "YOUR_API_KEY";  // Get from buzzhive.io dashboard

// LoRa pins
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

// Status LED
#define LED_PIN 2

// ============================================================================
// Data Structures
// ============================================================================

// Must match hive sensor packet structure
struct __attribute__((packed)) BuzzhivePacket {
    uint8_t hiveId;
    uint8_t queenStatus;
    uint8_t anomalyScore;
    int16_t temperature;
    uint8_t humidity;
    uint16_t batteryMv;
    uint32_t timestamp;
    uint8_t featureHash[4];
};

// Extended packet with MFCC features (for ML inference)
struct __attribute__((packed)) BuzzhivePacketFull {
    uint8_t hiveId;
    int16_t temperature;
    uint8_t humidity;
    uint16_t batteryMv;
    float mfccFeatures[78];  // Full MFCC features for inference
};

// ============================================================================
// Global Variables
// ============================================================================

WiFiClient wifiClient;
HTTPClient http;
bool wifiConnected = false;

// Status names for display
const char* QUEEN_STATUS_NAMES[] = {
    "Queenright",
    "Queenless",
    "Queen_Hatched", 
    "Queen_Accepted"
};

// ============================================================================
// WiFi Setup
// ============================================================================

void setupWiFi() {
    Serial.printf("📶 Connecting to WiFi: %s", WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.printf("\n✅ Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n⚠️ WiFi connection failed - will retry later");
    }
}

// ============================================================================
// LoRa Setup
// ============================================================================

void setupLoRa() {
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    
    if (!LoRa.begin(915E6)) {  // Match hive sensor frequency
        Serial.println("❌ LoRa init failed!");
        while (1);
    }
    
    // Match hive sensor settings
    LoRa.setSpreadingFactor(10);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    
    Serial.println("✅ LoRa initialized - listening for hive sensors");
}

// ============================================================================
// ML Inference
// ============================================================================

uint8_t runInference(const float* features) {
    // Normalize features using stored scaler parameters
    float normalizedFeatures[78];
    normalizeFeatures(features, normalizedFeatures);
    
    // Run XGBoost inference
    float scores[4];
    xgboostPredict(normalizedFeatures, scores);
    
    // Find class with highest score
    uint8_t bestClass = 0;
    float bestScore = scores[0];
    for (int i = 1; i < 4; i++) {
        if (scores[i] > bestScore) {
            bestScore = scores[i];
            bestClass = i;
        }
    }
    
    Serial.printf("🧠 ML Inference: %s (confidence: %.2f)\n", 
                  QUEEN_STATUS_NAMES[bestClass], bestScore);
    
    return bestClass;
}

// ============================================================================
// Cloud Upload
// ============================================================================

bool uploadToCloud(uint8_t hiveId, uint8_t queenStatus, uint8_t anomalyScore,
                   float temperature, uint8_t humidity, uint16_t batteryMv) {
    if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi not connected, skipping upload");
        return false;
    }
    
    // Build JSON payload
    StaticJsonDocument<512> doc;
    doc["hive_id"] = hiveId;
    doc["queen_status"] = queenStatus;
    doc["queen_status_name"] = QUEEN_STATUS_NAMES[queenStatus];
    doc["anomaly_score"] = anomalyScore;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["battery_mv"] = batteryMv;
    doc["timestamp"] = millis();
    
    String payload;
    serializeJson(doc, payload);
    
    // Send to API
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", API_KEY);
    
    int httpCode = http.POST(payload);
    
    if (httpCode == 200 || httpCode == 201) {
        Serial.println("☁️ Uploaded to cloud successfully");
        return true;
    } else {
        Serial.printf("❌ Upload failed: HTTP %d\n", httpCode);
        return false;
    }
    
    http.end();
}

// ============================================================================
// LoRa Packet Processing
// ============================================================================

void processPacket(int packetSize) {
    if (packetSize == sizeof(BuzzhivePacket)) {
        // Simple packet (already classified by hive sensor)
        BuzzhivePacket packet;
        LoRa.readBytes((uint8_t*)&packet, sizeof(packet));
        
        float temp = packet.temperature / 100.0;
        
        Serial.printf("\n📥 Received from Hive %d:\n", packet.hiveId);
        Serial.printf("   Queen Status: %s\n", QUEEN_STATUS_NAMES[packet.queenStatus]);
        Serial.printf("   Anomaly Score: %d\n", packet.anomalyScore);
        Serial.printf("   Temperature: %.1f°C\n", temp);
        Serial.printf("   Humidity: %d%%\n", packet.humidity);
        Serial.printf("   Battery: %d mV\n", packet.batteryMv);
        Serial.printf("   RSSI: %d dBm\n", LoRa.packetRssi());
        
        // Upload to cloud
        uploadToCloud(packet.hiveId, packet.queenStatus, packet.anomalyScore,
                      temp, packet.humidity, packet.batteryMv);
        
        // Blink LED to indicate received packet
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        
    } else if (packetSize == sizeof(BuzzhivePacketFull)) {
        // Full packet with MFCC features - run inference here
        BuzzhivePacketFull packet;
        LoRa.readBytes((uint8_t*)&packet, sizeof(packet));
        
        Serial.printf("\n📥 Received MFCC data from Hive %d\n", packet.hiveId);
        
        // Run ML inference on base station
        uint8_t queenStatus = runInference(packet.mfccFeatures);
        uint8_t anomalyScore = 0;  // TODO: Add VAE inference
        
        float temp = packet.temperature / 100.0;
        
        // Upload to cloud
        uploadToCloud(packet.hiveId, queenStatus, anomalyScore,
                      temp, packet.humidity, packet.batteryMv);
        
        // Blink LED
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(50);
            digitalWrite(LED_PIN, LOW);
            delay(50);
        }
        
    } else {
        Serial.printf("⚠️ Unknown packet size: %d bytes\n", packetSize);
    }
}

// ============================================================================
// Main Setup & Loop
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("\n🐝 Buzzhive Base Station v1.0");
    Serial.println("================================");
    
    setupWiFi();
    setupLoRa();
    
    Serial.println("\n✅ Ready! Waiting for hive sensor data...\n");
}

void loop() {
    // Check for incoming LoRa packets
    int packetSize = LoRa.parsePacket();
    if (packetSize > 0) {
        processPacket(packetSize);
    }
    
    // Reconnect WiFi if disconnected
    static unsigned long lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 30000) {  // Check every 30 seconds
        lastWifiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("📶 Reconnecting WiFi...");
            WiFi.reconnect();
        }
    }
    
    // Small delay to prevent tight loop
    delay(10);
}

