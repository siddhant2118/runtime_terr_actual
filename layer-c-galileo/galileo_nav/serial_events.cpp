#include "serial_events.h"
#include "config.h"

#ifdef USE_SOFTWARE_SERIAL
    SoftwareSerial EspSerial(PIN_RX_FROM_ESP, PIN_TX_TO_ESP);
    #define COMM_PORT EspSerial
#else
    #define COMM_PORT ESP_SERIAL
#endif

void setupSerialEvents() {
    COMM_PORT.begin(SERIAL_BAUD_RATE);
    delay(100);
}

void sendEvent(const char* eventName) {
    if (eventName != NULL) {
        COMM_PORT.println(eventName);
        // Debug output to main USB serial as well
        Serial.print("DEBUG SENT: ");
        Serial.println(eventName);
    }
}
