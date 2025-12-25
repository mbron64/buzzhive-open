# Buzzhive ML Models

This directory contains the pre-trained machine learning models for bee colony status classification.

## Available Models

### XGBoost Queen Status Detector

| File | Description | Size |
|------|-------------|------|
| `xgboost_queen_detector.json` | Full XGBoost model (800 trees) | ~4.2 MB |
| `buzzhive_ml.h` | C header with scaler parameters | ~2.4 KB |
| `scaler_params.json` | Feature normalization parameters | ~3.8 KB |

### Model Performance

| Metric | Value |
|--------|-------|
| Accuracy | 78.8% |
| F1 Score | 0.787 |
| Classes | 4 (Queenright, Queenless, Queen_Hatched, Queen_Accepted) |
| Input Features | 78 (MFCC mean/std + deltas) |

### Class Distribution

| Class | Name | Training Samples |
|-------|------|------------------|
| 0 | Queenright | 725 |
| 1 | Queenless | 661 |
| 2 | Queen_Hatched | 1,088 |
| 3 | Queen_Accepted | 2,493 |

## Usage

### On ESP32 Base Station

The base station uses a simplified inference with the scaler parameters:

```cpp
#include "buzzhive_ml.h"

float features[78];  // MFCC features
float normalized[78];

// Normalize features
for (int i = 0; i < 78; i++) {
    normalized[i] = (features[i] - MEAN[i]) / SCALE[i];
}

// Run inference (simplified or full model)
int prediction = runInference(normalized);
```

### Full Model Loading

For higher accuracy, load the full JSON model:

```python
import xgboost as xgb
import json
import numpy as np

# Load model
model = xgb.Booster()
model.load_model('xgboost_queen_detector.json')

# Load scaler
with open('scaler_params.json') as f:
    scaler = json.load(f)

# Normalize features
features = np.array(your_features)
normalized = (features - scaler['mean']) / scaler['scale']

# Predict
import xgboost as xgb
dmatrix = xgb.DMatrix(normalized.reshape(1, -1))
prediction = model.predict(dmatrix)
```

## Training Your Own Model

See the [training notebooks](../examples/custom-model/) for how to train with your own data.

### Requirements

- Python 3.8+
- librosa (audio processing)
- scikit-learn
- xgboost
- pandas, numpy

### Quick Training

```bash
cd examples/custom-model
pip install -r requirements.txt
python train.py --data /path/to/your/audio/
```

## Model Architecture

```
Audio (10 sec @ 22050 Hz)
         │
         ▼
┌─────────────────┐
│ Pre-emphasis    │
│ (0.97 filter)   │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│ MFCC Extraction │
│ (13 coeffs)     │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│ Delta + Delta²  │
│ (39 features)   │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│ Mean + Std      │
│ (78 features)   │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│ StandardScaler  │
│ (normalize)     │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│ XGBoost         │
│ (800 trees)     │
└─────────────────┘
         │
         ▼
   Queen Status
   (0-3)
```

## Improving Accuracy

Based on research by [Chen et al. 2024](https://doi.org/10.1016/j.eswa.2024.124898), accuracy can reach 98.8% with:

1. **Controlled data collection** - Known queen status during recording
2. **Higher quality audio** - I2S MEMS microphone at 44.1kHz
3. **Balanced classes** - Equal samples per condition
4. **Clean labels** - Ground truth from hive inspections

## License

Models are released under Apache 2.0 license. See [LICENSE](../LICENSE).

