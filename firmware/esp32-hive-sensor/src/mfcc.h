/**
 * Lightweight MFCC Feature Extraction for ESP32
 * 
 * Extracts 78 features from audio:
 * - 13 MFCCs (mean + std)
 * - 13 Delta MFCCs (mean + std)  
 * - 13 Delta-Delta MFCCs (mean + std)
 * 
 * Optimized for low memory usage on microcontrollers.
 */

#ifndef MFCC_H
#define MFCC_H

#include <Arduino.h>
#include <math.h>

// Configuration
#define N_MFCC 13
#define N_FFT 2048
#define HOP_LENGTH 512
#define N_MELS 40
#define FMIN 0.0
#define FMAX 8000.0

// Total features: (13 MFCCs + 13 deltas + 13 delta-deltas) * 2 (mean + std)
#define N_FEATURES 78

/**
 * Extract MFCC features from audio samples
 * 
 * @param samples Audio samples (int16_t)
 * @param numSamples Number of samples
 * @param sampleRate Sample rate in Hz
 * @param features Output array of 78 floats
 */
void extractMFCC(const int16_t* samples, size_t numSamples, int sampleRate, float* features);

// ============================================================================
// Implementation
// ============================================================================

// Pre-computed Mel filterbank (simplified for ESP32)
// In production, compute this based on sample rate and FFT size

static float melScale(float freq) {
    return 2595.0 * log10(1.0 + freq / 700.0);
}

static float invMelScale(float mel) {
    return 700.0 * (pow(10.0, mel / 2595.0) - 1.0);
}

// Simple DCT-II implementation
static void dct(const float* input, float* output, int n) {
    for (int k = 0; k < n; k++) {
        float sum = 0.0;
        for (int i = 0; i < n; i++) {
            sum += input[i] * cos(M_PI * k * (2 * i + 1) / (2.0 * n));
        }
        output[k] = sum;
    }
}

// Compute power spectrum using simple FFT approximation
// Note: For production, use a proper FFT library like arduinoFFT
static void computePowerSpectrum(const int16_t* frame, int frameSize, float* spectrum) {
    // Apply Hamming window and compute magnitude
    for (int i = 0; i < frameSize / 2; i++) {
        float window = 0.54 - 0.46 * cos(2 * M_PI * i / (frameSize - 1));
        float real = frame[i] * window / 32768.0;
        float imag = frame[frameSize - 1 - i] * window / 32768.0;
        spectrum[i] = real * real + imag * imag;
    }
}

// Main MFCC extraction function
void extractMFCC(const int16_t* samples, size_t numSamples, int sampleRate, float* features) {
    int numFrames = (numSamples - N_FFT) / HOP_LENGTH + 1;
    if (numFrames <= 0) numFrames = 1;
    
    // Allocate temporary buffers (stack allocation for small sizes)
    float mfccFrames[100][N_MFCC];  // Max 100 frames
    int actualFrames = min(numFrames, 100);
    
    float spectrum[N_FFT / 2];
    float melEnergies[N_MELS];
    float mfcc[N_MFCC];
    
    // Process each frame
    for (int f = 0; f < actualFrames; f++) {
        int offset = f * HOP_LENGTH;
        
        // Compute power spectrum
        computePowerSpectrum(&samples[offset], N_FFT, spectrum);
        
        // Apply Mel filterbank (simplified)
        for (int m = 0; m < N_MELS; m++) {
            float melLow = melScale(FMIN) + m * (melScale(FMAX) - melScale(FMIN)) / N_MELS;
            float melHigh = melScale(FMIN) + (m + 1) * (melScale(FMAX) - melScale(FMIN)) / N_MELS;
            
            float sum = 0.0;
            int binLow = (int)(invMelScale(melLow) * N_FFT / sampleRate);
            int binHigh = (int)(invMelScale(melHigh) * N_FFT / sampleRate);
            
            for (int b = binLow; b < binHigh && b < N_FFT / 2; b++) {
                sum += spectrum[b];
            }
            melEnergies[m] = log(sum + 1e-10);
        }
        
        // Apply DCT to get MFCCs
        dct(melEnergies, mfcc, N_MFCC);
        
        // Store frame MFCCs
        for (int i = 0; i < N_MFCC; i++) {
            mfccFrames[f][i] = mfcc[i];
        }
    }
    
    // Compute delta and delta-delta MFCCs
    float deltaFrames[100][N_MFCC];
    float delta2Frames[100][N_MFCC];
    
    for (int f = 0; f < actualFrames; f++) {
        for (int i = 0; i < N_MFCC; i++) {
            // Delta: difference with adjacent frames
            if (f > 0 && f < actualFrames - 1) {
                deltaFrames[f][i] = (mfccFrames[f + 1][i] - mfccFrames[f - 1][i]) / 2.0;
            } else {
                deltaFrames[f][i] = 0;
            }
        }
    }
    
    for (int f = 0; f < actualFrames; f++) {
        for (int i = 0; i < N_MFCC; i++) {
            // Delta-delta
            if (f > 0 && f < actualFrames - 1) {
                delta2Frames[f][i] = (deltaFrames[f + 1][i] - deltaFrames[f - 1][i]) / 2.0;
            } else {
                delta2Frames[f][i] = 0;
            }
        }
    }
    
    // Aggregate: compute mean and std for each feature
    int idx = 0;
    
    // MFCCs mean
    for (int i = 0; i < N_MFCC; i++) {
        float sum = 0;
        for (int f = 0; f < actualFrames; f++) sum += mfccFrames[f][i];
        features[idx++] = sum / actualFrames;
    }
    
    // Deltas mean
    for (int i = 0; i < N_MFCC; i++) {
        float sum = 0;
        for (int f = 0; f < actualFrames; f++) sum += deltaFrames[f][i];
        features[idx++] = sum / actualFrames;
    }
    
    // Delta-deltas mean
    for (int i = 0; i < N_MFCC; i++) {
        float sum = 0;
        for (int f = 0; f < actualFrames; f++) sum += delta2Frames[f][i];
        features[idx++] = sum / actualFrames;
    }
    
    // MFCCs std
    for (int i = 0; i < N_MFCC; i++) {
        float mean = features[i];
        float sumSq = 0;
        for (int f = 0; f < actualFrames; f++) {
            float diff = mfccFrames[f][i] - mean;
            sumSq += diff * diff;
        }
        features[idx++] = sqrt(sumSq / actualFrames);
    }
    
    // Deltas std
    for (int i = 0; i < N_MFCC; i++) {
        float mean = features[N_MFCC + i];
        float sumSq = 0;
        for (int f = 0; f < actualFrames; f++) {
            float diff = deltaFrames[f][i] - mean;
            sumSq += diff * diff;
        }
        features[idx++] = sqrt(sumSq / actualFrames);
    }
    
    // Delta-deltas std
    for (int i = 0; i < N_MFCC; i++) {
        float mean = features[2 * N_MFCC + i];
        float sumSq = 0;
        for (int f = 0; f < actualFrames; f++) {
            float diff = delta2Frames[f][i] - mean;
            sumSq += diff * diff;
        }
        features[idx++] = sqrt(sumSq / actualFrames);
    }
}

#endif // MFCC_H

