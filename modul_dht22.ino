#include <DHT.h>

// Pin DHT22 terhubung ke D14 (GPIO 14)
#define DHTPIN 14
#define DHTTYPE DHT22  // Diubah dari DHT11 ke DHT22

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
    Serial.println("Gagal membaca sensor DHT22!");
  }

  // DHT22 butuh jeda pembacaan minimal 2 detik
  delay(2000); 
}
