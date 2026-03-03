# Vacuum Clogging dettection Device
Smart-Vac is an IoT system using an ESP32 and MPU6050 sensor to monitor vacuum health. It employs an Embedded AI model to detect blockages through vibration analysis, triggering real-time OLED feedback and WhatsApp alerts via Node-RED. This project demonstrates full-stack integration of hardware, machine learning, and cloud logic.

#  Smart-Vac: AI-Based Vacuum Clogging Detection System

Smart-Vac is an IoT-enabled predictive maintenance system that detects vacuum cleaner blockages using vibration pattern analysis.

The system combines:

- ESP32 microcontroller
- MPU6050 vibration sensor
- Embedded TinyML model
- OLED real-time display
- Node-RED cloud logic
- WhatsApp alert system

This project demonstrates full-stack integration of Embedded AI, IoT, and Cloud Automation.

---

## 🚀 Features

- Real-time vibration monitoring
- AI-based clog detection
- OLED health status display
- Instant WhatsApp alerts
- Cloud-based monitoring via Node-RED
- Edge AI inference on ESP32

---

## System Architecture

1. MPU6050 captures vibration signals
2. ESP32 processes data
3. Embedded AI model predicts blockage
4. OLED displays status
5. Node-RED triggers WhatsApp alert

---

##  Hardware Components

- ESP32 Dev Module
- MPU6050 Accelerometer/Gyroscope
- 0.96" I2C OLED Display
- Power Supply
- Jumper Wires

---

##  Machine Learning Pipeline

1. Collect vibration dataset
2. Label: Normal vs Blocked
3. Train model (TensorFlow/Keras)
4. Convert to TensorFlow Lite
5. Deploy as C header file on ESP32

---

##  Installation

###  Firmware Setup

- Install Arduino IDE
- Install ESP32 board package
- Install required libraries:
  - Wire.h
  - Adafruit_GFX
  - Adafruit_SSD1306
  - MPU6050
  - TensorFlowLite_ESP32

Upload firmware from:
