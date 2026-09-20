const int trigPin = 5;   // Pin D5 di ESP32
const int echoPin = 18;  // Pin D18 di ESP32

void setup() {
  // Inisialisasi Serial Monitor dengan speed 115200
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.println("--- Pengujian Sensor JSN-SR04T Siap ---");
}

void loop() {
  // Bersihkan pin TRIG
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Kirim pulsa trigger selama 10 mikrodetik
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Baca durasi pantulan pulsa suara dari pin ECHO (mikrodetik)
  long duration = pulseIn(echoPin, HIGH);

  // Hitung jarak dalam cm (Kecepatan suara = 0.0343 cm/us)
  float distance = (duration * 0.0343) / 2.0;

  // JSN-SR04T memiliki blind zone di bawah ~20 cm
  if (distance >= 20.0 && distance <= 450.0) {
    Serial.print("Jarak: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Di luar jangkauan (Jarak < 20 cm atau > 450 cm)");
  }

  delay(500); // Pembacaan dilakukan setiap 0.5 detik
}
