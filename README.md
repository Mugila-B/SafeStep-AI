# SafeStep AI 🚶‍♂️

### AI-Enabled Smart Cane for Obstacle and Wet-Floor Detection

SafeStep AI is a smart cane prototype designed to assist visually impaired users by detecting **physical obstacles** and **wet-floor conditions** while walking.

The system uses an **ESP32** as the main controller, an **HC-SR04 ultrasonic sensor** for obstacle detection, and a **capacitive moisture sensor** for detecting wet surfaces. A vibration motor provides tactile alerts to the user based on the detected hazard.

---

## 🎯 Problem

Visually impaired people may face difficulty identifying obstacles and wet or slippery surfaces while walking.

Traditional walking canes mainly depend on physical contact with obstacles. SafeStep AI aims to provide an additional layer of electronic hazard detection and vibration-based feedback.

---

## 💡 Solution

SafeStep AI combines multiple sensors with an ESP32 to monitor the environment around the cane.

```text
        HC-SR04
           │
           │ Obstacle Distance
           ↓
    ┌────────────────┐
    │                │
    │     ESP32      │
    │                │
    └────────────────┘
           ↑
           │ Moisture Level
           │
   Capacitive Moisture
       Sensor

           │
           ↓
    Hazard Detection
           │
           ↓
     Vibration Motor
           │
           ↓
          User
```

The ESP32 reads the sensor values and determines whether a potential obstacle or wet-floor condition is present. The vibration motor then provides a tactile warning.

---

## 🔧 Hardware Components

| Component                  | Purpose                            |
| -------------------------- | ---------------------------------- |
| ESP32 WROOM-32 DevKit      | Main microcontroller               |
| HC-SR04 Ultrasonic Sensor  | Detects physical obstacles         |
| Capacitive Moisture Sensor | Detects wet/moist floor conditions |
| 5V PWM Vibration Motor     | Provides tactile hazard alerts     |
| PVC Pipe / Cane Body       | Physical prototype structure       |

---

## ⚙️ How It Works

### 1. Obstacle Detection

The HC-SR04 ultrasonic sensor sends an ultrasonic pulse and measures the time taken for the reflected signal to return.

The ESP32 uses this measurement to estimate the distance to an object.

If an obstacle is detected within the configured detection range, the system generates an obstacle warning.

### 2. Wet-Floor Detection

The capacitive moisture sensor measures the moisture level near the tip of the cane.

The ESP32 reads the sensor value and uses it to identify a potential wet-floor condition.

### 3. ESP32 Processing

The ESP32 acts as the central controller.

It receives readings from both sensors and processes them to determine the current hazard condition.

### 4. Vibration Alert

A vibration motor provides tactile feedback to the user.

Different vibration patterns can be used to distinguish between different hazard conditions.

For example:

```text
Obstacle detected  → Short vibration
Wet floor detected → Long vibration
No hazard          → No vibration
```

---

## 🧠 AI / ML Development

The current prototype primarily uses **sensor-based hazard detection and threshold logic**.

A future version of SafeStep AI can use machine learning to improve hazard classification by combining sensor readings and environmental information.

Potential future classification:

```text
Sensor Data
     │
     ↓
Feature Extraction
     │
     ↓
Machine Learning Model
     │
     ├── Dry
     ├── Damp
     └── Hazardous Wet
```

A possible future implementation is a lightweight **Random Forest / TinyML model** trained using sensor data and deployed on the ESP32.

The ML component is considered a **future development stage** and is not claimed as part of the current hardware prototype unless implemented in the code.

---

## 🌐 IoT Connectivity

The ESP32 can connect to Wi-Fi and communicate sensor/hazard information to a web backend.

This provides the possibility of monitoring the detected conditions remotely and integrating SafeStep with a web-based interface.

The current repository contains the **ESP32 firmware** for the hardware prototype.

---

## 🛠️ Current Features

* ESP32-based smart cane prototype
* Ultrasonic obstacle detection
* Capacitive moisture sensing
* Vibration-based user alerts
* Separate hazard indication through vibration patterns
* Wi-Fi connectivity
* Prototype designed for integration with a web-based monitoring system
* Portable battery-powered design

---

## 🚀 Future Development

* Machine-learning-based hazard classification
* TinyML deployment directly on ESP32
* Improved wet-floor classification under changing environmental conditions
* Temperature and humidity sensing
* Adaptive sensor thresholds
* More distinct vibration patterns
* Improved physical enclosure and cane integration
* Enhanced web-based monitoring and visualization

---

## 📁 Project Structure

```text
SafeStep-AI/
│
├── SafeStep_AI.ino
└── README.md
```

`SafeStep_AI.ino` contains the main ESP32 firmware for the current prototype.

---

## 🔬 Technology Stack

**Hardware**

* ESP32
* HC-SR04
* Capacitive Moisture Sensor
* Vibration Motor

**Programming**

* C/C++
* Arduino IDE

**Connectivity**

* Wi-Fi
* HTTP communication

**Future AI/ML**

* Python
* Scikit-learn
* Random Forest
* TinyML

---

## 🎓 Project Objective

The objective of SafeStep AI is to enhance traditional cane-based mobility assistance by combining **embedded systems, sensors, IoT connectivity, and future machine learning techniques** to provide additional environmental hazard awareness through tactile feedback.

---


## ⭐ About This Project

SafeStep AI is developed as an academic/project prototype exploring the use of **Artificial Intelligence, IoT, embedded systems, and sensor technology** to address real-world accessibility challenges.
