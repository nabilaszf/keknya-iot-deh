const int LED = 2;
const int LEDHIJAU = 4;
void setup() {
  pinMode(LED, OUTPUT);
  pinMode(LEDHIJAU, OUTPUT);
}

void loop() {
  digitalWrite(LED,HIGH);
  delay(1000);
  digitalWrite(LED,LOW);
  delay(500);
  
  digitalWrite(LEDHIJAU,HIGH);
  delay(500);
  digitalWrite(LEDHIJAU,LOW);
  delay(1000);
}

