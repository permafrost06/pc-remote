#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "Pulse";
const char* password = "easypassword1234";

String serverName = "http://192.168.0.106:8000";
String getPCOnRequestURL = serverName + "/esp/pc/check-request";
String markReqFulfilledURL = serverName + "/esp/pc/request-fulfilled";
String getResetRequestURL = serverName + "/esp/pc/check-reset-request";
String markResetReqFulfilledURL = serverName + "/esp/pc/reset-request-fulfilled";
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

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.print("\nLocal IP: ");
    Serial.println(WiFi.localIP());
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
        Serial.println("Turn on requested: " + resetReq);
        if (resetReq && !digitalRead(input5)) {
            resetPC();
            Serial.println(getPayload(markResetReqFulfilledURL));
        }
    }
    
    lastTime = millis();
}
