// ESP32 Sheldon Rover - Autonomous + Joystick
// Hosts a Wi-Fi AP with HTTP endpoints + LittleFS

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// =============================================================
// CONFIGURATION
// =============================================================

const char* AP_SSID = "SheldonRover";
const char* AP_PASSWORD = "bazinga123";

// Motor Pins (L298N)
#define PIN_MOTOR_LEFT_FWD   5
#define PIN_MOTOR_LEFT_BCK   6
#define PIN_MOTOR_RIGHT_FWD  7
#define PIN_MOTOR_RIGHT_BCK  15

// Sensors
#define PIN_ULTRASONIC_TRIG  16
#define PIN_ULTRASONIC_ECHO  17
#define PIN_BUMP_LEFT        18
#define PIN_BUMP_RIGHT       8

// Constants
#define OBSTACLE_DIST_CM     25
#define REVERSE_TIME_MS      800
#define TURN_TIME_MS         600
#define RANDOM_EVENT_MS      20000

// =============================================================
// GLOBAL STATE
// =============================================================

WebServer server(80);

String currentMode = "MANUAL"; // MANUAL or AUTO
String lastEvent = "";         // Event for iPhone 2 (Face)

unsigned long lastAutoMove = 0;
unsigned long lastRandomEvent = 0;
int autoState = 0; // 0=Forward, 1=Back, 2=Turn

// =============================================================
// UTIL
// =============================================================

void moveMotors(int x, int y) {
    // x: -100 to 100 (left/right)
    // y: -100 to 100 (back/fwd)
    
    // Limits
    if(x < -100) x = -100; if(x > 100) x = 100;
    if(y < -100) y = -100; if(y > 100) y = 100;

    int leftSpeed = y + x;
    int rightSpeed = y - x;

    leftSpeed = constrain(leftSpeed, -100, 100);
    rightSpeed = constrain(rightSpeed, -100, 100);

    // Left Motor
    if (leftSpeed > 0) {
        analogWrite(PIN_MOTOR_LEFT_FWD, map(leftSpeed, 0, 100, 0, 255));
        digitalWrite(PIN_MOTOR_LEFT_BCK, LOW);
    } else {
        analogWrite(PIN_MOTOR_LEFT_FWD, 0);
        analogWrite(PIN_MOTOR_LEFT_BCK, map(abs(leftSpeed), 0, 100, 0, 255)); 
        // Note: For PWM on BCK pin, use analogWrite if supported or just digitalWrite HIGH for simple H-Bridge
        // L298N supports PWM on controls.
    }

    // Right Motor
    if (rightSpeed > 0) {
        analogWrite(PIN_MOTOR_RIGHT_FWD, map(rightSpeed, 0, 100, 0, 255));
        digitalWrite(PIN_MOTOR_RIGHT_BCK, LOW);
    } else {
        analogWrite(PIN_MOTOR_RIGHT_FWD, 0);
        analogWrite(PIN_MOTOR_RIGHT_BCK, map(abs(rightSpeed), 0, 100, 0, 255));
    }
}

void stopMotors() {
    moveMotors(0, 0);
}

float getDistance() {
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    long duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, 30000); // 30ms timeout
    if (duration == 0) return 100.0; // No echo = far
    return duration * 0.034 / 2;
}

bool checkBump() {
    return (digitalRead(PIN_BUMP_LEFT) == LOW || digitalRead(PIN_BUMP_RIGHT) == LOW);
}

// =============================================================
// HTTP HANDLERS
// =============================================================

void sendCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleStatic(String path, String type) {
    if(LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, type);
        file.close();
    } else {
        server.send(404, "text/plain", "File Missing: " + path);
    }
}

void handleRoot() { handleStatic("/index.html", "text/html"); }
void handleVoice() { handleStatic("/voice.html", "text/html"); }
void handleAudioMap() { handleStatic("/audio_map.json", "application/json"); }

void handleMode() {
    sendCORS();
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        if(doc["mode"].is<String>()) {
            currentMode = doc["mode"].as<String>();
            Serial.println("MODE SET: " + currentMode);
            // Reset auto state
            if(currentMode == "AUTO") {
                autoState = 0; 
                lastAutoMove = millis();
            } else {
                stopMotors();
            }
        }
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleMove() {
    sendCORS();
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        int x = doc["x"];
        int y = doc["y"];
        
        // Joystick Override
        if (currentMode == "AUTO" && (abs(x) > 10 || abs(y) > 10)) {
            currentMode = "MANUAL";
            Serial.println("Joystick Override -> MANUAL");
        }
        
        if (currentMode == "MANUAL") {
            moveMotors(x, y);
        }
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleEventPost() {
    sendCORS();
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        String e = doc["event"];
        if (e.length() > 0) {
            lastEvent = e;
            Serial.println("EVENT: " + lastEvent);
        }
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleEventGet() {
    sendCORS();
    JsonDocument doc;
    doc["event"] = lastEvent;
    String out;
    serializeJson(doc, out);
    lastEvent = ""; // Consume
    server.send(200, "application/json", out);
}

void handleStatus() {
    sendCORS();
    JsonDocument doc;
    doc["mode"] = currentMode;
    doc["distance"] = getDistance(); // debug
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleAudio() {
    sendCORS();
    String path = server.uri();
    if(LittleFS.exists(path)) {
        File f = LittleFS.open(path, "r");
        server.streamFile(f, "audio/mpeg");
        f.close();
    } else {
        server.send(404, "text/plain", "Not Found");
    }
}

// =============================================================
// SETUP
// =============================================================

void setup() {
    Serial.begin(115200);
    
    // Pins
    pinMode(PIN_MOTOR_LEFT_FWD, OUTPUT);
    pinMode(PIN_MOTOR_LEFT_BCK, OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_FWD, OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_BCK, OUTPUT);
    
    pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ECHO, INPUT);
    pinMode(PIN_BUMP_LEFT, INPUT_PULLUP);
    pinMode(PIN_BUMP_RIGHT, INPUT_PULLUP);

    if(!LittleFS.begin(true)){
        Serial.println("LITTLEFS FAIL");
        return;
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.println(WiFi.softAPIP());

    // Routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/voice", HTTP_GET, handleVoice);
    server.on("/audio_map.json", HTTP_GET, handleAudioMap);
    server.on("/move", HTTP_POST, handleMove);
    server.on("/mode", HTTP_POST, handleMode);
    server.on("/event", HTTP_POST, handleEventPost);
    server.on("/event", HTTP_GET, handleEventGet);
    server.on("/status", HTTP_GET, handleStatus);
    
    // Options
    server.on("/move", HTTP_OPTIONS, [](){ sendCORS(); server.send(204); });
    server.on("/mode", HTTP_OPTIONS, [](){ sendCORS(); server.send(204); });
    server.on("/event", HTTP_OPTIONS, [](){ sendCORS(); server.send(204); });
    
    // Audio wildcard
    server.onNotFound([](){
        if(server.uri().endsWith(".mp3")) handleAudio();
        else server.send(404);
    });

    server.begin();
    lastEvent = "BOOT";
}

// =============================================================
// MAIN LOOP - THE BRAIN
// =============================================================

void loop() {
    server.handleClient();

    // Autonomous Logic
    if (currentMode == "AUTO") {
        unsigned long now = millis();
        float dist = getDistance();
        bool bump = checkBump();

        // 1. Safety / Collision
        if (autoState == 0) { // Driving Forward state
            if (bump || (dist < OBSTACLE_DIST_CM && dist > 1.0)) {
                // COLLISION DETECTED
                stopMotors();
                autoState = 1; // Go to Back state
                lastAutoMove = now;
                lastEvent = (bump) ? "COLLISION" : "STUCK"; // Or SAW_HUMAN if distance
                if(dist < 10) lastEvent = "SAW_HUMAN"; // Close proxy
                Serial.println("Obstacle! " + lastEvent);
            } else {
                moveMotors(0, 40); // Drive Forward slow
            }
        } 
        else if (autoState == 1) { // Reversing
            moveMotors(0, -45);
            if (now - lastAutoMove > REVERSE_TIME_MS) {
                autoState = 2; // Turn
                lastAutoMove = now;
            }
        }
        else if (autoState == 2) { // Turning
            moveMotors(60, 0); // Spin right
            if (now - lastAutoMove > TURN_TIME_MS) {
                autoState = 0; // Back to Forward
                lastAutoMove = now;
            }
        }

        // 2. Random Commentary
        if (now - lastRandomEvent > RANDOM_EVENT_MS) {
            if (random(0, 100) < 30) { // 30% chance every 20s
                 lastEvent = "RANDOM";
            }
            lastRandomEvent = now;
        }
    }
    
    delay(5);
}
