#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>
#include "credentials.h"

const char* ssid = WIFI_SSID;
const char* eap_identity = WIFI_IDENTITY;
const char* eap_username = WIFI_USERNAME;
const char* eap_password = WIFI_PASSWORD;

bool targetFound = false;

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t reason = info.wifi_sta_disconnected.reason;
  Serial.print("\nDisconnected! Reason: ");
  switch (reason) {
    case 1:  Serial.println("1 - Unspecified"); break;
    case 2:  Serial.println("2 - Auth expired"); break;
    case 6:  Serial.println("6 - Not authenticated (wrong credentials?)"); break;
    case 15: Serial.println("15 - 4-way handshake timeout"); break;
    case 19: Serial.println("19 - No AP found (check SSID)"); break;
    case 22: Serial.println("22 - 802.1X auth failed (wrong username/password)"); break;
    case 23: Serial.println("23 - Cipher rejected"); break;
    case 24: Serial.println("24 - Beacon timeout"); break;
    case 25: Serial.println("25 - No AP found"); break;
    case 201: Serial.println("201 - No SSID found"); break;
    case 202: Serial.println("202 - Auth failed (check credentials)"); break;
    default: Serial.println(reason); break;
  }
  delay(3000);
  WiFi.reconnect();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  // Scan for available networks
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks:");

  for (int i = 0; i < n; i++) {
    Serial.print("  ");
    Serial.print(i + 1);
    Serial.print(". ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(" dBm)");

    if (WiFi.SSID(i) == ssid) {
      targetFound = true;
    }
  }

  if (!targetFound) {
    Serial.print("\nTarget SSID \"");
    Serial.print(ssid);
    Serial.println("\" not found in scan!");
    Serial.println("Check the exact SSID name above and update the code.");
    return;
  }

  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)eap_identity, strlen(eap_identity));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t*)eap_username, strlen(eap_username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t*)eap_password, strlen(eap_password));
  esp_wifi_sta_wpa2_ent_set_disable_time_check(true);
  esp_wifi_sta_wpa2_ent_enable();

  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 10000) {
      lastPrint = millis();
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
  }
}
