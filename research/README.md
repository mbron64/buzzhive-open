# Buzzhive ML Research

ML training pipeline for bee colony queen status classification. Reproduces the results described in [RESEARCH.md](../RESEARCH.md).

## Quick Start

```bash
cd research
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Then open the notebooks in order:

1. **`01_data_exploration.ipynb`** — Dataset analysis and visualization
2. **`02_feature_engineering.ipynb`** — MFCC feature extraction from audio
3. **`03_cnn_classifier.ipynb`** — CNN-based classifier
4. **`04_vae_anomaly_detector.ipynb`** — VAE anomaly detection model
5. **`05_hybrid_evaluation.ipynb`** — Hybrid CNN+VAE evaluation
6. **`06_traditional_ml_classifier.ipynb`** — XGBoost / Random Forest / SVM (78.8% accuracy)
7. **`07_model_comparison.ipynb`** — Side-by-side comparison of all approaches

## Dataset

Download the [Smart Bee Colony dataset](https://www.kaggle.com/datasets/annajyang/beehive-sounds) from Kaggle and place it in a `data/` directory (gitignored).

## Outputs

Pre-computed outputs are included:

- **`outputs/models/`** — Trained models (XGBoost JSON, CNN/VAE PyTorch, sklearn joblib, scaler params)
- **`outputs/processed/`** — Train/val/test splits with extracted features
- **`outputs/*.png`** — Figures (confusion matrices, training curves, feature importance, etc.)

## Key Result

XGBoost with 78 MFCC features achieves **78.8% accuracy** on 4-class queen status classification (queenright, queenless, new queen, queen accepted). See notebook `06` for details.
