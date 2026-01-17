# Arduino Uno Setup Guide

Since you switched to an Arduino Uno, the steps are much easier than the Galileo!

## 1. Wiring (IMPORTANT)
The wiring is slightly different for the Arduino Uno to allow USB debugging.

*   **Pin 2** -> Connect to **ESP32 TX**
*   **Pin 3** -> Connect to **ESP32 RX**
*   **GND**   -> Connect to **ESP32 GND** (Common Ground is critical!)
*   **5V**    -> Power the motors (Do not run motors directly from the Arduino 5V pin if possible, use an external battery for motors).

## 2. Open the Project
1.  Open **Arduino IDE**.
2.  Go to **File > Open**.
3.  Navigate to your folder: `layer-c-galileo/galileo_nav/`
4.  Select `galileo_nav.ino`.

## 3. Select the Board
1.  Go to **Tools > Board**.
2.  Select **Arduino AVR Boards** -> **Arduino Uno**.

## 4. Select the Port
1.  Plug in your Arduino Uno via USB.
2.  Go to **Tools > Port**.
3.  You should see something like `/dev/cu.usbmodem...` or `/dev/cu.usbserial...` (it usually says "Arduino Uno" next to it).

## 5. Upload
1.  Click the **Arrow Icon (->)** in the top left.
2.  Wait for it to say **"Done uploading"**.

## 6. Verify
1.  Open **Tools > Serial Monitor**.
2.  Set baud rate to **115200** (for debug output).
3.  You should see "BOOT" or debug messages.
