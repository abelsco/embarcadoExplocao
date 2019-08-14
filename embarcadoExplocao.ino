#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>

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

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 15, 77);
//IPAddress ip(127, 0, 0, 1);
EthernetServer server(80);

void printSerialAmbiente()
{
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

void setWebAmbiente(EthernetClient client)
{
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

void printWebAmbiente(EthernetClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close"); // the connection will be closed after completion of the response
  //client.println("Refresh: 2");  // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  // output the value of each analog input pin
  setWebAmbiente(client);
  client.println("</html>");
}

void setJsonAmbiente(EthernetClient client, StaticJsonDocument<500> doc)
{
  JsonArray pressao = doc.createNestedArray("pressao");
  pressao.add(atualPre);
  JsonArray temperatura = doc.createNestedArray("temperatura");
  temperatura.add(atualTemp);
  JsonArray conceOxi = doc.createNestedArray("conceOxi");
  conceOxi.add(atualOxi);
  JsonArray fonteIg = doc.createNestedArray("fonteIg");
  fonteIg.add(atualIg);
  JsonArray umidade = doc.createNestedArray("umidade");
  umidade.add(atualUmi);
  JsonArray concePo = doc.createNestedArray("concePo");
  concePo.add(atualPoeira);
  serializeJson(doc, Serial);
  serializeJsonPretty(doc, client);
}

void printJsonAmbiente(EthernetClient client)
{
  StaticJsonDocument<500> doc;
  client.println(F("HTTP/1.0 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(measureJsonPretty(doc));
  setJsonAmbiente(client, doc);
  client.println();
}

void setAmbienteSimulado()
{
  atualTemp = analogRead(A0);
  atualUmi = analogRead(A1);
  atualPre = analogRead(A2);
  atualPoeira = analogRead(A3);
  atualOxi = analogRead(A4);
  atualIg = analogRead(A5);

  atualTemp = map(atualTemp, 0, 1023, 0, 800);
  atualUmi = map(atualUmi, 0, 409, 0, 100);
  atualPre = atualPre * 20 / 1023;
  atualPoeira = atualPoeira * 100 / 1023;
  atualOxi = map(atualOxi, 0, 409, 0, 100);
  atualIg = atualIg * 2 / 1023;
}

void webService()
{
  EthernetClient client = server.available();
  if (client)
  {
    client.setConnectionTimeout(TIMEOUT); // set the timeout duration for client.connect() and client.stop()
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          // printWebAmbiente(client);
          printJsonAmbiente(client);
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(3);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(250000);
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields

  while (!Serial)
    continue;
  Serial.println("=====================================================================");
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
      continue;
  }
  else if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }
  else if (Ethernet.hardwareStatus() == EthernetW5100)
  {
    Serial.println("W5100 Ethernet controller detected.");
    // start the server
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
    Serial.println("=====================================================================\n");
  }
}

void loop()
{
  // webService();
  // put your main code here, to run repeatedly:
  // unsigned long currentMillisWebService = millis();
  // if (currentMillisWebService - antWebServiceMillis > MILLS_WEBSERVICE)
  // {
  //   antWebServiceMillis = currentMillisWebService;
    webService();
  // }

  unsigned long currentMillisAmbiente = millis();
  if (currentMillisAmbiente - antAmbienteMillis > MILLS_AMBIENTE)
  {
    antAmbienteMillis = currentMillisAmbiente;
    setAmbienteSimulado();
    // printSerialAmbiente();
  }
  //
  //  unsigned long currentMillisAmbiente = millis();
  //  if (currentMillisAmbiente - antAmbienteMillis > MILLS_AMBIENTE)
  //  {
  //    antAmbienteMillis = currentMillisAmbiente;
  //    setAmbienteSimulado();
  //    // printSerialAmbiente();
  //
  //    webService();
  //  }
}
