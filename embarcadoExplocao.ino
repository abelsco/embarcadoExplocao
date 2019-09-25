#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>

#define TIMEOUT 1000
#define MILLS_AMBIENTE 30
#define MILLS_WEBSERVICE 1000
#define CODSERIE                                                               \
  "3e23e8160039594a33894f6564e1b1348bbd7a0088d42c4acb73eeaed59c009d" // Identificação
                                                                     // do
                                                                     // Arduino(SHA256)

/* PINOS (FAIXA-OPERACAO)[INTERVALO]
   A5 = TEMPERATURA (0 - 800)[0 - 1023]
   A4 = UMIDADE (0 - 100)[0-100]
   A3 = PRESSÃO (0 - 20)
   A2 = POEIRA (0 - 100)
   A1 = OXIGÊNIO (0 - 100)
   A0 = IGNICAO (0 - 2)
*/

float atualTemp = 0;   // Temperatura atual
float atualUmi = 0;    // Umidade atual
float atualPre = 0;    // Pressão atual
float atualPoeira = 0; // Concentração de poeira atual
float atualOxi = 0;    // Concentração de oxigênio atual
float atualIg = 0;     // Concentração da energia de ignição

long antAmbienteMillis = 0;  // Timer corrente do ambiente
long antWebClientMillis = 0; // Timer corrente do WebCliente
bool DHCP = false;           // Controla a aquisição do DHCP

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Endereço físico do Arduino
// Para o servidor, utilizar uma das duas definições abaixo
IPAddress servidor(192, 168, 16, 4); // IP da API
// char servidor[] = "www.meu-servidor.com";    // Endereço DNS da API
EthernetClient client; // Cliente HTTP

void setAmbienteSimulado() {
  // Atualiza os valores dos sensores
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

void printJsonAmbiente() {
  // Envia os dados para a API consumir
  StaticJsonDocument<250> doc;
  doc["pressao"] = atualPre;
  doc["temperatura"] = atualTemp;
  doc["conceOxi"] = atualOxi;
  doc["fonteIg"] = atualIg;
  doc["umidade"] = atualUmi;
  doc["concePo"] = atualPoeira;
  doc["codSerie"] = CODSERIE;
  client.println(F("POST /api/ambiente HTTP/1.1"));
  client.println(F("Host: 192.168.16.4:5001"));
  client.println(F("Content-Type: application/json"));
  client.println(F("User-Agent: arduino-ethernet"));
  client.println(F("Accept: */*"));
  client.println(F("Accept-Encoding: gzip, deflate"));
  client.println(F("Connection: keep-alive"));
  client.print(F("Content-Length: "));
  client.println(measureJsonPretty(doc));
  client.println();
  delay(1);
  serializeJsonPretty(doc, client);
}

void printResposta() {
  StaticJsonDocument<30> doc;
  if (client.available()) {
    // char c = client.read();
    // Serial.print(c);
    deserializeJson(doc, client);
    serializeJsonPretty(doc, Serial);
  }
}

void webClient() {
  // Implementa o WebCliente verificando se consegue se conectar no servidor
  client.stop();
  if (client.connect(servidor, 5001)) {
    printJsonAmbiente();
    // Espera um tempo
    delay(1);
  }
}

void setup() {
  // Inicia a Serial
  Serial.begin(57600);
  Serial.println("===========================================================");
  Serial.println("WebClient Arduino");
  Serial.println("Iniciando DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Não foi possível registrar DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Shield de rede não encontrado");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
  }

  Serial.print("IP registrado: ");
  Serial.println(Ethernet.localIP());
  Serial.println(
      "===========================================================\n");
  // Delay para o Shield iniciar:
  delay(1000);
}

void loop() {
  // Caso queira visualizar a resposta
  // printResposta();
  unsigned long currentMillisWebClient = millis();
  if (currentMillisWebClient - antWebClientMillis > MILLS_WEBSERVICE) {
    antWebClientMillis = currentMillisWebClient;
    webClient();
  }

  unsigned long currentMillisAmbiente = millis();
  if (currentMillisAmbiente - antAmbienteMillis > MILLS_AMBIENTE) {
    antAmbienteMillis = currentMillisAmbiente;
    setAmbienteSimulado();
  }
}
