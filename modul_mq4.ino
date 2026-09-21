const int MQ4_PIN = 34;                     // Pin Analog GPIO 34
const float VOLTAGE_DIVIDER_FACTOR = 1.5;   // Rangkain 3 resistor (5V -> 3.3V)
const float RL_VALUE = 10.0;                // Resistor beban modul MQ-4 (10 kOhm)

// Variabel Kalibrasi
float R0 = 10.0; // Nilai R0 default (akan diperbarui otomatis saat startup)

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  Serial.println("\n================================================");
  Serial.println("  SISTEM DETEKSI METANA MQ-4 (AUTO-KALIBRASI)   ");
  Serial.println("================================================");
  
  // Minta waktu warming-up sensor
  Serial.println("[1/2] Memanaskan heater sensor... (Tunggu 5 detik)");
  for(int i = 5; i > 0; i--) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();

  // Jalankan Auto Kalibrasi R0 di Udara Bersih Ruangan
  R0 = hitungAutoR0();
  
  Serial.println("[2/2] Kalibrasi Selesai!");
  Serial.print(">>> Nilai R0 Baseline Ruangan: ");
  Serial.print(R0, 2);
  Serial.println(" kOhm <<<\n");
  
  Serial.println("--- MULAI MONITORING GAS METANA ---");
}

void loop() {
  // 1. Baca Tegangan Sensor dengan Filtering (20 Sampel)
  long adcSum = 0;
  for (int i = 0; i < 20; i++) {
    adcSum += analogRead(MQ4_PIN);
    delay(10);
  }
  float adcRaw = adcSum / 20.0;
  float vAdc = (adcRaw / 4095.0) * 3.3;
  float vSensor = vAdc * VOLTAGE_DIVIDER_FACTOR;

  // Batasi agar tidak terjadi math error
  if (vSensor < 0.05) vSensor = 0.05;
  if (vSensor > 4.95) vSensor = 4.95;

  // 2. Hitung RS (Resistansi Sensor Saat Ini)
  float RS = ((5.0 - vSensor) / vSensor) * RL_VALUE;

  // 3. Hitung Kadar Gas Metana dalam PPM
  float ratio = RS / R0;
  float ppmMethane = 1000.0 * pow(ratio, -2.83); // Rumus logaritmik datasheet MQ-4

  // 4. Output Log & Evaluasi Ambang Batas Standar Ruangan
  Serial.print("V_Sensor: ");
  Serial.print(vSensor, 2);
  Serial.print(" V | Metana: ");
  Serial.print(ppmMethane, 0);
  Serial.print(" PPM | Status: ");

  // Kriteria Standar Deteksi Metana
  if (ppmMethane < 200.0) {
    Serial.println("[AMBALAN NORMAL] Udara bersih, tidak ada metana.");
  } else if (ppmMethane >= 200.0 && ppmMethane <= 1000.0) {
    Serial.println("[TERDETEKSI] Ada jejak gas metana di ruangan!");
  } else {
    Serial.println("[BAHAYA KEBOCORAN] Kadar gas metana tinggi!");
  }

  delay(1000);
}

// Fungsi Otomatis Mencari R0 Ruangan Saat Booting
float hitungAutoR0() {
  Serial.println("Mengukur baseline udara ruangan...");
  float rsSum = 0;
  for (int i = 0; i < 50; i++) {
    float adcRaw = analogRead(MQ4_PIN);
    float vAdc = (adcRaw / 4095.0) * 3.3;
    float vSensor = vAdc * VOLTAGE_DIVIDER_FACTOR;
    if (vSensor < 0.05) vSensor = 0.05;
    
    float rs = ((5.0 - vSensor) / vSensor) * RL_VALUE;
    rsSum += rs;
    delay(100);
  }
  float rsAir = rsSum / 50.0;
  
  // Menurut Datasheet MQ-4: RS/R0 di udara bersih = 4.4
  return (rsAir / 4.4); 
}
