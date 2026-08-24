#include <DHT.h>

// Pin DHT11 terhubung ke D14 (GPIO 14)
#define DHTPIN 14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  // Baca data suhu & kelembapan
  float kelembapan = dht.readHumidity();
  float suhu = dht.readTemperature();

  // Cek apakah sensor berhasil terbaca
  if (!isnan(kelembapan) && !isnan(suhu)) {
    // Format khusus agar muncul di Serial Plotter (Label:Nilai)
    Serial.print("Suhu:");
    Serial.print(suhu);
    Serial.print(",");
    Serial.print("Kelembapan:");
    Serial.println(kelembapan);
  } else {
    Serial.println("Gagal membaca sensor DHT11!");
  }

  // DHT11 butuh jeda pembacaan sekitar 1-2 detik
  delay(2000); 
}