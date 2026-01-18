# 🔌 Sheldon Rover: Wiring Guide

## 1. Quick System Overview
Here is how the wires connect.

```mermaid
graph TD
    BATTERY[Battery 12V] -->|Red| SW[Switch]
    BATTERY -->|Black| GND_COMMON[Common Ground Point]
    
    SW -->|12V| L298N[L298N Driver]
    GND_COMMON -->|GND| L298N
    GND_COMMON -->|GND| ESP32[ESP32 Board]
    
    L298N -->|5V Out| ESP32
    
    ESP32 -->|GPIO 5| L298N_IN1[IN1: Left Fwd]
    ESP32 -->|GPIO 6| L298N_IN2[IN2: Left Back]
    ESP32 -->|GPIO 7| L298N_IN3[IN3: Right Fwd]
    ESP32 -->|GPIO 15| L298N_IN4[IN4: Right Back]
    
    L298N -->|OUT1/2| MOTORS_L[Left Motors]
    L298N -->|OUT3/4| MOTORS_R[Right Motors]
```

---

## 2. Breadboard Map (Top View)
Use this map to see where wires go on the breadboard.

```ascii
      [ BATTERY PACK ]
         |      |
       (Red)  (Black)
         |      |
      [SWITCH]  |
         |      |
         |      v
         v    (Common GND Row on Breadboard)
       (12V)    ============================
         |          |           |
         |          |           |
    +----v----------v-----------v----+
    | [12V]       [GND]        [GND] |   <-- ESP32 GND Pin
    |                                |
    |   L298N MOTOR DRIVER           |
    |                                |
    | [IN1] [IN2] [IN3] [IN4]        |
    +---^-----^-----^-----^----------+
        |     |     |     |
        |     |     |     |  (Signal Wires)
        |     |     |     |
    +---5-----6-----7-----15---------+
    | GPIO Pins on ESP32 Side        |
    |                                |
    |          [ ESP32 ]             |
    +--------------------------------+
```

---

## 3. Wiring Checklist (Step-by-Step)
Connect these wires one by one.

| Check | From Component | Pin Name | To Component | Pin Name | Wire Color |
| :--- | :--- | :--- | :--- | :--- | :--- |
| [ ] | **L298N** | `IN1` | **ESP32** | `GPIO 5` | Any |
| [ ] | **L298N** | `IN2` | **ESP32** | `GPIO 6` | Any |
| [ ] | **L298N** | `IN3` | **ESP32** | `GPIO 7` | Any |
| [ ] | **L298N** | `IN4` | **ESP32** | `GPIO 15` | Any |
| [ ] | ** Battery ** | `Red (+)` | **Switch** | `Input` | Red |
| [ ] | **Switch** | `Output` | **L298N** | `12V` | Red |
| [ ] | **Battery** | `Black (-)` | **L298N** | `GND` | Black |
| [ ] | **L298N** | `GND` | **ESP32** | `GND` | Black |

---

## 4. Troubleshooting
*   **Motors Twitching?** -> You forgot the **Common Ground** (Battery (-) to ESP32 GND). Connect it immediately!
*   **Nothing lights up?** -> Check if the **Switch** is ON.
*   **One side goes backwards?** -> Swap the two wires going to that *motor* (e.g., swap wires at OUT1).
