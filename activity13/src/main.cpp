#define NO_GLOBAL_BLYNK

#include "credentials.blynk.h"
#include <Arduino.h>
#include <MWIT_WiFi.h>
#include <Log.h>
#include <DHT22_Sensor.h>
#include <OTA.h>
#include <BlynkClient.h>
#include <BlynkSimpleEsp32.h>
#include "credentials.h"

#define DHTPIN          21
#define LED_PIN         2
#define READ_INTERVAL   5000

#define VPIN_LED        0
#define VPIN_TEMP       1
#define VPIN_HUM        2

MWITWiFi wifi(WIFI_SSID, WIFI_IDENTITY, WIFI_USERNAME, WIFI_PASSWORD);
OTA ota("activity13-ota");
BlynkClient blynk;
DHT22_Sensor dht22(DHTPIN);

static double ledDuty = 0.0;

// Blynk app slider (V0, 0..1) → analog output on LED_PIN
BLYNK_WRITE(VPIN_LED) {
    ledDuty = constrain((double)param.asDouble(), 0.0, 1.0);
    analogWrite(LED_PIN, (int)(ledDuty * 255.0));
    Logger.printf("[Blynk] LED duty = %.2f (V%d)\n", ledDuty, VPIN_LED);
}

void setup() {
    Logger.begin(115200);
    Logger.println("=== Activity 13  |  Blynk Smart Room ===\n");

    dht22.begin();

    wifi.begin();
    ota.begin();

    analogWrite(LED_PIN, 0);  // start LED off

    blynk.begin();
    Logger.printf("[Blynk] Connecting... (RSSI %d dBm)\n", WiFi.RSSI());
}

void loop() {
    static unsigned long lastRead = 0;
    unsigned long now = millis();

    ota.handle();
    Logger.handle();
    blynk.handle();

    if (!wifi.connected()) {
        Logger.printf("[WiFi] Disconnected (status %d), waiting...\n", WiFi.status());
        delay(500);
        return;
    }

    if (now - lastRead >= READ_INTERVAL) {
        lastRead = now;

        auto data = dht22.read();
        if (!data.valid) {
            Logger.printf("[Sensor] Read FAILED (pin %d)\n", DHTPIN);
            return;
        }

        Logger.printf("[Sensor] T=%.1f C  H=%.1f %%\n", data.temperature, data.humidity);
        blynk.writeVp(VPIN_TEMP, data.temperature);
        blynk.writeVp(VPIN_HUM, data.humidity);
    }

    delay(100);
}
