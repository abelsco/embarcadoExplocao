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

long antAmbienteMillis = 0;
long antWebServiceMillis = 0;
bool DHCP = false;
bool CONSUMIR = false;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetServer server(3000);
String LOG = "";

String toString(const IPAddress &address) {
  return String() + address[0] + "." + address[1] + "." + address[2] + "." +
         address[3];
}

void setLOG(String client, String Header) {
  LOG = "\n\n[CLIENTE]: ";
  LOG.concat(client + "\n{\n");
  LOG.concat(Header);
  LOG.concat("\n[SENSORES]: {");
  LOG.concat("\n-pressao: " + String(atualPre));
  LOG.concat("\n-temperatura: " + String(atualTemp));
  LOG.concat("\n-conceOxi: " + String(atualOxi));
  LOG.concat("\n-fonteIg: " + String(atualIg));
  LOG.concat("\n-umidade: " + String(atualUmi));
  LOG.concat("\n-concePo: " + String(atualPoeira));
  LOG.concat("\n}\n}\n");
}

void gravaMicroSD() { Serial.print(LOG); }

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

void printJsonAmbiente(EthernetClient client) {
  StaticJsonDocument<138> doc;
  doc["pressao"] = atualPre;
  doc["temperatura"] = atualTemp;
  doc["conceOxi"] = atualOxi;
  doc["fonteIg"] = atualIg;
  doc["umidade"] = atualUmi;
  doc["concePo"] = atualPoeira;
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

void resourceWebServer(EthernetClient client, String req) {
  if (req == "GET /api/ambiente/ HTTP/1.1") {
    printJsonAmbiente(client);
  } else if (req == "GET /api/ambiente/? HTTP/1.1") {
    printJsonAmbiente(client);
  } else {
    LOG = "";
  }
}

void webService() {
  CONSUMIR = false;
  String Header = "";
  String req = "";
  String ip = "";
  EthernetClient client = server.available();
  if (client) {
    client.setConnectionTimeout(TIMEOUT);
    Serial.print("[CLIENTE]: ");
    ip = toString(client.remoteIP());
    Serial.println(ip);
    client.connected();
    if (client.available()) {
      Header = client.readString();

      req = Header.substring(0, Header.indexOf('\r'));
      // Serial.println(Header);
      Serial.print("[HTTP-REQUEST]: ");
      Serial.println(req);
      resourceWebServer(client, req);
      CONSUMIR = true;
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    client.flush();
    Serial.println("CONCLUÍDO\n");
    server.flush();
    if (CONSUMIR) {
      setLOG(ip, Header);
      gravaMicroSD();
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10); // Most Arduino shields

  Serial.println("==========================================================="
                 "==========");
  Serial.println("Ethernet WebServer");

  // start the Ethernet connection and the server:
  while (!DHCP) {
    if (Ethernet.begin(mac) == 1) {
      DHCP = true;
    }
    Ethernet.maintain();
  }
  // start the WebServer
  Serial.print("Arduino está no IP: ");
  Serial.println(Ethernet.localIP());
  server.begin();
  Serial.println("==========================================================="
                 "==========\n");
}

void loop() {
  unsigned long currentMillisWebService = millis();
  if (currentMillisWebService - antWebServiceMillis > MILLS_WEBSERVICE) {
    antWebServiceMillis = currentMillisWebService;
    webService();
  }

  unsigned long currentMillisAmbiente = millis();
  if (currentMillisAmbiente - antAmbienteMillis > MILLS_AMBIENTE) {
    antAmbienteMillis = currentMillisAmbiente;
    setAmbienteSimulado();
  }
}
