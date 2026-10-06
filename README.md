# Temperature_Logger

ESP32가 DHT22로 온도를 10초마다 측정해 PC의 HTTP 서버로 GET 요청(쿼리 문자열)으로 보내고, 동시에 자체 웹 서버도 띄우는 실습 예제.

## 개요

ESP32 한 대가 HTTP 클라이언트와 HTTP 서버 역할을 함께 한다. 클라이언트로서는 DHT22 온도 값을 `?temperature=<값>` 형태로 외부 서버(포트 8000)에 보내고, 서버로서는 포트 80에서 같은 형식의 요청을 받아 값을 시리얼에 기록한다.

## 하드웨어

- 보드: ESP32 DOIT DevKit V1 (`esp32doit-devkit-v1`)
- 센서: DHT22 온습도 센서

| 장치 | 신호 | ESP32 핀 |
|------|------|----------|
| DHT22 | DATA | GPIO 15 (`DHTPIN`) |

## 동작 방식

### 측정 및 전송 (클라이언트)

- `loop()`에서 `millis()` 기준 10초(10000 ms)마다 `dht.getTempAndHumidity()` 호출
- 온도 값이 `NaN`이 아니면 `http://<서버 주소>:8000/?temperature=<온도>`로 GET 요청
- 응답 코드를 출력하고, 200이면 응답 본문도 시리얼에 출력
- 습도 값은 읽지만 전송하지 않는다

### 내장 웹 서버 (포트 80)

| 경로 | 동작 |
|------|------|
| `/` | `temperature` 인자가 있으면 시리얼에 기록하고 `Temperature received: <값> °C` 응답(200), 없으면 `Temperature parameter missing!` 응답(400) |
| `/inline` | `Hello from the inline function` 응답(200) |
| 그 외 | `File Not Found` 응답(404) |

- mDNS 이름 `server` 등록 (`server.local`)

## 개발 환경

- PlatformIO, platform `espressif32`, framework `arduino`
- lib_deps: `beegee-tokyo/DHT sensor library for ESPx@^1.18` (`DHTesp`)
- 내장 라이브러리: `WiFi.h`, `ESPmDNS.h`, `WebServer.h`, `HTTPClient.h`
- 업로드 속도 460800, 시리얼 모니터 속도 115200

## 설정

`src/main.cpp`에서 다음 값을 바꾼다.

- `ssid`, `password` : 접속할 WiFi 정보
- `loop()`의 `serverPath` : 온도를 받을 PC 서버의 IP 주소와 포트

## 빌드 및 실행

```bash
pio run -t upload
pio device monitor -b 115200
```

온도를 받을 쪽에는 `serverPath`에 적은 주소·포트(8000)에서 HTTP 서버가 실행 중이어야 한다.

## 폴더 구조

```
Temperature_Logger/
├── platformio.ini   # 보드·속도·DHT 라이브러리 설정
└── src/
    └── main.cpp     # DHT22 측정 + HTTP GET 전송 + 웹 서버
```
