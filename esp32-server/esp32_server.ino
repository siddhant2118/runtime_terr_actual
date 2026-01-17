// ESP32 Sheldon Rover - Wi-Fi Server + Robot Control
// Hosts a Wi-Fi AP with HTTP endpoints for remote control

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// =============================================================
// CONFIGURATION
// =============================================================

// Wi-Fi Access Point settings
const char* AP_SSID = "SheldonRover";
const char* AP_PASSWORD = "bazinga123";  // At least 8 characters

// Motor Pins (L298N Driver)
#define PIN_MOTOR_LEFT_FWD   26
#define PIN_MOTOR_LEFT_BCK   25
#define PIN_MOTOR_RIGHT_FWD  33
#define PIN_MOTOR_RIGHT_BCK  32

// Sensor Pins
#define PIN_ULTRASONIC_TRIG  13
#define PIN_ULTRASONIC_ECHO  12
#define PIN_BUMP_LEFT        14
#define PIN_BUMP_RIGHT       27

// Thresholds
#define COLLISION_DIST_CM    15
#define IDLE_TIMEOUT_MS      10000

// =============================================================
// GLOBAL STATE
// =============================================================

WebServer server(80);

String lastEvent = "";
String currentMode = "MID";
bool motorsStopped = true;
unsigned long lastActivityTime = 0;

// =============================================================
// MOTOR CONTROL
// =============================================================

void setupMotors() {
    pinMode(PIN_MOTOR_LEFT_FWD, OUTPUT);
    pinMode(PIN_MOTOR_LEFT_BCK, OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_FWD, OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_BCK, OUTPUT);
    stopMotors();
}

void stopMotors() {
    analogWrite(PIN_MOTOR_LEFT_FWD, 0);
    digitalWrite(PIN_MOTOR_LEFT_BCK, LOW);
    analogWrite(PIN_MOTOR_RIGHT_FWD, 0);
    digitalWrite(PIN_MOTOR_RIGHT_BCK, LOW);
    motorsStopped = true;
}

void moveMotors(int x, int y) {
    // x: -100 to 100 (left/right)
    // y: -100 to 100 (backward/forward)
    
    int leftSpeed = constrain(y + x, -100, 100);
    int rightSpeed = constrain(y - x, -100, 100);
    
    // Left motor
    if (leftSpeed >= 0) {
        analogWrite(PIN_MOTOR_LEFT_FWD, map(leftSpeed, 0, 100, 0, 255));
        digitalWrite(PIN_MOTOR_LEFT_BCK, LOW);
    } else {
        analogWrite(PIN_MOTOR_LEFT_FWD, 0);
        digitalWrite(PIN_MOTOR_LEFT_BCK, HIGH);
        // Note: For proper reverse PWM, use analogWrite on BCK pin too
    }
    
    // Right motor
    if (rightSpeed >= 0) {
        analogWrite(PIN_MOTOR_RIGHT_FWD, map(rightSpeed, 0, 100, 0, 255));
        digitalWrite(PIN_MOTOR_RIGHT_BCK, LOW);
    } else {
        analogWrite(PIN_MOTOR_RIGHT_FWD, 0);
        digitalWrite(PIN_MOTOR_RIGHT_BCK, HIGH);
    }
    
    motorsStopped = (x == 0 && y == 0);
    lastActivityTime = millis();
}

// =============================================================
// SENSOR READING
// =============================================================

void setupSensors() {
    pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ECHO, INPUT);
    pinMode(PIN_BUMP_LEFT, INPUT_PULLUP);
    pinMode(PIN_BUMP_RIGHT, INPUT_PULLUP);
}

float readUltrasonic() {
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    
    long duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, 30000);
    float distance = duration * 0.034 / 2;
    
    return (distance == 0) ? 999 : distance;
}

bool checkCollision() {
    // Check bump switches (active LOW with pullup)
    if (digitalRead(PIN_BUMP_LEFT) == LOW || digitalRead(PIN_BUMP_RIGHT) == LOW) {
        return true;
    }
    
    // Check ultrasonic
    float dist = readUltrasonic();
    if (dist < COLLISION_DIST_CM && dist > 0) {
        return true;
    }
    
    return false;
}

// =============================================================
// HTTP HANDLERS
// =============================================================

void sendCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptions() {
    sendCORS();
    server.send(204);
}

void handleStatus() {
    sendCORS();
    
    StaticJsonDocument<256> doc;
    doc["status"] = "online";
    doc["mode"] = currentMode;
    doc["lastEvent"] = lastEvent;
    doc["motorsStopped"] = motorsStopped;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleGetEvent() {
    sendCORS();
    
    StaticJsonDocument<128> doc;
    doc["event"] = lastEvent;
    
    String response;
    serializeJson(doc, response);
    
    // Clear event after reading (one-shot)
    lastEvent = "";
    
    server.send(200, "application/json", response);
}

void handlePostEvent() {
    sendCORS();
    
    if (server.hasArg("plain")) {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc.containsKey("event")) {
            lastEvent = doc["event"].as<String>();
            Serial.println("EVENT: " + lastEvent);
            
            server.send(200, "application/json", "{\"ok\":true}");
            return;
        }
    }
    
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handlePostMode() {
    sendCORS();
    
    if (server.hasArg("plain")) {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc.containsKey("mode")) {
            currentMode = doc["mode"].as<String>();
            Serial.println("MODE: " + currentMode);
            
            server.send(200, "application/json", "{\"ok\":true}");
            return;
        }
    }
    
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handlePostStop() {
    sendCORS();
    
    stopMotors();
    lastEvent = "";
    Serial.println("EMERGENCY STOP");
    
    server.send(200, "application/json", "{\"ok\":true,\"stopped\":true}");
}

void handlePostSay() {
    sendCORS();
    
    if (server.hasArg("plain")) {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc.containsKey("id")) {
            String audioId = doc["id"].as<String>();
            Serial.println("SAY: " + audioId);
            
            // Store as event so iPhone 1 (mouth) can pick it up
            lastEvent = "SAY:" + audioId;
            
            server.send(200, "application/json", "{\"ok\":true}");
            return;
        }
    }
    
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handlePostMove() {
    sendCORS();
    
    if (server.hasArg("plain")) {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc.containsKey("x") && doc.containsKey("y")) {
            int x = doc["x"].as<int>();
            int y = doc["y"].as<int>();
            
            moveMotors(x, y);
            
            server.send(200, "application/json", "{\"ok\":true}");
            return;
        }
    }
    
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
}

void handleNotFound() {
    sendCORS();
    server.send(404, "application/json", "{\"error\":\"Not found\"}");
}

// =============================================================
// SETUP & LOOP
// =============================================================

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Sheldon Rover ESP32 ===");
    
    // Setup hardware
    setupMotors();
    setupSensors();
    
    // Start Wi-Fi AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);
    
    // Setup HTTP routes
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/status", HTTP_OPTIONS, handleOptions);
    
    server.on("/event", HTTP_GET, handleGetEvent);
    server.on("/event", HTTP_POST, handlePostEvent);
    server.on("/event", HTTP_OPTIONS, handleOptions);
    
    server.on("/mode", HTTP_POST, handlePostMode);
    server.on("/mode", HTTP_OPTIONS, handleOptions);
    
    server.on("/stop", HTTP_POST, handlePostStop);
    server.on("/stop", HTTP_OPTIONS, handleOptions);
    
    server.on("/say", HTTP_POST, handlePostSay);
    server.on("/say", HTTP_OPTIONS, handleOptions);
    
    server.on("/move", HTTP_POST, handlePostMove);
    server.on("/move", HTTP_OPTIONS, handleOptions);
    
    server.onNotFound(handleNotFound);
    
    server.begin();
    Serial.println("HTTP server started!");
    Serial.println("Connect to WiFi: " + String(AP_SSID));
    Serial.println("Then open: http://" + IP.toString());
    
    // Send BOOT event
    lastEvent = "BOOT";
    lastActivityTime = millis();
}

void loop() {
    // Handle HTTP requests
    server.handleClient();
    
    // Check for autonomous events (collision, stuck, idle)
    static unsigned long lastSensorCheck = 0;
    if (millis() - lastSensorCheck > 100) {  // Check every 100ms
        lastSensorCheck = millis();
        
        // Only check sensors if motors are running
        if (!motorsStopped) {
            if (checkCollision()) {
                stopMotors();
                lastEvent = "COLLISION";
                Serial.println("AUTO: COLLISION");
            }
        }
        
        // Check for idle timeout
        if (millis() - lastActivityTime > IDLE_TIMEOUT_MS && lastEvent.isEmpty()) {
            lastEvent = "IDLE_TOO_LONG";
            Serial.println("AUTO: IDLE_TOO_LONG");
            lastActivityTime = millis();  // Reset to avoid spam
        }
    }
    
    delay(1);
}
