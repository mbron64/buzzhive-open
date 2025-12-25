/**
 * Buzzhive Base Station Configuration
 * 
 * Edit this file to customize your base station settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// WiFi Configuration
// ============================================================================

// Your WiFi network name
#define WIFI_SSID "YOUR_WIFI_SSID"

// Your WiFi password
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ============================================================================
// Cloud API Configuration
// ============================================================================

// Buzzhive Cloud API endpoint
#define API_ENDPOINT "https://api.buzzhive.io/v1/telemetry"

// Your API key (get from buzzhive.io dashboard)
// Leave empty for local-only mode
#define API_KEY ""

// ============================================================================
// LoRa Configuration
// ============================================================================

// LoRa frequency (must match hive sensors)
#define LORA_FREQUENCY 915E6

// LoRa settings (must match hive sensors)
#define LORA_SPREADING_FACTOR 10
#define LORA_BANDWIDTH 125E3

// ============================================================================
// Pin Definitions
// ============================================================================

// LoRa Module (SX1276)
#define LORA_SS_PIN 5
#define LORA_RST_PIN 14
#define LORA_DIO0_PIN 2

// Status LED
#define LED_PIN 2

// ============================================================================
// Feature Flags
// ============================================================================

// Enable full XGBoost model from SPIFFS (requires model upload)
// #define USE_FULL_MODEL

// Enable MQTT instead of HTTP
// #define USE_MQTT

// Enable local web server for configuration
// #define ENABLE_WEB_CONFIG

#endif // CONFIG_H

