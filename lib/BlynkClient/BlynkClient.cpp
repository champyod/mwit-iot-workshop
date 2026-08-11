#include <credentials.blynk.h>
#include <BlynkClient.h>
#include <BlynkSimpleEsp32.h>

static const uint32_t RECONNECT_INTERVAL_MS = 5000;

BlynkClient::BlynkClient() {}

void BlynkClient::begin() {
    // WiFi is connected by the caller (MWIT enterprise) before this runs,
    // so Blynk.config() must be used — NOT Blynk.begin(auth, ssid, pass)
    // which would restart WiFi with a PSK profile.
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    began_ = true;
}

void BlynkClient::handle() {
    if (!began_) return;
    if (!Blynk.connected() && millis() - lastReconnectMs_ >= RECONNECT_INTERVAL_MS) {
        lastReconnectMs_ = millis();
        Blynk.connect();
    }
    Blynk.run();
}

bool BlynkClient::connected() {
    return Blynk.connected();
}

void BlynkClient::writeVp(int vpin, double value) {
    if (Blynk.connected()) Blynk.virtualWrite(vpin, value);
}

void BlynkClient::writeVp(int vpin, int value) {
    if (Blynk.connected()) Blynk.virtualWrite(vpin, value);
}

void BlynkClient::writeVp(int vpin, const char* value) {
    if (Blynk.connected()) Blynk.virtualWrite(vpin, value);
}