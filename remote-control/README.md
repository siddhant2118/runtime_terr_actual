# Remote Control Web UI (iPhone 2)

Mobile-friendly web interface for controlling the Sheldon Rover.

## Features

- 🛑 **Emergency Stop** - Big red button at top
- 🎚️ **Mode Selector** - LOW / MID / HIGH intensity
- 💥 **Event Buttons** - Trigger COLLISION, STUCK, RESET, BOOT, etc.
- 🔊 **Audio Selector** - Pick specific clips from database
- 🕹️ **Joystick** - Movement control

## Usage

### Option 1: Open directly on iPhone
1. Connect iPhone to same Wi-Fi as ESP32
2. Open Safari
3. Navigate to: `file:///path/to/index.html`

### Option 2: Serve via ESP32
Upload to ESP32's SPIFFS/LittleFS and serve at `/remote`

### Option 3: Local server (testing)
```bash
cd remote-control
python3 -m http.server 8080
```
Then open `http://localhost:8080` on phone

## ESP32 Endpoints Required

| Endpoint | Method | Body | Purpose |
|----------|--------|------|---------|
| `/event` | POST | `{"event": "COLLISION"}` | Inject event |
| `/mode` | POST | `{"mode": "HIGH"}` | Set intensity |
| `/stop` | POST | -- | Emergency stop |
| `/say` | POST | `{"id": "RANDOM_2_0"}` | Play specific audio |
| `/move` | POST | `{"x": 50, "y": 100}` | Joystick movement |
| `/status` | GET | -- | Check connection |

## Screenshots

Open `index.html` in browser to preview!
