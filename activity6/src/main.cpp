#include <Arduino.h>
#include <MWIT_WiFi.h>
#include <Log.h>
#include <DHT22_Sensor.h>
#include <GG_Sheet.h>
#include <LED.h>
#include <OTA.h>
#include <time.h>

#define DHTPIN          32
#define LED_READING_PIN 5
#define LED_ERROR_PIN   2

MWITWiFi wifi("MWIT-WiFi", "s6709222", "s6709222", "g++-std=c++17");
OTA ota("activity6-ota");

GGSheet sheet("script.google.com",
    "/macros/s/AKfycbwark0PAVrM48JAUuynaW8O_9JuJmEOfvVuC5N4PqbBsdD-1OHRUaRd3FusJAWCSQo4ww/exec");

DHT22_Sensor dht22(DHTPIN);
LED ledReading(LED_READING_PIN);
LED ledError(LED_ERROR_PIN);

const unsigned long SEND_INTERVAL = 5000;
char runStart[24];

void setup() {
    Logger.begin(115200);
    Logger.println("=== Activity 6 ===\n");

    ledReading.off();
    ledError.off();
    dht22.begin();

    wifi.begin();
    ota.begin();

    configTime(0, 0, "pool.ntp.org", "time.google.com");
    Logger.print("[NTP] Syncing");
    struct tm timeinfo;
    for (int i = 0; i < 30; i++) {
        if (getLocalTime(&timeinfo)) {
            strftime(runStart, sizeof(runStart), "%Y-%m-%d_%H-%M-%S", &timeinfo);
            Logger.printf(" OK — start: %s\n", runStart);
            break;
        }
        Logger.print(".");
        delay(500);
    }
    if (runStart[0] == '\0') {
        strcpy(runStart, "unknown");
        Logger.println(" FAILED — using 'unknown'");
    }
}

void loop() {
    static unsigned long lastSend = 0;
    unsigned long now = millis();

    ota.handle();
    Logger.handle();

    if (!wifi.connected()) {
        ledReading.blink(5, 700);
        delay(500);
        return;
    }

    if (now - lastSend >= SEND_INTERVAL) {
        lastSend = now;

        ledReading.on();
        auto data = dht22.read();
        ledReading.off();
        if (data.valid) {
            bool ok = sheet.send(runStart, data.temperature, data.humidity,
                                 data.heatIndex);
            if (!ok) {
                ledError.blinkCount(3);
            }
        }
    }

    ledError.tick();
    delay(100);
}
