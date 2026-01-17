#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define SERIAL_BAUD_RATE 9600

// --- Serial Configuration ---
// If using Arduino UNO (ATmega328P), we use SoftwareSerial for the ESP32
// so we can keep the main "Serial" free for USB debugging.
#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
    #include <SoftwareSerial.h>
    #define USE_SOFTWARE_SERIAL
    #define PIN_RX_FROM_ESP  2  // Connect to ESP32 TX
    #define PIN_TX_TO_ESP    3  // Connect to ESP32 RX
#elif defined(ESP32)
    // ESP32 WROOM / DevKit Pinout
    #define ESP_SERIAL Serial // Use USB Serial for output (or Serial2 if communicating with another device)
    
    // Motor Pins (Use GPIOs safe for output)
    #define PIN_MOTOR_LEFT_FWD   26
    #define PIN_MOTOR_LEFT_BCK   25
    #define PIN_MOTOR_RIGHT_FWD  33
    #define PIN_MOTOR_RIGHT_BCK  32
    
    // Sensor Pins
    #define PIN_ULTRASONIC_TRIG  13
    #define PIN_ULTRASONIC_ECHO  12
    #define PIN_BUMP_LEFT        14
    #define PIN_BUMP_RIGHT       27
    #define PIN_RESET_BUTTON     0  // BOOT button on board
#else
    // For Mega/Leonardo/Galileo (Boards with HW Serial1)
    #define ESP_SERIAL Serial1 
#endif

#ifndef ESP32
// --- Motor Pins (L298N Standard for Arduino/Galileo) ---
#define PIN_MOTOR_LEFT_FWD   5
#define PIN_MOTOR_LEFT_BCK   6
#define PIN_MOTOR_RIGHT_FWD  9
#define PIN_MOTOR_RIGHT_BCK  10
#endif

// --- Sensor Pins (Shared/Fallbacks if not defined above) ---
#ifndef PIN_ULTRASONIC_TRIG
#define PIN_ULTRASONIC_TRIG  2
#define PIN_ULTRASONIC_ECHO  3
#define PIN_BUMP_LEFT        4
#define PIN_BUMP_RIGHT       7
#define PIN_RESET_BUTTON     8
#endif

// --- Thresholds ---
#define COLLISION_DIST_CM    15
#define IDLE_TIMEOUT_MS      10000

#endif // CONFIG_H
