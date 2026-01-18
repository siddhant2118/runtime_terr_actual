#include <Arduino.h>

// WORKING CONFIGURATION (Verified 2026-01-18)
// Pins: 4, 5, 6, 7
// Logic: Loop Forward 2s, Stop 2s

#define IN1 4
#define IN2 5
#define IN3 6
#define IN4 7

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n\n============================================");
  Serial.println("   ✅ ARCHIVED TEST CODE: MOTORS ONLY       ");
  Serial.println("============================================");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void loop() {
  // BOTH FORWARD
  Serial.println("LOOP: Motors FORWARD (2s)...");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(2000);

  // STOP
  Serial.println("LOOP: Motors STOP (2s)...");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(2000);
}
