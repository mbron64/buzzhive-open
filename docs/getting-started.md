# Getting Started with Buzzhive

This guide will walk you through setting up your Buzzhive beehive monitoring system.

## 📋 Prerequisites

### Hardware Required

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32-S3 DevKit | 1 | For hive sensor (has PSRAM for audio) |
| ESP32 DevKit | 1 | For base station |
| INMP441 I2S Microphone | 1 | Digital MEMS mic |
| SX1276 LoRa Module | 2 | One for each ESP32 |
| SHT31 Temp/Humidity Sensor | 1 | Optional but recommended |
| LiPo Battery | 1 | 7200 mAh thin profile (fits inside hive) |
| TP4056 Charging Module | 1 | For battery charging |
| Antenna (915MHz or 868MHz) | 2 | Match your region |

### Software Required

- [PlatformIO](https://platformio.org/) (VSCode extension or CLI)
- Git
- USB drivers for ESP32

## 🚀 Quick Setup

### Step 1: Clone the Repository

```bash
git clone https://github.com/buzzhive/buzzhive-open.git
cd buzzhive-open
```

### Step 2: Configure the Hive Sensor

1. Open `firmware/esp32-hive-sensor/src/config.h`
2. Set your `HIVE_ID` (unique for each hive)
3. Set `LORA_FREQUENCY` for your region:
   - Americas: `915E6`
   - Europe: `868E6`
   - Asia: `433E6`

```c
#define HIVE_ID 1
#define LORA_FREQUENCY 915E6
```

### Step 3: Flash the Hive Sensor

```bash
cd firmware/esp32-hive-sensor
pio run --target upload
```

### Step 4: Configure the Base Station

1. Open `firmware/esp32-base-station/src/config.h`
2. Set your WiFi credentials:

```c
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourPassword"
```

3. (Optional) Add your Buzzhive API key for cloud features:

```c
#define API_KEY "your-api-key-here"
```

### Step 5: Flash the Base Station

```bash
cd ../esp32-base-station
pio run --target upload
```

### Step 6: Test the System

1. Power on the base station (connect to USB or power supply)
2. Open Serial Monitor at 115200 baud
3. You should see: `✅ Ready! Waiting for hive sensor data...`
4. Power on the hive sensor
5. Wait for data transmission (default: every 15 minutes, or immediately on first boot)

## 🔧 Hardware Assembly

### Hive Sensor Wiring

```
ESP32-S3          INMP441 Mic
--------          -----------
GPIO12  ────────  SCK
GPIO13  ────────  SD
GPIO15  ────────  WS
3.3V    ────────  VDD
GND     ────────  GND
                  L/R → GND (left channel)

ESP32-S3          SX1276 LoRa
--------          -----------
GPIO5   ────────  NSS
GPIO14  ────────  RST
GPIO2   ────────  DIO0
GPIO18  ────────  SCK
GPIO19  ────────  MISO
GPIO23  ────────  MOSI
3.3V    ────────  VCC
GND     ────────  GND

ESP32-S3          SHT31 (Optional)
--------          ----------------
GPIO21  ────────  SDA
GPIO22  ────────  SCL
3.3V    ────────  VIN
GND     ────────  GND
```

### Base Station Wiring

```
ESP32             SX1276 LoRa
-----             -----------
GPIO5   ────────  NSS
GPIO14  ────────  RST
GPIO2   ────────  DIO0
GPIO18  ────────  SCK
GPIO19  ────────  MISO
GPIO23  ────────  MOSI
3.3V    ────────  VCC
GND     ────────  GND
```

## 📊 Understanding the Data

### Queen Status Classes

| Class | Name | Description |
|-------|------|-------------|
| 0 | Queenright | Queen present and laying |
| 1 | Queenless | No queen detected |
| 2 | Queen_Hatched | New queen recently emerged |
| 3 | Queen_Accepted | Queen present and accepted by colony |

### LED Indicators

**Hive Sensor:**
- Single blink: Transmission sent
- Rapid blink: Error

**Base Station:**
- Single blink: Packet received
- Triple blink: ML inference completed
- Solid: WiFi connected

## 🔋 Power Optimization

### Battery Life Estimates

| Mode | Interval | Battery Life (7200 mAh) |
|------|----------|-------------------------|
| Active | 15 min | ~2.5 months |
| Winter | 2 hours | ~6 months |
| Adaptive | Auto | ~12 months average |

### Tips for Longer Battery Life

1. **Use winter mode** when temperatures drop below 15°C
2. **Increase sleep interval** if queen status is stable
3. **Use solar panel** (5V, 1W minimum) for indefinite operation
4. **Disable unused sensors** in config.h

## 🌐 Cloud Integration (Optional)

### Using Buzzhive Cloud

1. Create account at [buzzhive.io](https://buzzhive.io)
2. Add your hive in the dashboard
3. Copy your API key
4. Add to base station config.h
5. Reflash base station

### Self-Hosted Option

Run your own backend:

```bash
cd examples/self-hosted-backend
docker-compose up -d
```

Update `API_ENDPOINT` in config.h to your server address.

## 🐛 Troubleshooting

### No Data Received at Base Station

1. Check LoRa frequency matches on both devices
2. Ensure antennas are connected
3. Check distance (max ~2km line of sight)
4. Verify hive sensor is powered

### Poor Classification Accuracy

1. Ensure microphone is properly positioned inside hive
2. Check audio quality with test recording
3. Retrain model with local data (see training guide)

### WiFi Connection Issues

1. Check credentials in config.h
2. Ensure base station is within WiFi range
3. Check router allows new devices

## 📚 Next Steps

- [Hardware Build Guide](hardware-build-guide.md) - Detailed PCB assembly
- [Training Guide](training-guide.md) - Train custom models
- [API Reference](api-reference.md) - Cloud API documentation
- [Contributing](../CONTRIBUTING.md) - Help improve Buzzhive

## 🆘 Getting Help

- 💬 [Discord Community](https://discord.gg/buzzhive)
- 📧 support@buzzhive.io
- 🐛 [GitHub Issues](https://github.com/buzzhive/buzzhive-open/issues)

