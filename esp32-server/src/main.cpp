// ESP32 Robot - Fixed Hardware Config
// Pins: 4, 5, 6, 7 | No PWM | /detect endpoint

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// =============================================================
// CONFIGURATION
// =============================================================

const char* AP_SSID = "SheldonRover";
const char* AP_PASSWORD = "bazinga123";

// NEW PIN MAP (Requested)
// Left Motor
#define IN1 4  // Forward
#define IN2 5  // Backward
// Right Motor
#define IN3 6  // Forward
#define IN4 7  // Backward

// =============================================================
// GLOBAL STATE
// =============================================================

WebServer server(80);
String lastEvent = ""; // For audio polling

// =============================================================
// MOTOR FUNCTIONS (Full Speed / Bang-Bang)
// =============================================================

void setupMotors() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    
    // Initial State: Stopped
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void stopMotors() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    Serial.println("MOTORS: STOP");
}

void moveForward() {
    // Left Fwd, Right Fwd
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("MOTORS: FORWARD");
}

void moveBackward() {
    // Left Back, Right Back
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("MOTORS: BACKWARD");
}

void turnLeft() {
    // Left Back, Right Fwd (Spin)
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("MOTORS: LEFT");
}

void turnRight() {
    // Left Fwd, Right Back (Spin)
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("MOTORS: RIGHT");
}

// Joystick Support (Mapped to Bang-Bang)
void moveMotorsLegacy(int x, int y) {
    if (abs(x) < 20 && abs(y) < 20) { stopMotors(); return; }
    
    // Simple 4-way direction for joystick
    if (y > 30) { moveForward(); }
    else if (y < -30) { moveBackward(); }
    else if (x < -30) { turnLeft(); }
    else if (x > 30) { turnRight(); }
}

// =============================================================
// EVENT LOGIC (Maneuvers)
// =============================================================

void performManeuver(String type) {
    Serial.println("MANEUVER: " + type);
    
    // Record event for Voice interface
    lastEvent = type; 
    // MAPPING: "saw_human" -> "SAW_HUMAN" (Case correction for audio map)
    if(type == "saw_human") lastEvent = "SAW_HUMAN";
    if(type == "collision") lastEvent = "COLLISION";
    if(type == "stuck") lastEvent = "STUCK";
    if(type == "random") lastEvent = "RANDOM";

    if (type == "saw_human") {
        // Stop → Turn right → Move forward
        stopMotors();
        delay(500);
        turnRight();
        delay(800);
        moveForward();
        // Stays forward until next event? Or for a duration?
        // User said "Move forward", implies continuous.
        // We leave it running forward.
    }
    else if (type == "collision") {
        // Stop → Backup 1 sec → Turn left → Move forward
        stopMotors();
        delay(200);
        moveBackward();
        delay(1000);
        turnLeft();
        delay(800);
        moveForward();
    }
    else if (type == "stuck") {
        // Backup 2 sec → Turn 180 (Spin) → Move forward
        stopMotors();
        moveBackward();
        delay(2000);
        turnRight(); // 180ish
        delay(1500);
        moveForward();
    }
    else if (type == "random") {
        // Random movement
        int action = random(0, 4);
        if(action == 0) moveForward();
        if(action == 1) moveBackward();
        if(action == 2) turnLeft();
        if(action == 3) turnRight();
        delay(1000);
        stopMotors();
    }
    else if (type == "stop") {
        stopMotors();
    }
}

// =============================================================
// HTTP HANDLERS
// =============================================================

void sendCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
}

// 1. Existing Handlers (Remote/Voice)
void handleStatic(String path, String type) {
    if(LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, type);
        file.close();
    } else {
        server.send(404, "text/plain", "Not Found");
    }
}
void handleRoot() { handleStatic("/index.html", "text/html"); }
void handleVoice() { handleStatic("/voice.html", "text/html"); }
void handleAudioMap() { handleStatic("/audio_map.json", "application/json"); }

// 2. NEW DETECT HANDLER (Requested)
// GET /detect?type=saw_human
void handleDetect() {
    sendCORS();
    if (server.hasArg("type")) {
        String type = server.arg("type");
        server.send(200, "text/plain", "OK: " + type);
        performManeuver(type);
    } else {
        server.send(400, "text/plain", "Missing type param");
    }
}

// 3. Status/Event polling for Voice
void handleEventGet() {
    sendCORS();
    JsonDocument doc;
    doc["event"] = lastEvent;
    String out;
    serializeJson(doc, out);
    lastEvent = ""; 
    server.send(200, "application/json", out);
}

// 4. Legacy Joystick Handler (Mapped to new move functions)
void handleMove() {
    sendCORS();
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        int x = doc["x"];
        int y = doc["y"];
        moveMotorsLegacy(x, y);
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

// Audio Wildcard
void handleAudio() {
    sendCORS();
    String path = server.uri();
    if(LittleFS.exists(path)) {
        File f = LittleFS.open(path, "r");
        server.streamFile(f, "audio/mpeg");
        f.close();
    } else {
        server.send(404, "text/plain", "Audio Not Found");
    }
}

// =============================================================
// SETUP
// =============================================================

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Sheldon Rover Configured ===");
    
    setupMotors();

    // LittleFS
    if(!LittleFS.begin(true)){
        Serial.println("LittleFS Mount Failed");
        return;
    }

    // WiFi
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Point IP: ");
    Serial.println(IP); // Print IP for iPhone app!

    // Routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/voice", HTTP_GET, handleVoice);
    server.on("/audio_map.json", HTTP_GET, handleAudioMap);
    
    // NEW: Detection Endpoint
    server.on("/detect", HTTP_GET, handleDetect);
    
    // Legacy support
    server.on("/event", HTTP_GET, handleEventGet);
    server.on("/move", HTTP_POST, handleMove);
    server.on("/move", HTTP_OPTIONS, [](){ sendCORS(); server.send(204); });

    // Audio Fallback
    server.onNotFound([](){
        if(server.uri().endsWith(".mp3")) handleAudio();
        else server.send(404);
    });

    server.begin();
    Serial.println("HTTP Server Started");
}

void loop() {
    server.handleClient();
    delay(1);
    // Note: No autonomous loop here anymore. 
    // Movement is purely event-driven by /detect or joystick.
}
