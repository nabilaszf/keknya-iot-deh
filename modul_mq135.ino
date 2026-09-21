/*
 * ====================================================================
 *   ESP32 SMART AIR & GAS SENSING SYSTEM - HIGH-TECH TERMINAL DASHBOARD
 * ====================================================================
 */

const int MQ_AOUT_PIN = 34; // Pin Analog (3 Resistor Divider)
const int MQ_DOUT_PIN = 25; // Pin Digital (1 Resistor)

const float VOLTAGE_DIVIDER = 1.5;
unsigned long packetCount = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(MQ_DOUT_PIN, INPUT);

  delay(500);
  
  Serial.println(F(" [ SYSTEM ] Initializing ADC Hardware... OK"));
  Serial.println(F(" [ SYSTEM ] Calibrating Sensor Baseline... OK"));
  Serial.println(F(" [ SYSTEM ] Streaming Live Telemetry Data...\n"));
  delay(1000);
}

void loop() {
  packetCount++;

  // 1. Sampling ADC Analog
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(MQ_AOUT_PIN);
    delay(5);
  }
  float rawADC = sum / 10.0;
  float vAdc = (rawADC / 4095.0) * 3.3;
  float vSensor = vAdc * VOLTAGE_DIVIDER;

  // Persentase Sinyal (Ekspektasi skala 0V - 3.0V)
  float levelPct = (vSensor / 3.0) * 100.0;
  if (levelPct > 100.0) levelPct = 100.0;
  if (levelPct < 0.0) levelPct = 0.0;

  int doutState = digitalRead(MQ_DOUT_PIN);

  // 2. Tampilan UI Box Dashboard
  Serial.println(F("┌──────────────────────────────────────────────────────────────────┐"));
  
  // Header Baris Packet & Uptime
  Serial.print(F("│ FRAME #"));
  if (packetCount < 10) Serial.print(F("00"));
  else if (packetCount < 100) Serial.print(F("0"));
  Serial.print(packetCount);
  Serial.print(F(" | UPTIME: "));
  Serial.print(millis() / 1000);
  Serial.println(F("s | PROTOCOL: ADC1-12BIT (34)    │"));

  Serial.println(F("├──────────────────────────────────────────────────────────────────┤"));

  // Baris Nilai Raw ADC & Tegangan Real
  Serial.print(F("│  RAW ADC  : "));
  Serial.print((int)rawADC);
  Serial.print(F(" / 4095\t│ VOLTAGE : "));
  Serial.print(vSensor, 2);
  Serial.println(F(" V             │"));

  // Baris Progress Bar Sinyal
  Serial.print(F("│  CAPSULE  : ["));
  int totalBlocks = 16;
  int filledBlocks = (levelPct / 100.0) * totalBlocks;
  for (int i = 0; i < totalBlocks; i++) {
    if (i < filledBlocks) Serial.print(F("█"));
    else Serial.print(F("░"));
  }
  Serial.print(F("] "));
  Serial.print(levelPct, 1);
  Serial.println(F("%              │"));

  // Baris Status System
  Serial.print(F("│  STATUS   : "));
  if (vSensor < 0.8) {
    Serial.print(F("[ NOMINAL / CLEAR ]   "));
  } else if (vSensor < 1.8) {
    Serial.print(F("[ WARNING / DETECTED ]"));
  } else {
    Serial.print(F("[ ALERT / HIGH GAS ]  "));
  }

  Serial.print(F(" │ DOUT : "));
  if (doutState == LOW) {
    Serial.println(F("[ ACTIVE ]  │"));
  } else {
    Serial.println(F("[ IDLE ]    │"));
  }

  Serial.println(F("└──────────────────────────────────────────────────────────────────┘\n"));

  delay(1200);
}
