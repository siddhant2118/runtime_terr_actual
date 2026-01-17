# ESP32 Server - Sheldon Rover

This folder contains the ESP32 Arduino code that acts as the robot's brain and Wi-Fi server.

## What it does

- Creates Wi-Fi Access Point: **SheldonRover** (password: `bazinga123`)
- Hosts HTTP API at `http://192.168.4.1`
- Controls motors via joystick commands
- Detects collisions and stores events

## HTTP Endpoints

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/status` | GET | Check if ESP32 is online |
| `/event` | GET | Get current event (iPhone 1 polls this) |
| `/event` | POST | Inject event manually (from remote) |
| `/mode` | POST | Set intensity mode |
| `/stop` | POST | Emergency stop all motors |
| `/say` | POST | Request specific audio clip |
| `/move` | POST | Send joystick movement |

## Upload Instructions

### Option 1: Arduino IDE

1. Open `esp32_server.ino` in Arduino IDE
2. Install ESP32 board (Tools > Board > Boards Manager > search "esp32")
3. Install ArduinoJson library (Tools > Manage Libraries > search "ArduinoJson")
4. Select board: **DOIT ESP32 DEVKIT V1**
5. Select port: `/dev/cu.usbserial-10`
6. Click Upload

### Option 2: PlatformIO

```bash
cd esp32-server
pio run -t upload
```

## Wiring

| Component | ESP32 Pin |
|-----------|-----------|
| Left Motor FWD | GPIO 26 |
| Left Motor BCK | GPIO 25 |
| Right Motor FWD | GPIO 33 |
| Right Motor BCK | GPIO 32 |
| Ultrasonic Trig | GPIO 13 |
| Ultrasonic Echo | GPIO 12 |
| Bump Switch L | GPIO 14 |
| Bump Switch R | GPIO 27 |

## Connect from iPhone

1. Connect iPhone to WiFi: **SheldonRover**
2. Password: `bazinga123`
3. Open Safari: `http://192.168.4.1` (or use remote control page)
