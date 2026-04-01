# Buzzhive Open - Project Instructions

## Project Overview

Buzzhive is an open-source AI-powered beehive health monitoring system. It uses audio analysis to detect queen status and anomalies in beehives.

### Architecture
```
ESP32 Hive Sensor --LoRa--> ESP32 Base Station --WiFi--> Cloud/Dashboard
     |                            |
     | Records audio              | Runs XGBoost ML
     | Extracts MFCCs             | 78.8% accuracy
     | Battery powered            | Wall powered
```

### Key Components
- **Hive Sensor**: ESP32-S3 + INMP441 mic + LoRa + sensors (temp/humidity)
- **Base Station**: ESP32 + LoRa + WiFi + XGBoost inference
- **ML Model**: XGBoost classifier (78.8% accuracy, 4 queen status classes)
- **Features**: 78 MFCC coefficients extracted from 10-second audio

### Queen Status Classes
- `0` - Queenright (healthy, queen present)
- `1` - Queenless (queen lost/dead)
- `2` - Queen_New (newly introduced queen)
- `3` - Queen_Accepted (queen accepted by colony)

## Task Management

When working on complex tasks:
- Always create a structured todo list at the beginning using the TodoWrite tool
- Break down tasks into specific, actionable subtasks
- Mark todos as `in_progress` when starting work on them
- Mark todos as `completed` immediately after finishing each item
- Only have ONE todo as `in_progress` at a time
- This helps track progress and ensures nothing is missed

## Code Style

### C/C++ (ESP32 Firmware)
- Use PlatformIO project structure
- Follow Arduino-style naming (camelCase for functions, UPPER_CASE for constants)
- Keep functions small and focused
- Comment complex logic, especially audio/ML processing
- Use `config.h` for all configurable parameters

### Python (ML/Training)
- Follow PEP 8 style
- Use type hints where helpful
- Document functions with docstrings
- Keep notebooks focused on single tasks

## Important Directories

```
buzzhive-open/
├── firmware/
│   ├── esp32-hive-sensor/    # Hive sensor firmware (complete)
│   └── esp32-base-station/   # Base station firmware (complete)
├── models/                    # Trained ML models (complete)
├── hardware/                  # PCB/enclosure designs (TODO: empty)
├── mobile-app/               # Mobile app source (TODO: empty)
├── examples/                 # Training examples (TODO: empty)
└── docs/                     # Documentation
```

## Development Notes

- The ML model was trained on 7,100 audio samples from the Kaggle Smart Bee Colony dataset
- Research paper reference: Chen et al. 2024 achieved 98.8% with controlled conditions
- Our 78.8% accuracy is realistic for real-world, noisy field data
- Power optimization is critical - target 6+ months battery life over winter
- LoRa bandwidth is limited (~100 bytes/packet) - send features, not raw audio

## When Making Changes

1. Test firmware changes on actual ESP32 hardware when possible
2. Validate ML model changes against the test set in `/research/outputs/`
3. Update documentation when adding new features
4. Keep backward compatibility with existing deployed sensors
