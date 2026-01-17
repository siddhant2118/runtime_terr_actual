# Layer C: Intel Galileo Navigation

This component runs on the Intel Galileo board and handles navigation and event detection.

## Responsibilities
1.  **Navigation**: Controls motors to move the rover.
2.  **Event Detection**: Monitors sensors for collisions, stalls, or idle states.
3.  **Communication**: Sends event strings to the ESP32 (Layer B) via Serial1.

## Event Protocol
Sent over Serial1 (9600 baud), newline terminated:
- `BOOT`: Startup
- `MOVE_START`: Movement begins
- `MOVE_STOP`: Movement ends
- `COLLISION`: Physical obstacle hit
- `STUCK`: No progress despite movement
- `IDLE_TOO_LONG`: Inactive for >10s
- `RESET`: Manual reset

## Hardware Assumptions
- **Motors**: PWM controlled DC motors on standardized pins (see `include/config.h`).
- **Sensors**: 
    - Ultrasonic rangefinder (trig/echo)
    - Bump switches (digital input)
    - Encoders (interrupt/digital input)

## Build & Run
**Use the Arduino IDE.**

### A. Arduino Uno (Recommended)
1.  Open `galileo_nav/galileo_nav.ino` in Arduino IDE.
2.  Select **Tools > Board > Arduino Uno**.
3.  **Wiring Change**:
    *   Connect **Pin 2** to the **ESP32 TX**.
    *   Connect **Pin 3** to the **ESP32 RX**.
    *   *(This uses SoftwareSerial so you can still debug via USB)*.
4.  Upload.

### B. Intel Galileo (Legacy)
1.  See `SETUP_AND_UPLOAD.md` for special installation instructions.
2.  Select **Tools > Board > Intel Galileo Gen2**.
3.  Upload.

### ⚠️ Trouble finding the Board?
Since Intel discontinued the Galileo, it doesn't show up by default. You **MUST** add this rescue URL:
1.  Open **Arduino > Settings** (or File > Preferences).
2.  In **Additional Boards Manager URLs**, paste this exact link:
    `https://raw.githubusercontent.com/maxgerhardt/arduino-Intel-i586-revive/refs/heads/main/package_inteli586_index.json`
3.  Click OK.
4.  Go to **Tools > Board > Boards Manager...**
5.  Search for **"Intel i586"** and click Install.


## Debug / Simulation
You can open the Serial Monitor (115200 baud).
- Type `c` and hit enter to simulate a **COLLISION** event.
