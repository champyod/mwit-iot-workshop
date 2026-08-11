#include <Arduino.h>
#include <MWIT_WiFi.h>
#include <Log.h>
#include <DHT22_Sensor.h>
#include <DiscordWebhook.h>
#include <OTA.h>
#include <time.h>
#include "credentials.h"

#define DHTPIN          21
#define LED_ERROR_PIN   2

#define TEMP_THRESHOLD  30.0
#define HUM_THRESHOLD   80.0

static const char* WEBHOOK_URL = "https://discord.com/api/webhooks/1529030694695866459/ANRIp55vyJrnSiJqOO862UAMtasR7JAUjDNecSMEL_xuRrM4JJN6uTY2S3HAeaRiqz_b";

MWITWiFi wifi(WIFI_SSID, WIFI_IDENTITY, WIFI_USERNAME, WIFI_PASSWORD);
OTA ota("activity7-ota");
DiscordWebhook webhook(WEBHOOK_URL);
DHT22_Sensor dht22(DHTPIN);

enum class RoomState { NORMAL, ALERT };
RoomState state = RoomState::NORMAL;

const unsigned long READ_INTERVAL = 5000;

void setup() {
    Logger.begin(115200);
    Logger.println("=== Activity 7  |  Smart Monitor ===\n");

    dht22.begin();

    wifi.begin();
    ota.begin();

    configTime(0, 0, "pool.ntp.org", "time.google.com");
    Logger.print("[NTP] Syncing");
    struct tm timeinfo;
    for (int i = 0; i < 30; i++) {
        if (getLocalTime(&timeinfo)) {
            Logger.println(" OK");
            break;
        }
        Logger.print(".");
        delay(500);
    }
    if (!getLocalTime(&timeinfo)) {
        Logger.println(" FAILED");
    }

    Logger.printf("[Setup] Thresholds: >%.0f C  or  >%.0f %% RH\n",
                  TEMP_THRESHOLD, HUM_THRESHOLD);
    Logger.printf("[Setup] Reading interval: %lu s\n", READ_INTERVAL / 1000);
    Logger.printf("[Setup] Webhook target: %s\n", WEBHOOK_URL);
    Logger.printf("[Setup] WiFi RSSI: %d dBm\n\n", WiFi.RSSI());
}

void loop() {
    static unsigned long lastRead = 0;
    unsigned long now = millis();

    ota.handle();
    Logger.handle();

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

        Logger.printf("[Sensor] T=%.1f C  H=%.1f %%  HI=%.1f C  RSSI=%d dBm\n",
                      data.temperature, data.humidity, data.heatIndex, WiFi.RSSI());

        bool breached = (data.temperature > TEMP_THRESHOLD) ||
                        (data.humidity > HUM_THRESHOLD);
        Logger.printf("[State] %s  breached=%d  T>%.0f=%s  H>%.0f=%s\n",
                      state == RoomState::NORMAL ? "NORMAL" : "ALERT",
                      breached,
                      TEMP_THRESHOLD,
                      data.temperature > TEMP_THRESHOLD ? "YES" : "no",
                      HUM_THRESHOLD,
                      data.humidity > HUM_THRESHOLD ? "YES" : "no");

        if (state == RoomState::NORMAL && breached) {
            Logger.printf("[Alert] T=%.1f H=%.1f — sending alert embed\n",
                          data.temperature, data.humidity);
            webhook.sendAlert(data.temperature, data.humidity, data.heatIndex);
            state = RoomState::ALERT;

        } else if (state == RoomState::ALERT && !breached) {
            Logger.printf("[Alert] T=%.1f H=%.1f — sending recovery embed\n",
                          data.temperature, data.humidity);
            webhook.sendRecovery(data.temperature, data.humidity, data.heatIndex);
            state = RoomState::NORMAL;

        } else if (state == RoomState::ALERT && breached) {
            Logger.printf("[Alert] Still in ALERT — suppressing repeat\n");
        }
    }

    delay(100);
}
