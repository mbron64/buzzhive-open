# 🐝 Buzzhive Open

**Open-source beehive health monitoring using AI-powered audio analysis**

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Discord](https://img.shields.io/badge/Discord-Join%20Community-7289da)](https://discord.gg/buzzhive)

<p align="center">
  <img src="docs/images/buzzhive-system.png" alt="Buzzhive System" width="600">
</p>

## 🎯 What is Buzzhive?

Buzzhive is an AI-powered system that monitors beehive health by analyzing the sounds bees make. Using machine learning, it can detect:

- **Queen Status**: Is the queen present, missing, newly hatched, or being accepted?
- **Anomalies**: Unusual patterns that might indicate disease, pests, or stress

## ✨ Features

| Feature | Free (Open Source) | Pro (Cloud) |
|---------|-------------------|-------------|
| Queen status detection | ✅ 78.8% accuracy | ✅ 90%+ accuracy |
| Local inference | ✅ On base station | ✅ Plus cloud |
| Mobile app | ✅ Basic status | ✅ Full dashboard |
| Multi-hive support | ✅ 1 hive | ✅ Unlimited |
| Alerts | ❌ | ✅ Push + Email |
| Historical data | ❌ | ✅ 1 year |

## 🏗️ System Architecture

```
┌─────────────────┐      LoRa       ┌─────────────────┐      WiFi      ┌─────────────┐
│  Hive Sensor    │ ──────────────► │  Base Station   │ ─────────────► │  Dashboard  │
│  (ESP32 + Mic)  │   Audio data    │  (ESP32 + ML)   │   Results      │  (App/Web)  │
└─────────────────┘                 └─────────────────┘                └─────────────┘
     Battery                             Wall power                      Your phone
     6+ months                           XGBoost AI                      
```

## 🚀 Quick Start

### Option 1: Buy a Kit (Recommended)
Visit [buzzhive.io](https://buzzhive.io) to get a pre-assembled kit with everything you need.

### Option 2: Build Your Own

#### Hardware Required
- 2x ESP32-S3 DevKit
- 1x I2S MEMS Microphone (INMP441 or SPH0645)
- 2x LoRa Module (SX1276/SX1278)
- 1x Temperature/Humidity Sensor (SHT31)
- 1x LiPo Battery (10,000 mAh recommended)
- Custom PCB ([order from PCBWay](hardware/pcb/README.md))

#### Software Setup

1. **Clone this repo**
```bash
git clone https://github.com/buzzhive/buzzhive-open.git
cd buzzhive-open
```

2. **Flash the hive sensor**
```bash
cd firmware/esp32-hive-sensor
pio run --target upload
```

3. **Flash the base station**
```bash
cd ../esp32-base-station
pio run --target upload
```

4. **Install the mobile app**
- iOS: [App Store](https://apps.apple.com/app/buzzhive)
- Android: [Play Store](https://play.google.com/store/apps/details?id=io.buzzhive)

## 📊 How the AI Works

We use a **XGBoost classifier** trained on thousands of hours of beehive audio:

1. **Record** 10 seconds of audio every 30 minutes
2. **Extract** MFCC features (78 total)
3. **Classify** queen status using ML model
4. **Transmit** result over LoRa (just ~50 bytes)

```python
# Simplified inference pipeline
audio = record_audio(duration=10)        # 10 seconds
mfcc = extract_mfcc_features(audio)      # 78 features
status = xgboost_predict(mfcc)           # 0-3 class
send_lora(hive_id, status, temp, humid)  # Transmit
```

## 📁 Repository Structure

```
buzzhive-open/
├── firmware/
│   ├── esp32-hive-sensor/    # Hive sensor code
│   └── esp32-base-station/   # Base station with ML
├── hardware/
│   ├── pcb/                  # PCB design files
│   └── enclosure/            # 3D printable cases
├── models/                   # ML model files
├── mobile-app/              # React Native app
├── docs/                    # Documentation
└── examples/                # Example projects
```

## 🔬 Training Your Own Model

Want to improve the model or train for your specific bee species?

```bash
cd examples/custom-model
pip install -r requirements.txt
python train.py --data your_audio_folder/
```

See [Training Guide](docs/training-guide.md) for details.

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Areas We Need Help
- 🌍 Translations
- 📱 Mobile app improvements
- 🧪 Testing with different bee species
- 📝 Documentation
- 🐛 Bug fixes

## 📜 License

This project is licensed under the Apache 2.0 License - see [LICENSE](LICENSE) for details.

## 🙏 Acknowledgments

- Research based on [Chen et al. 2024](https://doi.org/10.1016/j.eswa.2024.124898)
- Audio dataset from [Open Source Beehives Project](https://opensourcebeehives.com)
- Community contributors

## 📞 Support

- 💬 [Discord Community](https://discord.gg/buzzhive)
- 📧 support@buzzhive.io
- 🐛 [GitHub Issues](https://github.com/buzzhive/buzzhive-open/issues)

---

<p align="center">
  Made with 🐝 by the Buzzhive team
</p>

