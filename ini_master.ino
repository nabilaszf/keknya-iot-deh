#include "DHT.h"

// =======================================================
// PIN DEFINITION (ESP32)
// =======================================================
#define DHTPIN    19
#define DHTTYPE   DHT22

#define TRIG_PIN  5
#define ECHO_PIN  18

#define MQ135_PIN 34
#define MQ4_PIN   35

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inisialisasi DHT22
  dht.begin();

  // Inisialisasi JSN-SR04T
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.println("\n=========================================");
  Serial.println("     TESTING ALL SENSORS (MULTI-SENSOR)  ");
  Serial.println("=========================================\n");
}

void loop() {
  // 1. Baca Sensor DHT22 (D19)
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // 2. Baca Sensor JSN-SR04T (D5/D18)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms (~5m)
  float distance = (duration > 0) ? (duration * 0.0343 / 2.0) : -1.0;

  // 3. Baca Sensor MQ-135 (D34)
  int mq135_adc = analogRead(MQ135_PIN);
  float mq135_volt = (mq135_adc / 4095.0) * 3.3;
  float mq135_pct = (mq135_adc / 4095.0) * 100.0;

  // 4. Baca Sensor MQ-4 (D35)
  int mq4_adc = analogRead(MQ4_PIN);
  float mq4_volt = (mq4_adc / 4095.0) * 3.3;
  float mq4_pct = (mq4_adc / 4095.0) * 100.0;

  // ==========================================
  // Output Serial Monitor
  // ==========================================
  Serial.println("-----------------------------------------");

  // Output DHT22
  if (isnan(humidity) || isnan(temp)) {
    Serial.println("DHT22 (D19)       -> [ERROR / UNCONNECTED]");
  } else {
    Serial.print("DHT22 (D19)       -> Suhu: ");
    Serial.print(temp, 1);
    Serial.print(" °C | Kelembapan: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
  }

  // Output JSN-SR04T
  if (distance < 0) {
    Serial.println("JSN-SR04T (D5/D18)-> [OUT OF RANGE / NO ECHO]");
  } else {
    Serial.print("JSN-SR04T (D5/D18)-> Jarak: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }

  // Output MQ-135
  Serial.print("MQ-135 (D34)      -> ADC: ");
  Serial.print(mq135_adc);
  Serial.print(" | Tegangan: ");
  Serial.print(mq135_volt, 2);
  Serial.print(" V | Level: ");
  Serial.print(mq135_pct, 1);
  Serial.println("%");

  // Output MQ-4
  Serial.print("MQ-4 (D35)        -> ADC: ");
  Serial.print(mq4_adc);
  Serial.print(" | Tegangan: ");
  Serial.print(mq4_volt, 2);
  Serial.print(" V | Level: ");
  Serial.print(mq4_pct, 1);
  Serial.println("%");

  delay(2000); // Interval pembacaan 2 detik (ideal buat DHT22)
}
