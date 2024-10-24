#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <DHTesp.h>
#include <HTTPClient.h>

#define DHTPIN 15         // DHT22 데이터 핀(GPIO 15)
#define DHTTYPE DHT22     // DHT22 센서 유형 설정

// WiFi 연결 정보
const char* ssid = "여기에_WiFi_이름을_입력하세요";       // WiFi 네트워크 이름(SSID)
const char* password = "여기에_WiFi_비밀번호를_입력하세요";    // WiFi 비밀번호

DHTesp dht; // DHT 센서 객체 생성
WebServer server(80); // 웹 서버 객체 생성 (포트 80)

void handleRoot() {
    // URL에서 쿼리 문자열을 파싱하여 온도 값을 가져옴
    if (server.hasArg("temperature")) {
        String temperature = server.arg("temperature");
        Serial.printf("Received temperature: %s °C\n", temperature.c_str()); // 전달된 온도 값 출력
        server.send(200, "text/plain", "Temperature received: " + temperature + " °C\n");
    } else {
        String message = "Temperature parameter missing!\n";
        server.send(400, "text/plain", message);
    }
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    server.send(404, "text/plain", message);
}

void setup(void) {
    Serial.begin(115200);
    dht.setup(DHTPIN, DHTesp::DHT22); // DHT 센서 초기화

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // WiFi 연결을 기다림
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected to %s, and Server IP : %s\n", ssid, WiFi.localIP().toString().c_str());
    MDNS.begin("server"); // mDNS responder 시작 ('server.local'로 접근 가능)

    // 루트 경로 요청 처리 설정
    server.on("/", handleRoot);

    // 인라인 핸들러 설정
    server.on("/inline", [](){
        server.send(200, "text/plain", "Hello from the inline function\n");
    });

    // 경로를 찾을 수 없는 경우 처리 설정
    server.onNotFound(handleNotFound);

    // 웹 서버 시작
    server.begin();
    Serial.println("HTTP server started");
}

void loop(void) {
    server.handleClient(); // 클라이언트 요청 처리

    // 10초마다 DHT22에서 온도를 측정하고 HTTP 요청 전송
    static unsigned long lastTime = 0;
    unsigned long now = millis();
    if (now - lastTime >= 10000) { // 10초 간격
        lastTime = now;
        TempAndHumidity data = dht.getTempAndHumidity();
        if (!isnan(data.temperature)) {
            // GET 요청 전송
            WiFiClient client;
            HTTPClient http;
            String serverPath = "http://여기에_서버_IP를_입력하세요:8000/?temperature=" + String(data.temperature);
            Serial.print("[HTTP] begin...\n");
            if (http.begin(client, serverPath)) {
                Serial.print("[HTTP] GET...\n");
                int httpCode = http.GET();
                if (httpCode > 0) {
                    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
                    if (httpCode == HTTP_CODE_OK) {
                        String payload = http.getString();
                        Serial.println(payload);
                    }
                } else {
                    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
                }
                http.end();
            } else {
                Serial.printf("[HTTP] Unable to connect\n");
            }
        }
    }
}