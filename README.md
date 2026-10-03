# SafeStep AI 🚶‍♂️

### AI-Enabled Smart Cane for Obstacle and Wet-Floor Detection

SafeStep AI is a smart cane prototype designed to provide additional environmental awareness for visually impaired users by detecting **physical obstacles** and **wet-floor conditions**.

The prototype uses an **ESP32**, **HC-SR04 ultrasonic sensor**, **capacitive moisture sensor**, and **vibration motor** to detect hazards and provide tactile alerts.

---

## 🎯 Problem

Visually impaired users may have difficulty identifying obstacles and wet or slippery surfaces while walking.

Traditional walking canes primarily rely on physical contact. SafeStep AI adds electronic sensing to provide an additional layer of hazard awareness through vibration feedback.

---

## 💡 Solution

SafeStep combines multiple sensors with an ESP32 to detect environmental hazards.

```text
        HC-SR04
            │
            │ Distance
            ↓
     ┌──────────────┐
     │              │
     │    ESP32     │
     │              │
     └──────────────┘
            ↑
            │ Moisture
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

---

## 🔧 Hardware Components

| Component                  | Purpose                |
| -------------------------- | ---------------------- |
| ESP32 WROOM-32 DevKit      | Main controller        |
| HC-SR04 Ultrasonic Sensor  | Obstacle detection     |
| Capacitive Moisture Sensor | Wet/moisture detection |
| 5V PWM Vibration Motor     | Tactile hazard alerts  |
| PVC Pipe / Cane Body       | Prototype structure    |

---

## ⚙️ System Working

### 1. Obstacle Detection

The HC-SR04 measures the distance between the cane and nearby objects using ultrasonic waves.

The ESP32 processes the measured distance and identifies an obstacle when it falls within the configured detection range.

### 2. Wet-Floor Detection

The capacitive moisture sensor measures moisture near the tip of the cane.

The ESP32 processes the sensor reading to determine the current surface condition, such as dry or potentially wet.

### 3. Hazard Alert

Based on the detected condition, the ESP32 controls the vibration motor to provide tactile feedback.

```text
Obstacle detected  → Short vibration
Wet floor detected → Long vibration
No hazard          → No vibration
```

---

## 📟 Prototype Output

The ESP32 provides real-time sensor and system information through the Arduino IDE Serial Monitor.

The output can include:

* Ultrasonic distance
* Obstacle status
* Moisture sensor reading
* Surface condition
* Risk score
* Hazard classification
* Vibration pattern
* Wi-Fi connection status
* Backend communication status

### Example Output

```text
Distance: 3.93 cm
Obstacle: YES
Moisture Raw: 3361
Moisture Percent: 5%
Surface: DRY
Risk Score: 60
AI Decision: MEDIUM_RISK
Hazard: OBSTACLE_DETECTED
Vibration Pattern ID: 0
WiFi: CONNECTED
```

The prototype can also send sensor and hazard information from the ESP32 to a backend through HTTP when Wi-Fi is available.

![SafeStep AI Serial Monitor Output](images/serial-monitor-output.png)

---

## 🌐 IoT Connectivity

The ESP32 supports Wi-Fi connectivity and HTTP communication with a backend system.

This enables sensor and hazard data from the cane to be transmitted for web-based monitoring and future system integration.

---

## 🧠 AI / ML Development

The **current prototype uses sensor readings and threshold-based logic for hazard detection**. It does not claim a machine-learning model as part of the current implementation unless one is actually deployed in the code.

A future version can use machine learning to improve surface and hazard classification by combining multiple sensor readings.

```text
Sensor Data
     │
     ↓
Feature Extraction
     │
     ↓
ML Model
     │
     ├── Dry
     ├── Damp
     └── Hazardous Wet
```

A lightweight **Random Forest model** can be developed and later converted for **TinyML deployment on the ESP32**.

---

## 🛠️ Current Features

* ESP32-based smart cane prototype
* Ultrasonic obstacle detection
* Capacitive moisture sensing
* Vibration-based tactile alerts
* Different alert patterns for detected hazards
* Wi-Fi connectivity
* HTTP-based backend communication
* Portable prototype design

---

## 🚀 Future Development

* Machine-learning-based hazard classification
* TinyML deployment on ESP32
* Improved wet-floor classification
* Temperature and humidity sensing
* Adaptive sensor thresholds
* More distinct vibration patterns
* Improved physical cane integration
* Enhanced web monitoring and visualization

---

## 📁 Project Structure

```text
SafeStep-AI/
│
├── SafeStep_AI.ino
└── README.md
```

`SafeStep_AI.ino` contains the ESP32 firmware for the current prototype.

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
* HTTP

**Future AI/ML**

* Python
* Scikit-learn
* Random Forest
* TinyML

---

## 🎓 Project Objective

SafeStep AI aims to enhance traditional cane-based mobility assistance by combining **embedded systems, sensor technology, IoT connectivity, and future machine learning** to provide additional environmental hazard awareness through tactile feedback.

---

## ⭐ About

SafeStep AI is an academic prototype exploring the application of **AI, IoT, embedded systems, and sensor technology** to accessibility and mobility assistance.
