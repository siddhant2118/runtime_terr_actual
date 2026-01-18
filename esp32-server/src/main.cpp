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

// PIN MAP (Restored to Working Config)
// Left Motor
#define IN1 4   // Forward
#define IN2 5   // Backward
// Right Motor
#define IN3 6   // Forward
#define IN4 7   // Backward

// =============================================================
// GLOBAL STATE
// =============================================================

WebServer server(80);
String lastEvent = ""; // For audio polling

// NEW: Split Modes
String driveMode = "AUTO";   // Default: AUTO (Starts automatically)
String voiceMode = "AUTO";   // Default: AUTO

// =============================================================
// MOTOR FUNCTIONS (Full Speed / Bang-Bang)
// =============================================================

// Helper to update event state for Joystick feedback
void setEvent(String evt) {
    lastEvent = evt;
}

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
    // setEvent("STOP"); // Optional: Audio for stop?
}

void moveForward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("MOTORS: FORWARD");
    setEvent("MOVING_FORWARD");
}

void moveBackward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("MOTORS: BACKWARD");
    setEvent("MOVING_BACKWARD");
}

void turnLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("MOTORS: LEFT");
    setEvent("TURNING_LEFT");
}

void turnRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("MOTORS: RIGHT");
    setEvent("TURNING_RIGHT");
}

// Joystick Support (Mapped to Bang-Bang)
String lastAction = "";

void moveMotorsLegacy(int x, int y) {
    String currentAction = "";

    if (abs(x) < 20 && abs(y) < 20) { 
        if(lastAction != "STOP") {
            stopMotors(); 
            lastAction = "STOP";
        }
        return; 
    }
    
    // Simple 4-way direction for joystick
    if (y > 30) { 
        if(lastAction != "FWD") { moveForward(); lastAction = "FWD"; }
    }
    else if (y < -30) { 
        if(lastAction != "BWD") { moveBackward(); lastAction = "BWD"; }
    }
    else if (x < -30) { 
        if(lastAction != "LEFT") { turnLeft(); lastAction = "LEFT"; }
    }
    else if (x > 30) { 
        if(lastAction != "RIGHT") { turnRight(); lastAction = "RIGHT"; }
    }
}

// =============================================================
// EVENT LOGIC (Maneuvers)
// =============================================================

unsigned long lastAudioTime = 0;
const int AUDIO_COOLDOWN = 5000; // 5 Seconds

// Flag: isManualTrigger = true skips cooldown and always plays/moves
void performManeuver(String type, boolean isManualTrigger) {
    Serial.println("MANEUVER: " + type + " (Manual: " + String(isManualTrigger) + ")");

    boolean shouldPlayAudio = false;
    boolean shouldMove = false;

    // 1. Analyze Event Type for AUDIO
    if (type == "saw_human") {
        if (voiceMode == "AUTO" && !isManualTrigger) {
            if (millis() - lastAudioTime > AUDIO_COOLDOWN) {
                shouldPlayAudio = true;
                lastAudioTime = millis();
            } else { Serial.println("AUDIO BLOCKED (Cooldown)"); }
        } else if (isManualTrigger) { shouldPlayAudio = true; }
    } 
    else {
        // Collision, Stuck, Random -> Always play audio
        shouldPlayAudio = true; 
    }

    // 2. Analyze Event Type for MOVEMENT
    // "Manual Drive" means ONLY Joystick (or explicit manual buttons) can move motors.
    // Auto-detection events (saw_human, collision, etc) are BLOCKED in Manual Mode.
    
    if (driveMode == "AUTO" || isManualTrigger) {
        if (type == "saw_human") shouldMove = true;
        else if (type == "collision") shouldMove = true;
        else if (type == "stuck") shouldMove = true;
        // "stop" works regardless, but handled below
        else if (type == "random") shouldMove = true;
    }

    if (type == "stop") {
        stopMotors(); // Safety: Stop always works
        shouldMove = false; 
    }

    // 3. Execute Audio
    if (shouldPlayAudio) {
        lastEvent = type; 
        if(type == "saw_human") lastEvent = "SAW_HUMAN";
        if(type == "collision") lastEvent = "COLLISION";
        if(type == "stuck") lastEvent = "STUCK";
        if(type == "random") lastEvent = "RANDOM";
    }

    // 4. Execute Movement
    if (shouldMove) {
        if (type == "saw_human") {
            // Stop → Turn right → Move forward
            stopMotors(); delay(500); turnRight(); delay(800); moveForward();
        }
        else if (type == "collision") {
            // Stop → Backup 1 sec → Turn left → Move forward
            stopMotors(); delay(200); moveBackward(); delay(1000); turnLeft(); delay(800); moveForward();
        }
        else if (type == "stuck") {
            // Backup 2 sec → Turn 180 (Spin) → Move forward
            stopMotors(); moveBackward(); delay(2000); turnRight(); delay(1500); moveForward();
        }
        else if (type == "random") {
            int action = random(0, 4);
            if(action == 0) { moveForward(); }
            if(action == 1) { moveBackward(); }
            if(action == 2) { turnLeft(); }
            if(action == 3) { turnRight(); }
            delay(1000); stopMotors();
        }
    }
}

// =============================================================
// HTTP HANDLERS
// =============================================================

void sendCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
}

// Debug wrapper
void logRequest(String handlerName) {
    Serial.print("REQ: ");
    Serial.print(server.method() == HTTP_GET ? "GET " : "POST ");
    Serial.print(server.uri());
    Serial.print(" -> ");
    Serial.println(handlerName);
}

// 1. Existing Handlers (Remote/Voice)
void handleStatic(String path, String type) {
    logRequest("STATIC: " + path);
    if(LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, type);
        file.close();
    } else {
        Serial.println("FILE MISSING: " + path);
        server.send(404, "text/plain", "File Missing");
    }
}
void handleRoot() { handleStatic("/index.html", "text/html"); }
void handleVoice() { handleStatic("/voice.html", "text/html"); }
void handleAudioMap() { handleStatic("/audio_map.json", "application/json"); }

// 2. DETECT HANDLER (Auto/App Trigger)
void handleDetect() {
    logRequest("DETECT");
    sendCORS();
    if (server.hasArg("type")) {
        String type = server.arg("type");
        // If Voice is MANUAL, we ignore "saw_human" coming from detection logic
        if (type == "saw_human" && voiceMode == "MANUAL") {
            server.send(200, "text/plain", "IGNORED (Voice Manual)");
            return;
        }
        
        server.send(200, "text/plain", "OK: " + type);
        performManeuver(type, false); // isManualTrigger = false
    } else {
        server.send(400, "text/plain", "Missing type param");
    }
}

// 3. NEW: EVENT POST HANDLER (Manual Trigger)
void handleEventPost() {
    logRequest("EVENT_POST");
    sendCORS();
    String type = "";
    
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        if (doc.containsKey("event")) type = doc["event"].as<String>();
    } else if (server.hasArg("event")) {
        type = server.arg("event");
    }

    if (type != "") {
        // Manual triggers ALWAYS fire
        performManeuver(type, true); // isManualTrigger = true
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

// 4. NEW: MODE SETTER
void handleSetMode() {
    logRequest("SET_MODE");
    sendCORS();
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        if (doc.containsKey("drive")) driveMode = doc["drive"].as<String>();
        if (doc.containsKey("voice")) voiceMode = doc["voice"].as<String>();
        
        Serial.println("MODES UPDATED -> Drive: " + driveMode + ", Voice: " + voiceMode);
    }
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}


void handleStatus() {
    // logRequest("STATUS"); // Reduce spam
    sendCORS();
    JsonDocument doc;
    doc["driveMode"] = driveMode;
    doc["voiceMode"] = voiceMode;
    doc["distance"] = -1; // No sensor in this config
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

// 5. Status/Event polling for Voice
void handleEventGet() {
    // logRequest("EVENT_POLL"); // Commented out to reduce spam
    sendCORS();
    JsonDocument doc;
    doc["event"] = lastEvent;
    String out;
    serializeJson(doc, out);
    lastEvent = ""; 
    server.send(200, "application/json", out);
}

// 6. Legacy Joystick Handler
void handleMove() {
    logRequest("MOVE");
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
    logRequest("AUDIO");
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
    delay(1000);
    Serial.println("\n\n=== BOOT STARTS ===");
    
    setupMotors();
    Serial.println("MOTORS: INITIALIZED (STOPPED)");

    // LittleFS
    if(!LittleFS.begin(true)){
        Serial.println("!!! LittleFS Mount Failed !!!");
        return;
    }
    Serial.println("LittleFS Mounted: OK");

    // WiFi
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Point IP: ");
    Serial.println(IP); 

    // Routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/voice", HTTP_GET, handleVoice);
    server.on("/audio_map.json", HTTP_GET, handleAudioMap);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/detect", HTTP_GET, handleDetect);
    server.on("/event", HTTP_GET, handleEventGet);
    server.on("/event", HTTP_POST, handleEventPost); // FIX: Allow POST
    server.on("/move", HTTP_POST, handleMove);
    server.on("/mode", HTTP_POST, handleSetMode);    // FIX: Set Mode
    
    server.on("/move", HTTP_OPTIONS, [](){ sendCORS(); server.send(204); });
    server.on("/detect", HTTP_OPTIONS, [](){ sendCORS(); server.send(204); });

    // CATCH ALL
    server.onNotFound([](){
        Serial.print("404 MISS: ");
        Serial.print(server.method() == HTTP_GET ? "GET " : "POST ");
        Serial.println(server.uri());
        
        if(server.uri().endsWith(".mp3")) {
            handleAudio();
        } else {
            server.send(404, "text/plain", "404 Not Found (Debug Mode)");
        }
    });

    server.begin();
    Serial.println("HTTP Server Started");
    Serial.println("=== READY ===");
}

void loop() {
    server.handleClient();
    delay(1);
    // Note: No autonomous loop here anymore. 
    // Movement is purely event-driven by /detect or joystick.
}
