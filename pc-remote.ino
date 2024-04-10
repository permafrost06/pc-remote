#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>

String serverName = "http://192.168.0.106:8000";
String getPCOnRequestURL = serverName + "/esp/pc/check-request";
String markReqFulfilledURL = serverName + "/esp/pc/request-fulfilled";
String getResetRequestURL = serverName + "/esp/pc/check-reset-request";
String markResetReqFulfilledURL = serverName + "/esp/pc/reset-request-fulfilled";
String getPowerButtonRequestURL = serverName + "/esp/pc/check-power-button-request";
String markPowerButtonReqFulfilledURL = serverName + "/esp/pc/power-button-request-fulfilled";
String markPCOnURL = serverName + "/esp/pc/mark-as-on";
String markPCOffURL = serverName + "/esp/pc/mark-as-off";

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

WiFiClient client;
HTTPClient http;

const int output4 = 4;
const int output0 = 0;
const int input5 = 5;

String getPayload(String endpoint) {
    http.begin(client, endpoint.c_str());
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

    String ssid, psk, secret;
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

    String message = getPayload(serverName + "/esp/pc/mark-as-" + getCurrentPCState());
    Serial.println(message);
    Serial.print("input5 - pc state: ");
    Serial.println(digitalRead(input5));
    
    bool turnOnReq = getPayloadAsBool(getPCOnRequestURL);
    Serial.println("Turn on requested: " + turnOnReq);
    if (turnOnReq && digitalRead(input5)) {
        turnOnPC();
        Serial.println(getPayload(markReqFulfilledURL));
    }

    if (!digitalRead(input5)) {
        bool resetReq = getPayloadAsBool(getResetRequestURL);
        Serial.println("Reset requested: " + resetReq);
        if (resetReq && !digitalRead(input5)) {
            resetPC();
            Serial.println(getPayload(markResetReqFulfilledURL));
        }
    }

    bool powerButtonReq = getPayloadAsBool(getPowerButtonRequestURL);
    Serial.println("Power button press requested: " + powerButtonReq);
    if (powerButtonReq) {
        pressPowerButton();
        Serial.println(getPayload(markPowerButtonReqFulfilledURL));
    }
    
    lastTime = millis();
}

