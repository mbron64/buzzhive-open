# BuzzHive ML Research

This document provides comprehensive information for ML researchers looking to understand, reproduce, or improve BuzzHive's audio-based bee colony health monitoring system.

## Table of Contents

- [Overview](#overview)
- [Model Card](#model-card)
- [Dataset](#dataset)
- [Training Procedure](#training-procedure)
- [Experiments & Results](#experiments--results)
- [Deployment Constraints](#deployment-constraints)
- [Improvement Opportunities](#improvement-opportunities)
- [Contributing](#contributing)
- [References](#references)

---

## Overview

BuzzHive uses machine learning to analyze bee colony audio and detect queen status. The system extracts MFCC features from 10-second audio recordings and classifies them into one of four queen status categories.

### Problem Statement

Beekeepers need to know if their hive is queenless, but traditional inspection methods are:
- Time-consuming (requires opening the hive)
- Disruptive to the colony
- Weather-dependent
- Requires experience to interpret

Audio analysis enables non-invasive, continuous monitoring.

### Current Best Model

| Metric | Value |
|--------|-------|
| Model | XGBoost (800 trees) |
| Accuracy | 78.8% |
| F1 Score | 0.787 |
| Model Size | ~4.2 MB (JSON) |
| Inference Time | <100ms on ESP32 |

---

## Model Card

### Model Details

- **Model Name**: BuzzHive Queen Status Detector
- **Model Type**: XGBoost Gradient Boosting Classifier
- **Version**: 1.0
- **License**: Apache 2.0
- **Developers**: BuzzHive Team
- **Contact**: GitHub Issues or Discussions

### Intended Use

**Primary Use**: Detecting queen status in honey bee (Apis mellifera) colonies via audio analysis.

**Intended Users**: Beekeepers, apiary managers, researchers.

**Out-of-Scope Uses**:
- Other bee species (not tested)
- Disease detection (model only detects queen status)
- Real-time streaming (designed for 10-second clips)

### Input/Output

**Input**: 78-dimensional feature vector (MFCC statistics)
- 13 MFCC coefficients × 2 (mean + std)
- 13 delta coefficients × 2 (mean + std)
- 13 delta-delta coefficients × 2 (mean + std)

**Output**: Class prediction (0-3)

| Class | Label | Description |
|-------|-------|-------------|
| 0 | Queenright | Healthy hive, queen present and laying |
| 1 | Queenless | Queen lost, dead, or absent |
| 2 | Queen_Hatched | Virgin queen recently emerged |
| 3 | Queen_Accepted | New queen accepted by colony |

### Performance Metrics

#### Overall Performance

| Model | Accuracy | F1 Score | Precision | Recall |
|-------|----------|----------|-----------|--------|
| **XGBoost** | **78.8%** | **0.787** | **0.788** | **0.788** |
| Random Forest | 75.2% | 0.747 | 0.748 | 0.752 |
| SVM (RBF) | 73.0% | 0.729 | 0.730 | 0.730 |
| CNN | 61.2% | 0.582 | — | — |

#### Per-Class Performance (XGBoost)

| Class | Precision | Recall | F1 | Support |
|-------|-----------|--------|-----|---------|
| Queenright | 0.76 | 0.74 | 0.75 | 145 |
| Queenless | 0.71 | 0.68 | 0.69 | 132 |
| Queen_Hatched | 0.78 | 0.82 | 0.80 | 218 |
| Queen_Accepted | 0.85 | 0.86 | 0.85 | 499 |

**Note**: Class imbalance affects performance. Queen_Accepted has the most samples and highest accuracy.

### Limitations

1. **Class Imbalance**: Training data is imbalanced (Queen_Accepted: 2,493 samples vs Queenless: 661 samples)
2. **Dataset Source**: Single dataset (Kaggle) with controlled conditions may not generalize to all environments
3. **Species**: Only tested on Apis mellifera (Western honey bee)
4. **Environment**: Background noise, weather, and hive type may affect accuracy
5. **Temporal**: Model sees 10-second snapshots, not trends over time

### Ethical Considerations

- False negatives (missing a queenless hive) could lead to colony loss
- Model should be used as a decision-support tool, not a replacement for inspection
- Beekeepers should verify predictions with physical inspection for critical decisions

---

## Dataset

### Source

**Kaggle Smart Bee Colony Monitor Dataset**

- **URL**: https://www.kaggle.com/datasets/annajyang/smart-bee-colony-monitor
- **Total Samples**: 7,100 audio recordings
- **Format**: WAV files, 22,050 Hz sample rate
- **Duration**: 10 seconds per clip
- **Labels**: Queen status (4 classes)

### Class Distribution

| Class | Name | Samples | Percentage |
|-------|------|---------|------------|
| 0 | Queenright | 725 | 10.2% |
| 1 | Queenless | 661 | 9.3% |
| 2 | Queen_Hatched | 1,088 | 15.3% |
| 3 | Queen_Accepted | 2,493 | 35.1% |
| — | Other/Unlabeled | 2,133 | 30.0% |

**Note**: Only labeled samples (4,967) were used for supervised learning.

### Data Split

| Split | Samples | Percentage |
|-------|---------|------------|
| Train | 3,477 | 70% |
| Validation | 497 | 10% |
| Test | 993 | 20% |

Stratified split to maintain class proportions.

### Data Collection Context

Audio was recorded from multiple hives using digital MEMS microphones placed inside the hive. Recordings include natural hive sounds (buzzing, fanning, queen piping).

---

## Training Procedure

### Feature Extraction Pipeline

```
Audio (10 sec @ 22,050 Hz)
         │
         ▼
┌─────────────────────┐
│ 1. Pre-emphasis     │  y[n] = x[n] - 0.97 * x[n-1]
│    (α = 0.97)       │
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│ 2. MFCC Extraction  │  librosa.feature.mfcc()
│    n_mfcc = 13      │  hop_length = 512
│    n_fft = 2048     │  fmin = 20, fmax = 8000
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│ 3. Delta Features   │  1st and 2nd order derivatives
│    width = 9        │
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│ 4. Statistics       │  Mean and Std across time
│    (78 features)    │  13 × 3 × 2 = 78
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│ 5. StandardScaler   │  Zero mean, unit variance
│                     │
└─────────────────────┘
         │
         ▼
┌─────────────────────┐
│ 6. XGBoost          │  800 trees, max_depth=6
│    Classifier       │
└─────────────────────┘
         │
         ▼
    Queen Status (0-3)
```

### XGBoost Hyperparameters

```python
xgb.XGBClassifier(
    n_estimators=800,
    max_depth=6,
    learning_rate=0.1,
    subsample=0.8,
    colsample_bytree=0.8,
    objective='multi:softmax',
    num_class=4,
    random_state=42,
    n_jobs=-1
)
```

### Reproducibility

To reproduce results:

1. **Random Seeds**: All experiments use `random_state=42`
2. **Data Split**: Use provided train/val/test CSVs in `outputs/processed/`
3. **Dependencies**:
   ```
   python==3.10+
   librosa==0.10.1
   scikit-learn==1.3.0
   xgboost==2.0.0
   numpy==1.24.0
   ```
4. **Hardware**: Training completed on CPU (M1 Mac) in ~5 minutes

---

## Experiments & Results

### Models Evaluated

| Approach | Model | Accuracy | Notes |
|----------|-------|----------|-------|
| **Traditional ML** | XGBoost | **78.8%** | Best overall |
| Traditional ML | Random Forest | 75.2% | Good baseline |
| Traditional ML | SVM (RBF) | 73.0% | Slower inference |
| Deep Learning | CNN (1D) | 61.2% | Insufficient data |
| Anomaly Detection | VAE | 0.61 AUC | For unknown anomalies |

### Key Findings

1. **Traditional ML outperforms Deep Learning** with this dataset size (7,100 samples). XGBoost beat CNN by 17+ percentage points.

2. **MFCC features are effective** for bee audio classification. 78 features capture the relevant acoustic patterns.

3. **Class imbalance hurts minority classes**. Queenless (smallest class) has lowest recall. SMOTE oversampling provided marginal improvement (+1-2%).

4. **VAE useful for anomaly detection**. While classification accuracy is lower, VAE can detect out-of-distribution samples (novel hive problems not in training data).

### What Didn't Work

| Experiment | Result | Reason |
|------------|--------|--------|
| Raw audio CNN | 58% accuracy | Insufficient data for deep learning |
| Mel spectrogram CNN | 61% accuracy | Same issue |
| Data augmentation (pitch shift, time stretch) | +1% accuracy | Limited impact |
| SMOTE oversampling | +2% accuracy | Marginal improvement |
| Ensemble (XGBoost + RF + SVM) | 79% accuracy | Not worth complexity |

---

## Deployment Constraints

### Hardware Target

- **MCU**: ESP32-S3 (240 MHz, 512KB SRAM, 8MB PSRAM)
- **Memory Budget**: <100KB for model parameters
- **Inference Time**: <500ms per prediction
- **Power**: Battery-powered, sleep between readings

### Current Deployment

The XGBoost model (4.2 MB JSON) is too large for ESP32 flash. Current solution:

1. **Simplified inference** using only scaler parameters (`buzzhive_ml.h`)
2. **Top-N trees** approach (use first 100 trees for ~70% accuracy)
3. **Quantization** potential (int8) not yet implemented

### Model Size Targets

| Approach | Size | Accuracy | Status |
|----------|------|----------|--------|
| Full XGBoost (800 trees) | 4.2 MB | 78.8% | Too large |
| Top-100 trees | ~500 KB | ~70% | Fits in flash |
| Quantized (int8) | ~1 MB | TBD | Not implemented |
| m2cgen C export | ~2 MB | 78.8% | Code size issue |

---

## Improvement Opportunities

### High Priority

1. **Collect more diverse data**
   - Different hive types (Langstroth, Top Bar, Warré)
   - Different climates and seasons
   - Different bee subspecies
   - Ground-truth labels from experienced beekeepers

2. **Address class imbalance**
   - Collect more Queenless and Queenright samples
   - Experiment with class weights, focal loss

3. **Model compression for ESP32**
   - Knowledge distillation to smaller model
   - Pruning and quantization
   - TinyML frameworks (TensorFlow Lite Micro)

### Medium Priority

4. **Temporal modeling**
   - Use sequences of readings over hours/days
   - LSTM or Transformer on feature sequences
   - Detect trends (swarming preparation over days)

5. **Multi-task learning**
   - Predict queen status + confidence score
   - Joint training with anomaly detection

6. **Better audio preprocessing**
   - Noise reduction (wind, rain, traffic)
   - Voice activity detection equivalent for bees

### Research Questions

- Can we detect swarming intent before it happens?
- How early can queenlessness be detected?
- Do different bee subspecies have distinct audio signatures?
- Can we distinguish between virgin and mated queens?

---

## Contributing

We welcome contributions from ML researchers! Here's how to help:

### Data Contributions

If you have labeled bee audio recordings:
1. Contact us via GitHub Issues
2. Include metadata: hive type, location, date, queen status verification method
3. Audio format: WAV, 16-bit, 22,050 Hz or higher

### Model Improvements

1. Fork the repository
2. Document your approach and results
3. Ensure reproducibility (random seeds, dependencies)
4. Compare against baseline (XGBoost 78.8%)
5. Submit a PR with your findings

### Evaluation Protocol

When reporting results, include:
- Accuracy, F1, Precision, Recall (macro-averaged)
- Per-class metrics
- Confusion matrix
- Training time and inference time
- Model size

---

## References

### Primary Research

1. **Chen et al. (2024)** - "A machine learning-based multiclass classification model for bee colony anomaly identification using an IoT-based audio monitoring system with an edge computing framework"
   - DOI: https://doi.org/10.1016/j.eswa.2024.124898
   - Achieved 98.8% accuracy with controlled experimental data
   - Expert Systems with Applications journal

### Dataset

2. **Kaggle Smart Bee Colony Monitor**
   - https://www.kaggle.com/datasets/annajyang/smart-bee-colony-monitor

### Related Work

3. **Bee audio classification literature**:
   - Frequency analysis of bee sounds (200-500 Hz fundamental)
   - Queen piping detection (specific frequency patterns)
   - Swarming sound signatures

### Tools & Libraries

- [librosa](https://librosa.org/) - Audio feature extraction
- [XGBoost](https://xgboost.readthedocs.io/) - Gradient boosting
- [scikit-learn](https://scikit-learn.org/) - ML utilities
- [m2cgen](https://github.com/BayesWitnesses/m2cgen) - Model to C code

---

## Contact

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Research questions and collaboration
- **Website**: [buzzhivetech.com](https://buzzhivetech.com)

---

*Last updated: February 2026*
