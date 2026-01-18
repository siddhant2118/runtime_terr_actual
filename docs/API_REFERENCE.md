# ESP32 Robot API Guide (For App Developer)

**Base URL:** `http://192.168.4.1` (Default AP IP)

## 1. Manual Audio Triggers (Buttons)
**Endpoint:** `POST /event`
**Header:** `Content-Type: application/json`
**Body:**
```json
{
  "event": "collision" 
}
```
*   **Use this for:** Manual buttons in your App (Collision, Stuck, Bazinga).
*   **Why:** These events Bypass the 5-second cooldown used for auto-detection.
*   **Supported Events:** `collision`, `stuck`, `random`, `saw_human`, `stop`.
*   **Play Specific File:** `{"event": "SAY:filename.mp3"}` (e.g. `saw_human_1.mp3`).

## 2. Auto-Detection (Camera Logic)
**Endpoint:** `GET /detect?type=saw_human`
*   **Use this for:** When your computer vision detects a person.
*   **Behavior:** The ESP32 enforces a **5-Second Cooldown** on this endpoint to prevent audio looping/spamming.
*   **Note:** If `voiceMode` is set to "MANUAL", this request will be **IGNORED** by the robot.

## 3. Mode Switching
**Endpoint:** `POST /mode`
**Header:** `Content-Type: application/json`
**Body:**
```json
{
  "drive": "MANUAL", 
  "voice": "AUTO"
}
```
*   **drive:** `AUTO` (Robot moves on detect) | `MANUAL` (Robot waits for joystick).
*   **voice:** `AUTO` (Robot speaks on detect) | `MANUAL` (Robot silent on detect, waits for buttons).
*   **Default:** Both modes start in `AUTO` on power-up.

## 4. Status Check
**Endpoint:** `GET /status`
**Response:**
```json
{
  "driveMode": "MANUAL",
  "voiceMode": "AUTO",
  "distance": -1
}
```

## 5. Joystick Control
**Endpoint:** `POST /move`
**Body:** `{"x": 100, "y": -100}` (Range: -100 to 100)
**Note:** Joystick commands work even in Auto Mode (Override).
