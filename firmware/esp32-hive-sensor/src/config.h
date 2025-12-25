/**
 * Buzzhive Hive Sensor Configuration
 * 
 * Edit this file to customize your hive sensor settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Hive Identification
// ============================================================================

// Unique ID for this hive (1-255)
// IMPORTANT: Each hive in your apiary must have a unique ID
#define HIVE_ID 1

// ============================================================================
// LoRa Configuration
// ============================================================================

// LoRa frequency (depends on your region)
// 915E6 = 915 MHz (Americas)
// 868E6 = 868 MHz (Europe)
// 433E6 = 433 MHz (Asia)
#define LORA_FREQUENCY 915E6

// LoRa spreading factor (7-12)
// Higher = longer range but slower
#define LORA_SPREADING_FACTOR 10

// LoRa bandwidth
#define LORA_BANDWIDTH 125E3

// ============================================================================
// Audio Configuration
// ============================================================================

// Sample rate for audio recording
#define AUDIO_SAMPLE_RATE 22050

// Recording duration in seconds
#define AUDIO_DURATION_SEC 10

// ============================================================================
// Power Management
// ============================================================================

// Sleep interval during active season (milliseconds)
#define ACTIVE_INTERVAL_MS (15 * 60 * 1000)  // 15 minutes

// Sleep interval during winter (milliseconds)
#define WINTER_INTERVAL_MS (2 * 60 * 60 * 1000)  // 2 hours

// Temperature threshold for winter mode (Celsius)
#define WINTER_TEMP_THRESHOLD 15.0

// ============================================================================
// Pin Definitions
// ============================================================================

// I2S Microphone (INMP441)
#define I2S_WS_PIN 15
#define I2S_SD_PIN 13
#define I2S_SCK_PIN 12

// LoRa Module (SX1276)
#define LORA_SS_PIN 5
#define LORA_RST_PIN 14
#define LORA_DIO0_PIN 2

// I2C for sensors
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Battery voltage monitoring (ADC)
#define BATTERY_PIN 36

// Status LED
#define LED_PIN 2

#endif // CONFIG_H

