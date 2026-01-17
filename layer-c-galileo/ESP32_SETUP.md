# ESP32 Setup Guide

Running the Navigation Logic directly on the ESP32!

## 1. Wiring (ESP32 Pins)
The wiring is DIFFERENT from Arduino. Use these pins:

| Component | ESP32 Pin | Logic |
| :--- | :--- | :--- |
| **Left Motor FWD** | GPIO **26** | PWM |
| **Left Motor BCK** | GPIO **25** | Digital |
| **Right Motor FWD** | GPIO **33** | PWM |
| **Right Motor BCK** | GPIO **32** | Digital |
| **Ultrasonic Trig** | GPIO **13** | Output |
| **Ultrasonic Echo** | GPIO **12** | Input |
| **Bump Switch L** | GPIO **14** | Input Pullup |
| **Bump Switch R** | GPIO **27** | Input Pullup |
| **Reset Button** | **BOOT** Btn | (Built-in) |

*Power Warning*: Power the L298N Motor Driver with batteries. Do NOT power motors from the ESP32 5V/3V pin.

## 2. Arduino IDE Setup
1.  **Install ESP32 Board Manager**:
    *   Go to **Arduino Settings**.
    *   Add URL: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
    *   Go to **Tools > Board > Boards Manager**, search `esp32` and install "esp32 by Espressif Systems".

2.  **Select Board**:
    *   **Tools > Board > esp32 > DOIT ESP32 DEVKIT V1** (or your specific model).

3.  **Upload**:
    *   Open `galileo_nav.ino`.
    *   Plug in ESP32.
    *   Select Port.
    *   Upload.
    *   *(Note: If upload fails, hold the BOOT button on the ESP32 when you see "Connecting..." in the console)*.

## 3. Usage
Open Serial Monitor at **115200**. You will see:
`BOOT`
`MOVE_START`
... etc.
