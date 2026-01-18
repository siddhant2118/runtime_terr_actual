# 🤖 Sheldon Rover (ESP32)

> **"Bazinga!"** - A personality-driven, web-controlled rover powered by ESP32.

![ESP32](https://img.shields.io/badge/Platform-ESP32-blue?logo=espressif)
![Status](https://img.shields.io/badge/Status-Stable-success)
![License](https://img.shields.io/badge/License-MIT-green)

The **Sheldon Rover** is an interactive robot platform that combines autonomous exploration with manual web control. It features "Soft Start" motor logic for battery safety, a dual-mode control system (Auto/Manual), and a personality engine that triggers audio events.

---

## 🚀 Features

*   **🕹️ Dual Control Modes:**
    *   **Drive:** Switch between **Smart Auto** (Camera Tracking) and **Safe Manual** (Joystick Only).
    *   **Voice:** Switch between **Auto-Banter** (Reacts to people) and **Manual Triggers**.
*   **🔊 Audio Core:**
    *   Plays "Bazinga" and other clips stored in Flash Memory (LittleFS).
    *   Smart Debouncing (No looping audio).
*   **🔋 Soft-Start Physics:**
    *   Motors ramp up gently to prevent brownouts.
    *   Coast delays protect the H-Bridge and battery.
*   **💻 Clean Web UI:**
    *   A modern, dark-themed control panel hosted directly on the robot.

---

## 📂 Project Structure

```bash
├── 📁 data/            # Web UI (HTML) & Audio Files (.mp3)
├── 📁 docs/            # Documentation & Guides
│   ├── API_REFERENCE.md    # API for App Developers
│   ├── WIRING.md           # Wiring Digram
│   └── WALKTHROUGH.md      # Features Overview
├── 📁 examples/        # Hardware Diagnostic Scripts
├── 📁 src/             # Main Firmware (C++)
└── platformio.ini      # Build Configuration
```

---

## 🛠️ Quick Start

### 1. Hardware Setup
Connect your ESP32 + L298N Driver using the verified pin map:
*   **Left Motor:** Pins 4 & 5
*   **Right Motor:** Pins 6 & 7
*   *(See [docs/WIRING.md](docs/WIRING.md) for full guide)*

### 2. Flash Firmware
Using **PlatformIO**:
```bash
# 1. Upload the File System (Web UI + Audio)
pio run --target uploadfs

# 2. Upload the Firmware
pio run --target upload
```

### 3. Connect
1.  Connect to Wi-Fi: `SheldonRover` (Password: `bazinga123`)
2.  Open Browser: `http://192.168.4.1`

---

## 📡 API Endpoints

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `POST` | `/move` | Drive Manually (`x`, `y`) |
| `POST` | `/event` | Trigger Audio (`{"event": "bazinga"}`) |
| `POST` | `/mode` | Set Modes (`{"drive": "AUTO"}`) |
| `GET` | `/status` | Check Mode & Health |

*(See [docs/API_REFERENCE.md](docs/API_REFERENCE.md) for full details)*

---

## 🔋 Hardware Notice
If you experience **brownouts** (ESP32 restarting when motors move):
1.  Ensure you are **NOT** using a standard 9V battery (too weak).
2.  Use a **2S LiPo (7.4V)** or high-current AA pack.
3.  Check the "Soft Start" logic in `src/main.cpp`.

---

*Verified Stable Build: Jan 2026*
