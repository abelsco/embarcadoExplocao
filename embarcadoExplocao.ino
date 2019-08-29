#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>

#define TIMEOUT 1000
#define MILLS_AMBIENTE 3
#define MILLS_WEBSERVICE 45

/* PINOS (FAIXA-OPERACAO)[INTERVALO]
   A5 = TEMPERATURA (0 - 800)[0 - 1023]
   A4 = UMIDADE (0 - 100)[0-100]
   A3 = PRESSÃO (0 - 20)
   A2 = POEIRA (0 - 100)
   A1 = OXIGÊNIO (0 - 100)
   A0 = IGNICAO (0 - 2)
*/

float atualTemp = 0;
float atualUmi = 0;
float atualPre = 0;
float atualPoeira = 0;
float atualOxi = 0;
float atualIg = 0;

float pressaoV = 1;
float temperaturaV = 1;
float conceoxiV = 1;
float fonteV = 1;
float umidadeV = 1;
float concepoV = 1;

long antAmbienteMillis = 0;
long antWebServiceMillis = 0;
bool DHCP = false;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
// IPAddress ip(192,168,16,177);
EthernetServer server(3000);

void defParametros(String tipoGrao) {
  if (tipoGrao == "Açúcar em pó") {
    pressaoV = 7.7;
    temperaturaV = 370;
    conceoxiV = 21;
    fonteV = 0.03;
    umidadeV = 100;
    concepoV = 45;
  }

  else if (tipoGrao == "Alho desidratado") {
    pressaoV = 4;
    temperaturaV = 360;
    conceoxiV = 21;
    fonteV = 0.24;
    umidadeV = 100;
    concepoV = 10;
  }

  else if (tipoGrao == "Arroz") {
    pressaoV = 3.3;
    temperaturaV = 510;
    conceoxiV = 21;
    fonteV = 0.1;
    umidadeV = 100;
    concepoV = 85;
  }

  else if (tipoGrao == "Casca de arroz") {
    pressaoV = 7.7;
    temperaturaV = 450;
    conceoxiV = 17;
    fonteV = 0.05;
    umidadeV = 100;
    concepoV = 55;
  }

  else if (tipoGrao == "Semente de arroz(casca)") {
    pressaoV = 4.3;
    temperaturaV = 490;
    conceoxiV = 12;
    fonteV = 0.08;
    umidadeV = 100;
    concepoV = 45;
  }

  else if (tipoGrao == "Proteína de soja") {
    pressaoV = 6.9;
    temperaturaV = 540;
    conceoxiV = 12;
    fonteV = 0.06;
    umidadeV = 100;
    concepoV = 40;
  }

  else if (tipoGrao == "Farinha de soja") {
    pressaoV = 6.6;
    temperaturaV = 550;
    conceoxiV = 15;
    fonteV = 0.1;
    umidadeV = 100;
    concepoV = 60;
  }

  else if (tipoGrao == "Trigo bruto") {
    pressaoV = 5;
    temperaturaV = 500;
    conceoxiV = 12;
    fonteV = 0.06;
    umidadeV = 100;
    concepoV = 0.5;
  }

  else if (tipoGrao == "Farinha de trigo") {
    pressaoV = 7;
    temperaturaV = 440;
    conceoxiV = 15;
    fonteV = 0.06;
    umidadeV = 100;
    concepoV = 50;
  }

  else if (tipoGrao == "Trigo Cereal") {
    pressaoV = 9.2;
    temperaturaV = 430;
    conceoxiV = 13;
    fonteV = 0.035;
    umidadeV = 100;
    concepoV = 35;
  }

  else if (tipoGrao == "Palha de trigo") {
    pressaoV = 8.2;
    temperaturaV = 470;
    conceoxiV = 13;
    fonteV = 0.035;
    umidadeV = 100;
    concepoV = 75;
  }

  else if (tipoGrao == "Polvilho de trigo") {
    pressaoV = 7;
    temperaturaV = 430;
    conceoxiV = 12;
    fonteV = 0.025;
    umidadeV = 100;
    concepoV = 45;
  }

  else if (tipoGrao == "Milho") {
    pressaoV = 8;
    temperaturaV = 400;
    conceoxiV = 13;
    fonteV = 0.04;
    umidadeV = 100;
    concepoV = 55;
  }

  else if (tipoGrao == "Casca de milho cru") {
    pressaoV = 8.7;
    temperaturaV = 410;
    conceoxiV = 17;
    fonteV = 0.04;
    umidadeV = 100;
    concepoV = 40;
  }

  else if (tipoGrao == "Polvilho de milho") {
    pressaoV = 10.2;
    temperaturaV = 390;
    conceoxiV = 11;
    fonteV = 0.03;
    umidadeV = 100;
    concepoV = 40;
  }

  else if (tipoGrao == "Semente de milho") {
    pressaoV = 8.9;
    temperaturaV = 450;
    conceoxiV = 12;
    fonteV = 0.045;
    umidadeV = 100;
    concepoV = 45;
    concepoV = 40;
  }
}
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

void setWebAmbiente(EthernetClient client) {
  client.print("========<br />Temperatura: ");
  client.print(atualTemp, 0);
  client.print("ºC<br />Umidade: ");
  client.print(atualUmi, 0);
  client.print(" %<br />Presão: ");
  client.print(atualPre);
  client.print(" kg/cm²<br />Concentração Pó: ");
  client.print(atualPoeira);
  client.print(" g/m³<br />Concentração Oxigênio: ");
  client.print(atualOxi, 0);
  client.print(" %<br />Energia Ignição: ");
  client.print(atualIg);
  client.println(" J<br />");
}

void printWebAmbiente(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close"); // the connection will be closed after
  // completion of the response
  // client.println("Refresh: 2");  // refresh the page automatically every 5
  // sec
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  // output the value of each analog input pin
  setWebAmbiente(client);
  client.println("</html>");
}

void printJsonAmbiente(EthernetClient client) {
  StaticJsonDocument<260> doc;
  doc["pressao"] = atualPre;
  doc["situPressao"] = atualUmi;
  doc["temperatura"] = atualTemp;
  doc["situTemperatura"] = atualUmi;
  doc["conceOxi"] = atualOxi;
  doc["situConceOxi"] = atualUmi;
  doc["fonteIg"] = atualIg;
  doc["situFonteIg"] = atualUmi;
  doc["umidade"] = atualUmi;
  doc["situUmidade"] = atualUmi;
  doc["concePo"] = atualPoeira;
  doc["situConcePo"] = atualUmi;
  doc["situaSilo"] = atualUmi;
  client.println(F("HTTP/1.0 200 OK"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Accept-Encoding: gzip, deflate"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(measureJsonPretty(doc));
  client.println();
  // serializeJsonPretty(doc, Serial);
  serializeJson(doc, client);
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

void resourceWebServer(EthernetClient client, String req) {
  if (req == "GET /api/ambiente/ HTTP/1.1") {
    printJsonAmbiente(client);
  } else if (req == "GET /api/ambiente/? HTTP/1.1") {
    printJsonAmbiente(client);
  } else {
    printSerialAmbiente();
  }
}

void webService() {
  EthernetClient client = server.available();
  if (client) {
    client.setConnectionTimeout(TIMEOUT); // set the timeout duration for
    // client.connect() and client.stop()
    Serial.println();
    Serial.print("[HOST]: ");
    Serial.println(client.remoteIP());
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    client.connected();
    if (client.available()) {
      String req = client.readStringUntil('\r');
      // String req = client.readString();
      Serial.print("[HTTP-REQUEST]: ");
      Serial.println(req);
      // client.flush();
      // Serial.println(req);
      // Serial.println(req2);
      resourceWebServer(client, req);
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10); // Most Arduino shields

  Serial.println(
      "=====================================================================");
  Serial.println("Ethernet WebServer");

  // start the Ethernet connection and the server:
  while (!DHCP) {
    if (Ethernet.begin(mac) != 0) {
      DHCP = true;
    }
  }
  // start the WebServer
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  server.begin();
  Serial.println("==========================================================="
                 "==========\n");
}

void loop() {
  // webService();
  // put your main code here, to run repeatedly:

  unsigned long currentMillisWebService = millis();
  if (currentMillisWebService - antWebServiceMillis > MILLS_WEBSERVICE) {
    antWebServiceMillis = currentMillisWebService;
    webService();
  }

  unsigned long currentMillisAmbiente = millis();
  if (currentMillisAmbiente - antAmbienteMillis > MILLS_AMBIENTE) {
    antAmbienteMillis = currentMillisAmbiente;
    setAmbienteSimulado();
    // printSerialAmbiente();
  }
}
