<p align="center">
  <img src="docs/images/buzzhive-logo.png" alt="BuzzHive" width="400">
</p>

<p align="center">
  <strong>Open-source smart beehive monitoring</strong><br>
  Temperature • Humidity • Weight • Audio AI
</p>

<p align="center">
  <a href="https://buzzhivetech.com">Website</a> •
  <a href="https://github.com/mbron64/buzzhive-open/discussions">Community</a>
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-Apache%202.0-blue.svg" alt="License"></a>
  <a href="https://github.com/mbron64/buzzhive-open/releases"><img src="https://img.shields.io/github/v/release/mbron64/buzzhive-open" alt="Release"></a>
</p>

<br>

<p align="center">
  <img src="docs/images/hero-device.jpg" alt="BuzzHive sensor on beehive" width="700">
</p>

---

## Overview

BuzzHive helps beekeepers monitor their hives without constant inspections. Our sensors track temperature, humidity, weight, and analyze audio to detect queen status — all synced to your phone.

**Why BuzzHive?**
- 🔓 **Fully open source** — firmware, hardware, everything
- 💰 **Affordable** — build your own from ~$45 (Bluetooth) or ~$80 (LoRa)
- 🤖 **AI-powered** — on-device audio analysis for queen detection
- 🔋 **Long battery life** — 12+ months on a single charge
- 📡 **Flexible connectivity** — Bluetooth for hobbyists, LoRa for larger operations

> **Note:** BuzzHive is currently in pre-launch. [Join the waitlist](https://buzzhivetech.com/#joinwaitlist) to get notified when commercial hardware becomes available, or build your own using this open-source project.

---

## How It Works

BuzzHive supports two connectivity options depending on your setup:

### Option A: Bluetooth (Direct to Phone)

```
┌──────────────────┐          ┌──────────────────┐          ┌──────────────────┐
│                  │          │                  │          │                  │
│   Hive Sensor    │  ─────►  │    Your Phone    │  ─────►  │      Cloud       │
│                  │   BLE    │                  │   WiFi   │    (optional)    │
│  • Temperature   │          │  • View data     │          │  • History       │
│  • Humidity      │          │  • ML inference  │          │  • Multi-hive    │
│  • Audio         │          │  • Get alerts    │          │  • Sharing       │
│                  │          │                  │          │                  │
└──────────────────┘          └──────────────────┘          └──────────────────┘
      In the hive                Sync when visiting              Optional
```

Best for: Hobbyist beekeepers with fewer hives who visit regularly.

### Option B: LoRa (Base Station)

```
┌──────────────────┐          ┌──────────────────┐          ┌──────────────────┐
│                  │          │                  │          │                  │
│   Hive Sensor    │  ─────►  │   Base Station   │  ─────►  │      Cloud       │
│                  │   LoRa   │                  │   WiFi   │    (optional)    │
│  • Temperature   │          │  • ML inference  │          │  • History       │
│  • Humidity      │          │  • Queen status  │          │  • Multi-hive    │
│  • Audio         │          │  • Alerts        │          │  • Sharing       │
│                  │          │                  │          │                  │
└──────────────────┘          └──────────────────┘          └──────────────────┘
      In the hive               At your home/apiary            Optional
```

Best for: Larger operations or remote apiaries where you want continuous monitoring without visiting.

### Getting Started

1. **Install** the sensor inside your hive
2. **Connect** via Bluetooth or LoRa depending on your setup
3. **Analyze** with on-device ML for queen status detection
4. **Get alerts** when something needs attention

---

## Features

| Feature | Description |
|---------|-------------|
| 🌡️ **Temperature** | Track brood nest temperature (±1°C accuracy) |
| 💧 **Humidity** | Monitor moisture levels to prevent mold |
| 🌬️ **Air Quality** | VOC gas sensing for hive health (BME680) |
| 🎤 **Audio AI** | Analyze bee sounds to detect queen status |
| 🔋 **Long Battery** | 12+ months on 7200mAh thin LiPo |
| 📡 **Flexible Connectivity** | Bluetooth (direct to phone) or LoRa (base station) |
| 🔓 **Open Source** | Full access to firmware, hardware, and models |

---

## Build Your Own

Everything you need to build your own BuzzHive sensor is in this repository.

### Hardware Options

#### Bluetooth Version (~$45)

| Component | Part | ~Cost |
|-----------|------|-------|
| Microcontroller | ESP32-S3 DevKit (has BLE + PSRAM) | $10 |
| Environmental Sensor | BME680 (temp, humidity, VOC) | $5 |
| Microphone | INMP441 I2S MEMS | $3 |
| Battery | 7200mAh thin LiPo | $22 |
| Enclosure + misc | 3D printed PETG, switch, connectors | $5 |
| **Total** | | **~$45** |

#### LoRa Version (~$80)

| Component | Part | ~Cost |
|-----------|------|-------|
| Hive Sensor | Heltec LoRa Kit 32 V3 (ESP32 + LoRa) | $18 |
| Base Station | ESP32 DevKit + SX1276 LoRa module | $15 |
| Environmental Sensor | BME680 (temp, humidity, VOC) | $5 |
| Microphone | INMP441 I2S MEMS | $3 |
| Battery | 7200mAh thin LiPo | $22 |
| Enclosure + misc | 3D printed PETG, switch, connectors | $8 |
| Antennas (x2) | 915MHz or 868MHz for your region | $9 |
| **Total** | | **~$80** |

Full BOM and PCB files available in [hardware/](hardware/).

### Installation

```bash
# Clone the repository
git clone https://github.com/mbron64/buzzhive-open.git
cd buzzhive-open

# Flash the hive sensor firmware
cd firmware/esp32-hive-sensor
pio run --target upload

# (LoRa only) Flash the base station firmware
cd ../esp32-base-station
pio run --target upload
```

See the [Getting Started Guide](docs/getting-started.md) for detailed instructions.

---

## Documentation

| Guide | Description |
|-------|-------------|
| [Getting Started](docs/getting-started.md) | First-time setup |
| [Build Guide](docs/build-guide.md) | DIY hardware assembly |
| [Firmware Guide](docs/firmware-guide.md) | Flashing and configuration |
| [API Reference](docs/api-reference.md) | For developers |
| [FAQ](docs/faq.md) | Common questions |
| [**Research & ML**](RESEARCH.md) | Model details, training data, improvement opportunities |

---

## Repository Structure

```
buzzhive-open/
├── firmware/
│   ├── esp32-hive-sensor/     # Hive sensor firmware (BLE + LoRa)
│   └── esp32-base-station/    # Base station firmware (LoRa only)
├── hardware/
│   ├── pcb/                   # KiCad PCB designs
│   └── enclosure/             # 3D printable cases
├── models/                    # Pre-trained ML models
└── docs/                      # Documentation
```

---

## AI & Machine Learning

BuzzHive uses on-device machine learning to analyze bee sounds and detect queen status.

| Model | Accuracy | Size | Description |
|-------|----------|------|-------------|
| XGBoost | 78.8% | 50KB | Queen status classification |
| VAE | — | 1.8MB | Anomaly detection (experimental) |

The model runs entirely on the device — your audio never leaves the hive.

**Want to improve the model?** See [Training Guide](docs/training-guide.md) to contribute labeled data or train custom models.

---

## Contributing

We welcome contributions of all kinds:

- 🐛 Bug reports and fixes
- ✨ New features
- 📖 Documentation improvements
- 🌍 Translations
- 🧪 Testing with different bee species

See [CONTRIBUTING.md](CONTRIBUTING.md) to get started.

---

## Community

- **GitHub Discussions**: [Ask questions](https://github.com/mbron64/buzzhive-open/discussions)
- **Website**: [buzzhivetech.com](https://buzzhivetech.com)

---

## License

This project is licensed under the [Apache License 2.0](LICENSE).

You are free to use, modify, and distribute this software. Attribution is appreciated but not required.

---

## Acknowledgments

- Audio analysis research based on [Chen et al. 2024](https://doi.org/10.1016/j.eswa.2024.124898)
- Thanks to all our [contributors](https://github.com/mbron64/buzzhive-open/graphs/contributors)
- Built with ❤️ for the beekeeping community

---

<p align="center">
  <a href="https://buzzhivetech.com">buzzhivetech.com</a>
</p>
