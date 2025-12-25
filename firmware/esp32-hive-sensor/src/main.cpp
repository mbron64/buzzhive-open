/**
 * Buzzhive Hive Sensor Firmware
 * 
 * Records audio from beehive, extracts MFCC features,
 * and transmits data via LoRa to base station.
 * 
 * Hardware:
 * - ESP32-S3
 * - INMP441 I2S Microphone
 * - SX1276 LoRa Module
 * - SHT31 Temperature/Humidity Sensor
 * 
 * License: Apache 2.0
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <ArduinoJson.h>
#include "config.h"
#include "mfcc.h"
#include "audio_compression.h"

// ============================================================================
// Configuration
// ============================================================================

// Unique hive identifier (change for each hive)
#define HIVE_ID 1

// Recording parameters
#define SAMPLE_RATE 22050
#define RECORD_DURATION_SEC 10
#define AUDIO_BUFFER_SIZE (SAMPLE_RATE * RECORD_DURATION_SEC)

// Sleep intervals (milliseconds)
#define ACTIVE_SEASON_INTERVAL_MS (15 * 60 * 1000)   // 15 minutes
#define WINTER_INTERVAL_MS (2 * 60 * 60 * 1000)      // 2 hours
#define ALERT_MODE_INTERVAL_MS (5 * 60 * 1000)       // 5 minutes

// LoRa pins (adjust for your PCB)
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

// I2S pins for INMP441 microphone
#define I2S_WS 15
#define I2S_SD 13
#define I2S_SCK 12
#define I2S_PORT I2S_NUM_0

// ============================================================================
// Global Variables
// ============================================================================

Adafruit_SHT31 sht31 = Adafruit_SHT31();
int16_t* audioBuffer = nullptr;
float mfccFeatures[78];  // 13 MFCCs + 13 deltas + 13 delta-deltas * (mean + std)

// Transmission packet structure
struct __attribute__((packed)) BuzzhivePacket {
    uint8_t hiveId;
    uint8_t queenStatus;      // 0-3: Queenright, Queenless, Hatched, Accepted
    uint8_t anomalyScore;     // 0-255 scaled
    int16_t temperature;      // x100 for 2 decimal precision
    uint8_t humidity;
    uint16_t batteryMv;
    uint32_t timestamp;
    uint8_t featureHash[4];   // Quick hash of MFCC features for validation
};

// ============================================================================
// I2S Microphone Setup
// ============================================================================

void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
}

// ============================================================================
// Audio Recording
// ============================================================================

bool recordAudio() {
    Serial.println("🎤 Recording audio...");
    
    size_t bytesRead = 0;
    size_t totalSamples = 0;
    
    unsigned long startTime = millis();
    
    while (totalSamples < AUDIO_BUFFER_SIZE) {
        size_t toRead = min((size_t)1024, (AUDIO_BUFFER_SIZE - totalSamples) * 2);
        i2s_read(I2S_PORT, &audioBuffer[totalSamples], toRead, &bytesRead, portMAX_DELAY);
        totalSamples += bytesRead / 2;
        
        // Timeout protection
        if (millis() - startTime > (RECORD_DURATION_SEC + 2) * 1000) {
            Serial.println("⚠️ Recording timeout");
            return false;
        }
    }
    
    Serial.printf("✅ Recorded %d samples in %lu ms\n", totalSamples, millis() - startTime);
    return true;
}

// ============================================================================
// MFCC Feature Extraction
// ============================================================================

void extractMFCCFeatures() {
    Serial.println("🔢 Extracting MFCC features...");
    
    // Apply pre-emphasis filter
    for (int i = AUDIO_BUFFER_SIZE - 1; i > 0; i--) {
        audioBuffer[i] = audioBuffer[i] - 0.97 * audioBuffer[i - 1];
    }
    
    // Normalize audio
    int16_t maxVal = 0;
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        if (abs(audioBuffer[i]) > maxVal) maxVal = abs(audioBuffer[i]);
    }
    if (maxVal > 0) {
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            audioBuffer[i] = (audioBuffer[i] * 32767) / maxVal;
        }
    }
    
    // Extract MFCCs using our lightweight library
    // This computes 13 MFCCs per frame, then aggregates
    extractMFCC(audioBuffer, AUDIO_BUFFER_SIZE, SAMPLE_RATE, mfccFeatures);
    
    Serial.println("✅ MFCC extraction complete");
}

// ============================================================================
// LoRa Transmission
// ============================================================================

void setupLoRa() {
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    
    if (!LoRa.begin(915E6)) {  // 915 MHz for US, 868 MHz for EU
        Serial.println("❌ LoRa init failed!");
        while (1);
    }
    
    // Optimize for range (low data rate)
    LoRa.setSpreadingFactor(10);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    
    Serial.println("✅ LoRa initialized");
}

void transmitData(uint8_t queenStatus, uint8_t anomalyScore) {
    float temp = sht31.readTemperature();
    float humid = sht31.readHumidity();
    
    BuzzhivePacket packet;
    packet.hiveId = HIVE_ID;
    packet.queenStatus = queenStatus;
    packet.anomalyScore = anomalyScore;
    packet.temperature = (int16_t)(temp * 100);
    packet.humidity = (uint8_t)humid;
    packet.batteryMv = analogRead(A0) * 2;  // Assuming voltage divider
    packet.timestamp = millis() / 1000;
    
    // Simple hash of first 4 MFCC values for data validation
    memcpy(packet.featureHash, mfccFeatures, 4);
    
    Serial.printf("📡 Transmitting: Queen=%d, Anomaly=%d, Temp=%.1f°C\n",
                  queenStatus, anomalyScore, temp);
    
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&packet, sizeof(packet));
    LoRa.endPacket();
    
    Serial.println("✅ Transmission complete");
}

// ============================================================================
// Power Management
// ============================================================================

bool isWinterMode() {
    // Simple heuristic: if hive temp < 20°C, likely winter/inactive
    float temp = sht31.readTemperature();
    return temp < 20.0;
}

uint32_t getSleepDuration() {
    if (isWinterMode()) {
        return WINTER_INTERVAL_MS;
    }
    return ACTIVE_SEASON_INTERVAL_MS;
}

void enterDeepSleep(uint32_t durationMs) {
    Serial.printf("💤 Sleeping for %lu seconds...\n", durationMs / 1000);
    Serial.flush();
    
    // Disable peripherals
    i2s_driver_uninstall(I2S_PORT);
    LoRa.sleep();
    
    esp_sleep_enable_timer_wakeup(durationMs * 1000ULL);
    esp_deep_sleep_start();
}

// ============================================================================
// Main Setup & Loop
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n🐝 Buzzhive Hive Sensor v1.0");
    Serial.printf("   Hive ID: %d\n", HIVE_ID);
    
    // Allocate audio buffer in PSRAM if available
    if (psramFound()) {
        audioBuffer = (int16_t*)ps_malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
        Serial.println("   Using PSRAM for audio buffer");
    } else {
        audioBuffer = (int16_t*)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
        Serial.println("   Using internal RAM for audio buffer");
    }
    
    if (!audioBuffer) {
        Serial.println("❌ Failed to allocate audio buffer!");
        while (1);
    }
    
    // Initialize peripherals
    Wire.begin();
    
    if (!sht31.begin(0x44)) {
        Serial.println("⚠️ SHT31 not found, continuing without temp/humidity");
    }
    
    setupI2S();
    setupLoRa();
    
    Serial.println("✅ Setup complete\n");
}

void loop() {
    // 1. Record audio
    if (!recordAudio()) {
        Serial.println("❌ Recording failed, retrying in 1 minute");
        enterDeepSleep(60 * 1000);
        return;
    }
    
    // 2. Extract MFCC features
    extractMFCCFeatures();
    
    // 3. For now, send features to base station for classification
    //    (Full on-device inference requires more memory)
    //    Base station will return classification via LoRa ACK
    
    // Placeholder: In standalone mode, we could run a simpler model here
    uint8_t queenStatus = 3;  // Default: Queen_Accepted (normal)
    uint8_t anomalyScore = 0;
    
    // 4. Transmit data
    transmitData(queenStatus, anomalyScore);
    
    // 5. Deep sleep until next reading
    enterDeepSleep(getSleepDuration());
}

