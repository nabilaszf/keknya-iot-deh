// =================================================================
// SUGENG NODE — HTTP POST Direct to Backend (WiFi Mode)
// ESP32 DevKit V1 + DHT22 + JSN-SR04T + MQ-135 + MQ-4
//
// Alur: ESP32 connect WiFi -> Baca Sensor -> Kirim HTTP POST ke
// Middleware Elysia.js (/api/telemetry) -> Sleep/Delay.
// =================================================================
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

// ---------- KONFIGURASI WIFI & BACKEND ----------
const char* WIFI_SSID     = "SUGENG";       // Ubah dengan SSID WiFi
const char* WIFI_PASSWORD = "2wsx1qaz";   // Ubah dengan Password WiFi

// URL Endpoint Backend Middleware Elysia.js
// Contoh Lokal: "http://192.168.1.5:3000/api/telemetry"
// Contoh Vercel: "https://be-sugeng.vercel.app/api/telemetry"
const char* BACKEND_URL   = "https://be-sugeng.vercel.app/api/telemetry";
const char* NODE_TOKEN    = "sugeng-secret-token-mgl-2026"; // Sesuaikan dengan NODE_API_KEY di .env backend

// ---------- CONFIG NODE & PIN ----------
const char* NODE_ID = "NODE-MGL-05";

#define DHTPIN    19
#define DHTTYPE   DHT22
#define TRIG_PIN  5
#define ECHO_PIN  18
#define MQ135_PIN 34
#define MQ4_PIN   35

// ---------- KONSTANTA BAK 120L ----------
#define H_TOTAL_CM 95.0
#define S_BLIND_CM 20.0

DHT dht(DHTPIN, DHTTYPE);

// Cache nilai sensor (fallback jika error)
float lastTemp = 0, lastHum = 0, lastDist = -1, lastVol = 0;
int lastMq135Adc = 0, lastMq4Adc = 0;
float lastMq135Pct = 0, lastMq4Pct = 0;
unsigned long bootMs = 0;

// ---------- FUNGSI PERHITUNGAN & MAPPING ----------
float hitungVolumePct(float jarakCm) {
  if (jarakCm < 0) return lastVol; // Jarak out of range -> gunakan cache nilai lama
  float v = (H_TOTAL_CM - jarakCm) / (H_TOTAL_CM - S_BLIND_CM) * 100.0;
  if (v < 0) v = 0;
  if (v > 100) v = 100;
  return v;
}

float mq135PctToPpm(float pct) { return pct * 1.2; }
float mq4PctToPpm(float pct)   { return pct * 4.0; }

// ---------- PEMBACAAN SENSOR ----------
void bacaSensor() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  if (!isnan(temp)) lastTemp = temp;
  if (!isnan(hum))  lastHum  = hum;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float dist = (duration > 0) ? (duration * 0.0343 / 2.0) : -1.0;
  lastDist = dist;
  lastVol = hitungVolumePct(dist);

  lastMq135Adc = analogRead(MQ135_PIN);
  lastMq4Adc   = analogRead(MQ4_PIN);
  lastMq135Pct = (lastMq135Adc / 4095.0) * 100.0;
  lastMq4Pct   = (lastMq4Adc / 4095.0) * 100.0;
}

// ---------- KONEKSI WIFI ----------
void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] Connection Failed! Will retry next loop.");
  }
}

// ---------- PENGIRIMAN DATA HTTP POST ----------
void kirimDataToBackend() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] Skip send: WiFi disconnect!");
    return;
  }

  HTTPClient http;
  http.begin(BACKEND_URL);

  // Set Header sesuai Spesifikasi Backend Elysia.js SUGENG
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Node-Token", NODE_TOKEN);

  // Buat JSON Document (ukuran 256 byte cukup untuk payload telemetri)
  StaticJsonDocument<256> doc;
  doc["nodeId"]               = NODE_ID;
  doc["temperatureCelsius"]   = serialized(String(lastTemp, 1));
  doc["humidityPercent"]      = serialized(String(lastHum, 1));
  doc["measuredDistanceCm"]   = serialized(String(lastDist, 1));
  doc["currentVolumePercent"] = serialized(String(lastVol, 1));
  doc["gasPpmMQ135"]          = serialized(String(mq135PctToPpm(lastMq135Pct), 1));
  doc["methanePpmMQ4"]        = serialized(String(mq4PctToPpm(lastMq4Pct), 1));
  doc["mq135Adc"]             = lastMq135Adc;
  doc["mq4Adc"]               = lastMq4Adc;
  doc["mq135Pct"]             = serialized(String(lastMq135Pct, 1));
  doc["mq4Pct"]               = serialized(String(lastMq4Pct, 1));
  doc["uptimeSec"]            = (millis() - bootMs) / 1000;

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.println("[HTTP] Sending POST Payload to Backend...");
  Serial.println(jsonString);

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("[HTTP] Response Code: %d\n", httpResponseCode);
    Serial.println("[HTTP] Server Response: " + response);
  } else {
    Serial.printf("[HTTP] POST Failed! Error Code: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end(); // Bebaskan resource HTTP Client
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  bootMs = millis();

  Serial.println("\n=========================================");
  Serial.println("    SUGENG NODE — DIRECT HTTP POST       ");
  Serial.println("=========================================");

  connectToWiFi();
}

// ---------- MAIN LOOP ----------
void loop() {
  // Pastikan Wi-Fi tetap terhubung
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  bacaSensor();
  
  // Cetak Log ke Serial Monitor
  Serial.println("-----------------------------------------");
  Serial.printf("DHT22 -> %.1f C | %.1f %%\n", lastTemp, lastHum);
  Serial.printf("JSN-SR04T -> %.1f cm | Vol %.1f %%\n", lastDist, lastVol);
  Serial.printf("MQ135 ADC %d -> %.1f ppm | MQ4 ADC %d -> %.1f ppm\n", 
    lastMq135Adc, mq135PctToPpm(lastMq135Pct), lastMq4Adc, mq4PctToPpm(lastMq4Pct));

  // Kirim data langsung ke Backend Serverless
  kirimDataToBackend();

  // Kirim data setiap 5 detik (untuk pengujian) / Ubah sesuai kebutuhan
  delay(5000);
}
