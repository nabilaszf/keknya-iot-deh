    // =======================================================
    // SUGENG NODE — USB Serial Live (tanpa WiFi / tanpa DB)
    // ESP32 DevKit V1 + DHT22 + JSN-SR04T + MQ-135 + MQ-4
    //
    // Jalur demo: ESP32 colok USB ke laptop, dashboard baca
    // Serial langsung via WebSocket Bridge atau Web Serial API.
    // Tiap 2 detik kirim 1 baris:  JSON:{...}
    // =======================================================
    #include "DHT.h"

    // ---------- PIN ----------
    #define DHTPIN    19
    #define DHTTYPE   DHT22
    #define TRIG_PIN  5
    #define ECHO_PIN  18
    #define MQ135_PIN 34
    #define MQ4_PIN   35

    // ---------- Konstanta bak 120L (ikut proposal) ----------
    #define H_TOTAL_CM 95.0
    #define S_BLIND_CM 20.0

    DHT dht(DHTPIN, DHTTYPE);

    // Cache nilai bagus terakhir (biar grafik dashboard tidak lompat saat sensor error)
    float lastTemp = 0, lastHum = 0, lastDist = -1, lastVol = 0;
    int lastMq135Adc = 0, lastMq4Adc = 0;
    float lastMq135Pct = 0, lastMq4Pct = 0;
    unsigned long bootMs = 0;

    float hitungVolumePct(float jarakCm) {
      if (jarakCm < 0) return lastVol; // out of range -> tahan nilai lama
      float v = (H_TOTAL_CM - jarakCm) / (H_TOTAL_CM - S_BLIND_CM) * 100.0;
      if (v < 0) v = 0;
      if (v > 100) v = 100;
      return v;
    }

    // Mapping demo ADC% -> ppm (skala dashboard: amonia >=30/60, metana >=100/250)
    float mq135PctToPpm(float pct) { return pct * 1.2; }
    float mq4PctToPpm(float pct)   { return pct * 4.0; }

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

    void kirimJson() {
      // SATU BARIS, diawali "JSON:" — ini yang dibaca dashboard & bridge.
      Serial.print("JSON:{\"nodeId\":\"NODE-MGL-05\",");
      Serial.print("\"temperatureCelsius\":"); Serial.print(lastTemp, 1); Serial.print(",");
      Serial.print("\"humidityPercent\":");    Serial.print(lastHum, 1);  Serial.print(",");
      Serial.print("\"measuredDistanceCm\":"); Serial.print(lastDist, 1); Serial.print(",");
      Serial.print("\"currentVolumePercent\":"); Serial.print(lastVol, 1); Serial.print(",");
      Serial.print("\"gasPpmMQ135\":");  Serial.print(mq135PctToPpm(lastMq135Pct), 1); Serial.print(",");
      Serial.print("\"methanePpmMQ4\":"); Serial.print(mq4PctToPpm(lastMq4Pct), 1);     Serial.print(",");
      Serial.print("\"mq135Adc\":"); Serial.print(lastMq135Adc); Serial.print(",");
      Serial.print("\"mq4Adc\":");   Serial.print(lastMq4Adc);   Serial.print(",");
      Serial.print("\"mq135Pct\":"); Serial.print(lastMq135Pct, 1); Serial.print(",");
      Serial.print("\"mq4Pct\":");   Serial.print(lastMq4Pct, 1);   Serial.print(",");
      Serial.print("\"uptimeSec\":"); Serial.print((millis() - bootMs) / 1000);
      Serial.println("}");
    }

    void setup() {
      Serial.begin(115200);
      delay(1000);
      dht.begin();
      pinMode(TRIG_PIN, OUTPUT);
      pinMode(ECHO_PIN, INPUT);
      digitalWrite(TRIG_PIN, LOW);
      bootMs = millis();

      Serial.println("\n=========================================");
      Serial.println("     SUGENG NODE — USB SERIAL LIVE      ");
      Serial.println("=========================================");
      Serial.println("Colok USB ke laptop, buka dashboard >");
      Serial.println("Data Sensor > Hubungkan Alat Realtime.");
      Serial.println("Baris JSON: diawali prefix 'JSON:'");
      Serial.println();
    }

    void loop() {
      bacaSensor();

      // Log serial monitor biasa
      Serial.println("-----------------------------------------");
      if (isnan(lastTemp) || isnan(lastHum)) {
        Serial.println("DHT22 (D19) -> [ERROR]");
	} else {
        Serial.printf("DHT22 -> %.1f C | %.1f %%\n", lastTemp, lastHum);
      }
      if (lastDist < 0) Serial.println("JSN-SR04T -> [OUT OF RANGE]");
      else Serial.printf("JSN-SR04T -> %.1f cm | Vol %.1f %%\n", lastDist, lastVol);
      Serial.printf("MQ135 ADC %d (%.1f %%) -> %.1f ppm | MQ4 ADC %d (%.1f %%) -> %.1f ppm\n",
        lastMq135Adc, lastMq135Pct, mq135PctToPpm(lastMq135Pct),
        lastMq4Adc, lastMq4Pct, mq4PctToPpm(lastMq4Pct));

      // Baris mesin untuk dashboard
      kirimJson();

      delay(2000);
    }
