# Generating Synthesis Sensor Data for Anomaly Detection

This script will:

1. Automatically generate synthetic sensor data (normal and anomalous).
2. Train the TinyML Neural Network.
3. Quantize and convert the model to TensorFlow Lite.
4. Automatically convert the model into the C-header file (dht_anomaly_model.h) and download it to your computer

## Data

- The temperature: Normal data is between 20-28°C, while anomalous data is between 32-45°C.
- The humidity: Normal data is between 30-60%, while anomalous data is between 80-100%.
- Normal data is labeled as 0, and anomalous data is labeled as 1.

## Usage

1. Run this script with Google Colab or any Python environment with TensorFlow installed. *This script is designed to run in Google Colab for ease of use and access to TensorFlow libraries.*
2. Once executed, wait for the model to train and convert. The generated `dht_anomaly_model.h` file will be available for download at the end of the script execution.
3. Copy the downloaded `dht_anomaly_model.h` file into `/include/` and `/src/` directories.
