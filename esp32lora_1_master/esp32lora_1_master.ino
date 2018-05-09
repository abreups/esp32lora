/*

Como configurar o protocolo LoRa no ESP32 LoRa
Autor: Paulo S. Abreu
07-maio-2018

Este programa utiliza tudo o que já foi feito 
no esp32ntp e acrescenta os comandos necessários
para se inicializar o protocolo LoRa.

*/

// biblioteca para conexão Wi-Fi
#include <WiFi.h>
//biblioteca para comunicação com o display
#include "SSD1306.h"
// bibliotecas para usar protocolo ntp
#include <NTPClient.h>
#include <WiFiUdp.h>

// Para usar o protocolo LoRa
#include <SPI.h>              // o chipset LoRa usa a interface SPI para comunicação
#include <LoRa.h>

// construtor do objeto do display
// parametros: address,SDA,SCL
// (Documentação da placa WiFi LoRa 32)
SSD1306 tela(0x3c, 4, 15); 

// ssid e senha da rede Wi-Fi à qual
// você pretende se conectar.
const char *ssid     = "psan";
const char *password = "PAPITO2010!@#";

// Portas usadas pelo chipset SX1278 LoRa
// Portas do chipset WIFI_LoRa_32
// GPIO5 — SX1278’s SCK
// GPIO19 — SX1278’s MISO
// GPIO27 — SX1278’s MOSI
// GPIO18 — SX1278’s CS
// GPIO14 — SX1278’s RESET
// GPIO26 — SX1278’s IRQ(Interrupt Request)
#define SS 18
#define RST 14
#define DI0 26 
#define BAND 433E6          // frequencia do radio LoRa do chipset

// constantes para LoRa
const int csPin = SS;       // LoRa radio chip select
const int resetPin = RST;   // LoRa radio reset
const int irqPin = DI0;     // hardware interrupt pin

// variáveis para rotinas LoRa
String outgoing;              // mensagem a ser enviada
byte msgCount = 0;            // contador de mensagens enviadas.
byte localAddress = 0xBB;     // endereço (inventado) deste dispositivo
byte destination = 0xFF;      // endereço (inventado) para onde a mensagem será enviada (FF = broadcast)
long lastSendTime = 0;        // horário do último pacote enviado
int interval = 5000;          // intervalo aproximado entre envio de mensagens



// Cria uma instância UDP para permitir enviar e receber pacotes UDP
WiFiUDP ntpUDP;
int16_t utc = -3; //UTC -3:00 Brazil
// NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
// define servidor ntp do NIC.br
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 60000);

void setup(){
  pinMode(25, OUTPUT); //Send success, LED will bright 1 second
  Serial.begin(115200);   // para debug apenas
  // Preparação do display

  //configura GPIO16 como saida
  pinMode(16,OUTPUT);       // GPIO16 como saída
  digitalWrite(16, LOW);    // reseta o OLED
  delay(10);                // aguarda pelo menos 5 ms
  digitalWrite(16, HIGH);   // mantém GPIO16 em 1 durante uso do OLED
  tela.init();              // inicializa o display
  tela.clear();             // limpa a tela do display

  // Coloca aviso de que vamos tentar nos conectar ao Wi-Fi
  tela.drawString(0, 0, "Conectando ao Wi-Fi");
  tela.display();

  // Conexão ao Wi-Fi
  WiFi.begin(ssid, password);   // inicializa o Wi-Fi

  // aguarda a conexão à rede Wi-Fi
  // Sai do loop se 60 * 500ms = 30s se passarem sem
  // que conexão seja estabelecida.
  int count = 0;
  while ( WiFi.status() != WL_CONNECTED && count < 60) {
    delay ( 500 );
    count++;
    tela.drawString(count, 10, "o");  // prepara o texto.
    tela.display();                   // mostra o texto no display.
  }
  if (count == 60) {
    tela.clear();   // apaga o texto que está no display
    tela.drawString(0, 0, "Falha tentando conectar ao Wi-Fi");
    tela.display(); // mostra o texto preparado no display
  } else {
    tela.clear();   // apaga o texto que está no display
    tela.drawString(0, 0, "Conectado ao Wi-Fi");
    // mostra o ssid da rede wifi
    tela.drawString(0, 10, WiFi.SSID());
    // mostra o endereço IP recebido
    tela.drawString(0, 20, WiFi.localIP().toString());
    tela.display(); // mostra o texto preparado no display
  }

  // ntp
  timeClient.begin();

  // Inicializa LoRa
  tela.drawString(0, 30, "Inicializando LoRa");
  tela.display();
  /*
   *  Veja documentação da biblioteca LoRa.h em 
   *  https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
   */
  // override the default CS, reset, and IRQ pins (optional)
  // Deve ser chamado antes de LoRa.begin()
  LoRa.setPins(csPin, resetPin, irqPin);
  // initializ radio em BAND MHz
  if (!LoRa.begin(BAND)) {
    tela.drawString(0, 40, "LoRa falhou.");
    tela.display();
    while (true);  // loop infinito.
  }
  tela.drawString(0, 40, "LoRa inicializado.");
  tela.display();
}

void loop() {

/*
  timeClient.update();
  int dia_da_semana = ((timeClient.getEpochTime() / 86400L) + 4) %7;
  switch (dia_da_semana) {
   case 0:
     Serial.println("Domingo");
   break;

   case 1:
     Serial.println("Segunda-feira");
   break;

   case 2:
     Serial.println("Terça-feira");
   break;

   case 3:
     Serial.println("Quarta-feira");
   break;

   case 4:
     Serial.println("Quinta-feira");
   break;

   case 5:
     Serial.println("Sexta-feira");
   break;

   case 6:
     Serial.println("Sábado");
   break;
   }
  
  Serial.println(timeClient.getFormattedTime());
  delay(3000);
*/

/*
  // condição para envio da mensagem LoRa
  if (millis() - lastSendTime > interval) {
    String message = "aLoRa!";
    sendMessage(message);  // esta função está definida mais abaixo
    Serial.println("Sending " + message + "para " + destination);
    lastSendTime = millis();            // guarda horário do último envio de mensagem
    interval = random(2000) + 5000;    // 5 a 7 segundos de novo intervalo
    digitalWrite(25, HIGH); // turn the default LED on GPIO25 (HIGH is the voltage level)
    delay(1000); // wait for a second
    digitalWrite(25, LOW); // turn the LED off by making the voltage LOW
  }
*/
  // Se receber um pacote LoRa:
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  // parsePacket: returns the packet size in bytes or 0 if no packet was received.
    
} // fim do void loop()

//------------------------
// Declarações de funções
//------------------------

// função que envia pacote LoRa
void sendMessage(String outgoing) {
    LoRa.beginPacket();                   // start packet
    LoRa.write(destination);              // add destination address (byte)
    LoRa.write(localAddress);             // add sender address (byte)
    LoRa.write(msgCount);                 // add message ID (byte)
    LoRa.write(outgoing.length());        // add payload
    LoRa.print(outgoing);                 // add payload
    LoRa.endPacket();                     // finish packet and send it (Returns 1 on success, 0 on failure.)
    msgCount++;                           // increment message ID
}

/*
    Veja documentação em https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
    void onReceive(int packetSize) { --> function to call when a packet is received.
      // ...
    }
*/
void onReceive(int packetSize) {        // 
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  // LoRa.read(): Reads the next byte from the packet.                pacote enviado        pacote recebido
  int recipient = LoRa.read();          // recipient address          destination       --> recipient
  byte sender = LoRa.read();            // sender address             localAddress      --> sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID            msgCount          --> incomingMsgId
  byte incomingLength = LoRa.read();    // incoming msg length        outgoing.length() --> incomingLength

  String incoming = "";

  // LoRa.available(): Returns number of bytes available for reading.
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

