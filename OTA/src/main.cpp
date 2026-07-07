#include <Arduino.h>
#include <MWIT_WiFi.h>
#include <Log.h>
#include <OTA.h>
#include "credentials.h"

MWITWiFi wifi(WIFI_SSID, WIFI_IDENTITY, WIFI_USERNAME, WIFI_PASSWORD);
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
