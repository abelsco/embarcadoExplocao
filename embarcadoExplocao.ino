float atualTemp = 0;
float atualUmi = 0;
float atualPre = 0;
float atualPoeira = 0;
float atualOxi = 0;
float atualIg = 0;

long antAmbienteMillis = 0;
long ambienteMillisInterval = 500;

void printSerialAmbiente() {
  Serial.print("========\nTemperatura: ");
  Serial.print(atualTemp, 0);
  Serial.print("ºC\nUmidade: ");
  Serial.print(atualUmi, 0);
  Serial.print(" %\nPresão: ");
  Serial.print(atualPre);
  Serial.print(" kg/cm²\nConcentração Pó: ");
  Serial.print(atualPoeira);
  Serial.print(" g/m³\nConcentração Oxigênio: ");
  Serial.print(atualOxi, 0);
  Serial.print(" %\nEnergia Ignição: ");
  Serial.print(atualIg);
  Serial.println(" J");
}

void setAmbienteSimulado() {
  atualTemp = analogRead(A5);
  atualUmi = analogRead(A4);
  atualPre = analogRead(A3);
  atualPoeira = analogRead(A2);
  atualOxi = analogRead(A1);
  atualIg = analogRead(A0);

  atualTemp = map(atualTemp, 0, 1023, 0, 800);
  atualUmi = map(atualUmi, 0, 409, 0, 100);
  atualPre = atualPre * 20 / 1023;
  atualPoeira = atualPoeira * 100 / 1023;
  atualOxi = map(atualOxi, 0, 409, 0, 100);
  atualIg = atualIg * 2 / 1023;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  /* PINOS (FAIXA-OPERACAO)[INTERVALO]
     A5 = TEMPERATURA (0 - 800)[0 - 1023]
     A4 = UMIDADE (0 - 100)[0-100]
     A3 = PRESSÃO (0 - 20)
     A2 = POEIRA (0 - 100)
     A1 = OXIGÊNIO (0 - 100)
     A0 = IGNICAO (0 - 2)
  */
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  if (currentMillis - antAmbienteMillis > ambienteMillisInterval) {
    antAmbienteMillis = currentMillis;
    setAmbienteSimulado();
    printSerialAmbiente();
  }
}
