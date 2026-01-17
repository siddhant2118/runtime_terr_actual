# Sheldon Rover 🤖

A sitcom-character robot that reacts to failures with Sheldon Cooper's personality.

## 🚀 Quick Start

### iPhone Controls (GitHub Pages)

| Device | Purpose | Link |
|--------|---------|------|
| **iPhone 2** | Remote Control | [Open Remote](https://siddhant2118.github.io/runtime_terr_actual/) |
| **iPhone 1** | Character Voice | [Open Voice](https://siddhant2118.github.io/runtime_terr_actual/character.html) |

### How to Use

1. **Power on ESP32** - Creates WiFi: `SheldonRover` (password: `bazinga123`)
2. **Connect both iPhones** to `SheldonRover` WiFi
3. **Open the links above** on each iPhone
4. Use Remote to trigger events → Character speaks!

---

## Project Structure

```
├── esp32-server/          # ESP32 WiFi server code
├── remote-control/        # iPhone 2 - Remote control UI
├── Preset Sheldon/        # iPhone 1 - Character engine & audio
├── docs/                  # GitHub Pages deployment
└── layer-b-esp32/         # MicroPython character engine
```

## ESP32 Endpoints

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/status` | GET | Check if online |
| `/event` | GET/POST | Get/inject events |
| `/stop` | POST | Emergency stop |
| `/say` | POST | Play specific audio |
| `/move` | POST | Joystick movement |

---

## Team

- **Layer A** - Character Logic & Voice (Manya)
- **Layer B** - ESP32 Server & Remote (Siddhant)
- **Layer C** - Robot Navigation