#include <Arduino.h>
#include <MWIT_WiFi.h>
#include <Log.h>
#include <OTA.h>

MWITWiFi wifi("MWIT-WiFi", "s6709222", "s6709222", "g++-std=c++17");
OTA ota("ota-bootstrap");

void setup() {
    Logger.begin(115200);
    Logger.println("=== OTA Bootstrap ===\n");

    wifi.begin();
    ota.begin();
}

void loop() {
    ota.handle();
    Logger.handle();

    static unsigned long lastPing = 0;
    if (millis() - lastPing > 5000) {
        lastPing = millis();
        Logger.printf("[Alive] IP: %s | RSSI: %d dBm | Telnet clients: %d\n",
                      wifi.localIP().toString().c_str(),
                      wifi.rssi(), Logger.connectedClients());
    }

    delay(10);
}
