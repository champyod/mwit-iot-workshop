#include "MWIT_WiFi.h"
#include <esp_wifi.h>
#include <Log.h>

MWITWiFi* MWITWiFi::self_ = nullptr;

MWITWiFi::MWITWiFi(const char* ssid,
                   const char* eap_identity,
                   const char* eap_username,
                   const char* eap_password)
    : ssid_(ssid)
    , eap_identity_(eap_identity)
    , eap_username_(eap_username)
    , eap_password_(eap_password) {
    self_ = this;
}

void MWITWiFi::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(100);
    esp_wifi_set_ps(WIFI_PS_NONE);

    Logger.printf("[WiFi] Scanning for \"%s\"...\n", ssid_);
    int n = WiFi.scanNetworks();

    WiFi.onEvent(onGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    esp_wifi_sta_wpa2_ent_set_identity(
        (uint8_t*)eap_identity_, strlen(eap_identity_));
    esp_wifi_sta_wpa2_ent_set_username(
        (uint8_t*)eap_username_, strlen(eap_username_));
    esp_wifi_sta_wpa2_ent_set_password(
        (uint8_t*)eap_password_, strlen(eap_password_));
    esp_wifi_sta_wpa2_ent_set_disable_time_check(true);
    esp_wifi_sta_wpa2_ent_enable();

    Logger.printf("[WiFi] Connecting to %s ...\n", ssid_);
    WiFi.begin(ssid_);

    // Block forever until connected
    while (!gotIP_) {
        delay(100);
    }

    Logger.printf("[WiFi] Connected! IP: %s\n", localIP().toString().c_str());
}

bool MWITWiFi::connected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress MWITWiFi::localIP() {
    return WiFi.localIP();
}

int8_t MWITWiFi::rssi() {
    return WiFi.RSSI();
}

void MWITWiFi::onGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (self_) self_->gotIP_ = true;
}

void MWITWiFi::onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    uint8_t reason = info.wifi_sta_disconnected.reason;
    Logger.printf("[WiFi] Disconnected! Reason: %d — reconnecting...\n", reason);
    if (self_) self_->gotIP_ = false;
    WiFi.reconnect();
}