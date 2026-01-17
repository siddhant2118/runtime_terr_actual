# Intel Galileo Setup & Upload Guide

Since the Intel Galileo is a legacy board (discontinued), setting it up requires specific steps. Follow this guide exactly.

## 1. Hardware Setup (CRITICAL)
*   **Power Supply**: You **MUST** plug in the 5V power adapter involved with the Galileo.
    *   *Warning*: The USB cable alone does NOT provide enough power. You will damage the board or corrupt the upload if you try USB-only.
*   **USB Connection**: Connect the USB cable from the port labeled **USB CLIENT** (near the ethernet jack) to your Mac.

## 2. Arduino IDE Setup (The "Rescue" Fix)
The standard installation will not work because Intel turned off their servers.

1.  Open Arduino IDE.
2.  Go to **Arduino IDE** (menu) -> **Settings** (or Preferences).
3.  Find the box **Additional Boards Manager URLs**.
4.  Paste this **Exact Link** (copy-paste carefully):
    ```text
    https://raw.githubusercontent.com/maxgerhardt/arduino-Intel-i586-revive/refs/heads/main/package_inteli586_index.json
    ```
5.  Click **OK**.
6.  Go to **Tools** -> **Board** -> **Boards Manager...**
7.  In the search bar, type: `Intel i586`
8.  You should see "Intel i586 Boards (Revived)". Click **INSTALL**.
    *   *Note: This might take a minute.*

## 3. Select the Board
1.  Go to **Tools** -> **Board** -> **Intel i586 Boards**.
2.  Select **Intel Galileo Gen2**.

## 4. Select the Port
1.  Go to **Tools** -> **Port**.
2.  Look for a port named `/dev/cu.usbmodem...` (it usually has a number like 14101 or 14201).
    *   *If you do not see this port: Check your USB cable and ensure the Power Adapter is plugged in.*

## 5. Compile & Upload
1.  Open the file `layer-c-galileo/galileo_nav/galileo_nav.ino`.
2.  Click the **Checkmark (Verify)** button first. It should say "Done compiling."
3.  Click the **Arrow (Upload)** button.
4.  Watch the console at the bottom.
    *   It should say "Transfer complete" when done.
    *   The board might reboot.

## Troubleshooting
*   **"No such file or directory"**: You didn't install the board package from Step 2 properly.
*   **"Upload failed"**: Unplug USB and Power, wait 10s, plug Power first, then USB.
