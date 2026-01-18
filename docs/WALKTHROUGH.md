# Sheldon Rover User Guide

## 1. Connect
1.  Turn on ESP32.
2.  Connect both iPhones to `SheldonRover` WiFi (pass: `bazinga123`).

## 2. Setup Screen
*   **Face Phone**: Go to `http://192.168.4.1/voice`. Tap **Start**.
*   **Remote Phone (LCARS)**: Go to `http://192.168.4.1`. You will see the **Star Trek PADD** interface.

## 3. Controls
*   **MANUAL Mode**: Use the Joystick (middle orange circle) to drive.
*   **AUTO Mode**: Tap "AUTO" (top right). Robot will drive itself and avoid walls.
*   **Audio**: Tap the LCARS buttons (Collision, Bazinga, etc) to speak.

# Walkthrough - Final Stable Build

## Overview
We have successfully deployed a **Stable Firmware** that balances robust motor control with a full-featured Web Interface.

## Key Features Implemented
### 1. Motor Safety & Stability (Soft Start)
- **Soft Start Logic**: Reduced inrush current to prevent brownouts.
- **Coast Delay**: Added delays between direction changes to protect the H-Bridge and battery.
- **Event-Driven Motion**: Motors move only on specific events, reducing idle power draw.

### 2. Web & App Integration
- **Full Web UI**: Restored `index.html` with Mode Toggles (Auto/Manual) and Audio Dropdown.
- **App Compatibility**: Fixed API (`POST /event`, `GET /detect`) so the external App works seamlessly.
- **Strict Manual Mode**: Enforced logic where "Manual Drive" completely blocks auto-maneuvers for safety.

## Verification
- **Upload**: Confirmed successful upload of the merged firmware.
- **Git**: Codebase pushed to repository with the message "Final Stable Build: Safe Motor Logic + Web UI".
- **Hardware Note**: User advised to use LiPo battery if "random" movements persist (due to low voltage).

## Usage
1. **Power On**: Robot starts in **AUTO** mode (Drive & Voice).
2. **Web Control**: Access `http://192.168.4.1` to toggle modes or play audio.
3. **App Control**: App can now trigger `POST /event` (Manual Manual) or `GET /detect` (Auto).

---
## Project History
*   **Override**: If you touch the Joystick while in AUTO, it correctly switches to MANUAL immediately.
