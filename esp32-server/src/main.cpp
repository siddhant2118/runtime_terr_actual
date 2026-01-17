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

// Motor Pins (L298N Driver) - ESP32-S3 compatible
#define PIN_MOTOR_LEFT_FWD   5
#define PIN_MOTOR_LEFT_BCK   6
#define PIN_MOTOR_RIGHT_FWD  7
#define PIN_MOTOR_RIGHT_BCK  15

// Sensor Pins - ESP32-S3 compatible
#define PIN_ULTRASONIC_TRIG  16
#define PIN_ULTRASONIC_ECHO  17
#define PIN_BUMP_LEFT        18
#define PIN_BUMP_RIGHT       8

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

// Function prototypes
void stopMotors();
void moveMotors(int x, int y);
void setupMotors();
void setupSensors();
float readUltrasonic();
bool checkCollision();
void handleRoot();
void handleVoice();

// =============================================================
// EMBEDDED WEB PAGE (served directly, no internet needed)
// =============================================================

const char REMOTE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Sheldon Remote</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;-webkit-user-select:none}
body{font-family:-apple-system,sans-serif;background:#1a1a2e;color:#fff;padding:15px;min-height:100vh}
h1{text-align:center;font-size:1.3rem;color:#00d4ff;margin-bottom:15px}
.status{background:rgba(255,255,255,.1);padding:10px;border-radius:10px;margin-bottom:15px;text-align:center}
.dot{width:10px;height:10px;border-radius:50%;background:#ff4444;display:inline-block;margin-right:8px}
.dot.ok{background:#44ff44}
.stop{width:100%;padding:18px;font-size:1.3rem;font-weight:bold;background:linear-gradient(145deg,#ff4444,#cc0000);border:none;border-radius:12px;color:#fff;margin-bottom:15px}
.grid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-bottom:15px}
.btn{padding:15px 8px;border:none;border-radius:10px;font-size:.75rem;font-weight:bold;color:#fff;text-transform:uppercase}
.btn:active{transform:scale(.95)}
.c1{background:linear-gradient(145deg,#ff6b6b,#ee5a5a)}
.c2{background:linear-gradient(145deg,#ffa500,#e69500)}
.c3{background:linear-gradient(145deg,#4ecdc4,#44b3ab)}
.c4{background:linear-gradient(145deg,#a855f7,#9333ea)}
.c5{background:linear-gradient(145deg,#22c55e,#16a34a)}
.c6{background:linear-gradient(145deg,#ec4899,#db2777)}
#log{background:#111;padding:10px;border-radius:8px;font-family:monospace;font-size:.8rem;max-height:100px;overflow-y:auto;color:#888}
</style>
</head>
<body>
<h1>🤖 Sheldon Remote</h1>
<div class="status"><span class="dot" id="dot"></span><span id="st">Checking...</span></div>
<button class="stop" onclick="stop()">⛔ EMERGENCY STOP</button>
<div class="grid">
<button class="btn c1" onclick="ev('COLLISION')">💥 Collision</button>
<button class="btn c2" onclick="ev('STUCK')">🚧 Stuck</button>
<button class="btn c3" onclick="ev('RESET')">🔄 Reset</button>
<button class="btn c4" onclick="ev('BOOT')">🚀 Boot</button>
<button class="btn c5" onclick="ev('SAW_HUMAN')">👀 Human</button>
<button class="btn c6" onclick="ev('RANDOM')">🎲 Random</button>
</div>
<div id="log">Ready</div>
<script>
const B='http://192.168.4.1';
function log(m){document.getElementById('log').innerHTML=m+'<br>'+document.getElementById('log').innerHTML}
function ev(e){log('📤 '+e);fetch(B+'/event',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({event:e})}).then(r=>r.json()).then(d=>log('✅ Sent')).catch(x=>log('❌ Error'))}
function stop(){log('🛑 STOP');fetch(B+'/stop',{method:'POST'}).then(r=>log('Stopped')).catch(x=>log('❌ Error'))}
function ck(){fetch(B+'/status').then(r=>r.json()).then(d=>{document.getElementById('dot').className='dot ok';document.getElementById('st').textContent='Connected'}).catch(x=>{document.getElementById('dot').className='dot';document.getElementById('st').textContent='Disconnected'})}
setInterval(ck,2000);ck();
</script>
</body>
</html>
)rawliteral";

// Character Voice Page (for iPhone mounted on robot)
const char VOICE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Sheldon Voice</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,sans-serif;background:#0a0a0a;color:#fff;min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center;padding:20px}
h1{font-size:2rem;color:#4ecdc4;margin-bottom:20px}
#status{font-size:1rem;color:#888;margin-bottom:30px}
#status.ok{color:#44ff44}
#event{font-size:3rem;margin-bottom:20px;min-height:60px}
#speech{font-size:1.2rem;color:#4ecdc4;font-style:italic;text-align:center;padding:20px;min-height:80px}
#start{padding:30px 60px;font-size:1.5rem;background:#22c55e;border:none;border-radius:20px;color:#fff;margin-bottom:20px}
.hidden{display:none}
</style>
</head>
<body>
<button id="start" onclick="init()">🔊 TAP TO START</button>
<div id="main" class="hidden">
<h1>🤖 Sheldon Voice</h1>
<div id="status">Connecting...</div>
<div id="event">--</div>
<div id="speech">"Waiting for events..."</div>
</div>
<script>
const LINES={
BOOT:["Systems online. I already regret this.","Boot complete. Try not to break anything."],
COLLISION:["This is malarkey!","We have made contact with an obstacle. How pedestrian.","That was entirely predictable."],
STUCK:["I am overwhelmed!","I appear to be immobilized. This is your fault.","The universe is testing my patience."],
RESET:["Resetting. Again. Predictable.","Fine. We shall try this again."],
SAW_HUMAN:["You are doomed!","A human. How... unfortunate.","I see you there. Do not touch me."],
RANDOM:["Bazinga!","Have you suffered a recent blow to the head?","I am not crazy. My mother had me tested."]
};
let synth=window.speechSynthesis;
function init(){
document.getElementById('start').classList.add('hidden');
document.getElementById('main').classList.remove('hidden');
let u=new SpeechSynthesisUtterance("Initializing Sheldon protocol.");
synth.speak(u);
poll();
setInterval(poll,500);
}
function poll(){
fetch('http://192.168.4.1/event').then(r=>r.json()).then(d=>{
document.getElementById('status').textContent='Connected';
document.getElementById('status').className='ok';
if(d.event && d.event!==''){
let ev=d.event;
document.getElementById('event').textContent=ev;
speak(ev);
}
}).catch(e=>{
document.getElementById('status').textContent='Disconnected';
document.getElementById('status').className='';
});
}
function speak(ev){
let lines=LINES[ev]||LINES['RANDOM'];
let line=lines[Math.floor(Math.random()*lines.length)];
document.getElementById('speech').textContent='"'+line+'"';
synth.cancel();
let u=new SpeechSynthesisUtterance(line);
u.rate=0.9;u.pitch=1.1;
synth.speak(u);
}
</script>
</body>
</html>
)rawliteral";

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

void handleRoot() {
    server.send(200, "text/html", REMOTE_HTML);
}

void handleVoice() {
    server.send(200, "text/html", VOICE_HTML);
}

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
    
    JsonDocument doc;
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
    
    JsonDocument doc;
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
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc["event"].is<const char*>()) {
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
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc["mode"].is<const char*>()) {
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
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc["id"].is<const char*>()) {
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
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error && doc["x"].is<int>() && doc["y"].is<int>()) {
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
    server.on("/", HTTP_GET, handleRoot);  // Serve remote control page
    server.on("/voice", HTTP_GET, handleVoice);  // Serve voice/speaker page
    
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
