#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <FS.h>

String ssid, psk, secret, baseURL;
String getPCOnRequestEndpoint = "/esp/pc/check-request";
String markReqFulfilledEndpoint = "/esp/pc/request-fulfilled";
String getResetRequestEndpoint = "/esp/pc/check-reset-request";
String markResetReqFulfilledEndpoint = "/esp/pc/reset-request-fulfilled";
String getPowerButtonRequestEndpoint = "/esp/pc/check-power-button-request";
String markPowerButtonReqFulfilledEndpoint = "/esp/pc/power-button-request-fulfilled";
String markPCOnURL = "/esp/pc/mark-as-on";
String markPCOffURL = "/esp/pc/mark-as-off";

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

const int output4 = 4;
const int output0 = 0;
const int input5 = 5;

String getPayload(String endpoint) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    HTTPClient http;

    http.begin(*client, endpoint);
    http.addHeader("Secret-Code", secret);
    http.GET();
    String payload = http.getString();
    http.end();
    return payload;
}

bool getPayloadAsBool(String endpoint) {
    String payload = getPayload(endpoint);
    if (payload == "1") {
        return true;
    }
    return false;
}

void turnOnPC() {
    while (digitalRead(input5) != 0) {
        digitalWrite(output4, LOW);
    }
    digitalWrite(output4, HIGH);
}

void pressPowerButton() {
    digitalWrite(output4, LOW);
    delay(500);
    digitalWrite(output4, HIGH);
}

void resetPC() {
    digitalWrite(output0, LOW);
    delay(500);
    digitalWrite(output0, HIGH);
}

String getCurrentPCState() {
    if (digitalRead(input5)) {
        return "off";
    }
    return "on";
}

void setup() {
    Serial.begin(115200);

    pinMode(input5, INPUT_PULLUP);

    pinMode(output4, OUTPUT);
    pinMode(output0, OUTPUT);
    digitalWrite(output4, HIGH);
    digitalWrite(output0, HIGH);

    if (!SPIFFS.begin()) {
        Serial.println("spiffs could not be initialized");
        return;
    }
    
    File configFile = SPIFFS.open("/config.txt", "r");

    if (!configFile) {
        Serial.println("Failed to open file for reading");
        return;
    }

    String config;
    while (configFile.available()) {
        config += char(configFile.read());
    }
    configFile.close();

    String key, val;
    bool keyRecorded = false;

    for (int i = 0; i < config.length(); i++) {
        char c = config[i];
        if (c == '\n') {
            if (key.equals("ssid")) {
                ssid = val; 
            }
            if (key.equals("psk")) {
                psk = val; 
            }
            if (key.equals("secretkey")) {
                secret = val; 
            }
            if (key.equals("baseurl")) {
                baseURL = val; 
            }
            key = "";
            val = "";
            keyRecorded = false;
            continue;
        }
        if (c == '=') {
            keyRecorded = true;
            continue;
        }
        if (keyRecorded) {
            val += c;
        } else {
            key += c;
        }
    }

    WiFi.begin(ssid, psk);
}

void loop() {
    if ((millis() - lastTime) <= timerDelay) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected");
        return;
    }

    String message = getPayload(baseURL + "/esp/pc/mark-as-" + getCurrentPCState());
    Serial.println(message);
    Serial.print("input5 - pc state: ");
    Serial.println(digitalRead(input5));
    
    bool turnOnReq = getPayloadAsBool(baseURL + getPCOnRequestEndpoint);
    Serial.println("Turn on requested: " + String(turnOnReq));
    if (turnOnReq && digitalRead(input5)) {
        turnOnPC();
        Serial.println(getPayload(baseURL + markReqFulfilledEndpoint));
    }

    if (!digitalRead(input5)) {
        bool resetReq = getPayloadAsBool(baseURL + getResetRequestEndpoint);
        Serial.println("Reset requested: " + String(resetReq));
        if (resetReq && !digitalRead(input5)) {
            resetPC();
            Serial.println(getPayload(baseURL + markResetReqFulfilledEndpoint));
        }
    }

    bool powerButtonReq = getPayloadAsBool(baseURL + getPowerButtonRequestEndpoint);
    Serial.println("Power button press requested: " + String(powerButtonReq));
    if (powerButtonReq) {
        pressPowerButton();
        Serial.println(getPayload(baseURL + markPowerButtonReqFulfilledEndpoint));
    }
    
    lastTime = millis();

    Serial.println(baseURL);
}

