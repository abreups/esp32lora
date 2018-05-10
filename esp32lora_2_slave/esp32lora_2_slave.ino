/*

Como configurar o protocolo LoRa no ESP32 LoRa
Autor: Paulo S. Abreu
09-maio-2018

Este programa ...


*/
//biblioteca para comunicação com o display
#include "SSD1306.h"

// Para usar o protocolo LoRa
#include <SPI.h>      // o chipset LoRa usa a interface SPI para comunicação
#include <LoRa.h>     // biblioteca de funções do LoRa

// construtor do objeto do display
// parametros: address,SDA,SCL
SSD1306 tela(0x3c, 4, 15); 

// Portas usadas pelo chipset SX1278 LoRa
// Portas do chipset WIFI_LoRa_32
// GPIO5 — SX1278’s SCK
// GPIO19 — SX1278’s MISO
// GPIO27 — SX1278’s MOSI
// GPIO18 — SX1278’s CS
// GPIO14 — SX1278’s RESET
// GPIO26 — SX1278’s IRQ(Interrupt Request)

// frequencia do radio LoRa do chipset
#define BAND 433E6

// constantes para chipset Wi-Fi LoRa 32
const int csPin = 18;       // LoRa radio chip select
const int resetPin = 14;    // LoRa radio reset
const int irqPin = 26;      // hardware interrupt pin

// variáveis para rotinas LoRa
String outgoing;              // mensagem a ser enviada
byte msgCount = 0;            // contador de mensagens enviadas.
byte localAddress = 0xBB;     // endereço (inventado) deste dispositivo
byte destination = 0xFF;      // endereço (inventado) para onde a mensagem será enviada (FF = broadcast)
long lastSendTime = 0;        // horário do último pacote enviado
int interval = 5000;          // intervalo aproximado entre envio de mensagens


void setup(){
  pinMode(25, OUTPUT);    // LED na placa para indicar envio de pacote LoRa
  Serial.begin(115200);   // para debug apenas

  // Prepara o display OLED
  pinMode(16,OUTPUT);       // GPIO16 como saída
  digitalWrite(16, LOW);    // reseta o OLED
  delay(10);                // aguarda pelo menos 5 ms
  digitalWrite(16, HIGH);   // mantém GPIO16 em 1 durante uso do OLED
  tela.init();              // inicializa o display
  tela.clear();             // limpa a tela do display

  // Inicializa LoRa
  tela.drawString(0, 30, "Inicializando LoRa");
  tela.display();
  // override the default CS, reset, and IRQ pins
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(BAND)) {                          // initializ radio em 433 MHz
    tela.drawString(0, 40, "LoRa falhou. Pânico");
    tela.display();
    while (true);                                   // loop infinito.
  }
  tela.drawString(0, 40, "LoRa inicializado!");
  tela.display();

  // Agora que o LoRa está inicializado,
  // pede para o master a hora ntp.
  //while ( (millis() - lastSendTime > interval)       ) {
    String message = "gettime";
    sendMessage(message);  // esta função está definida mais abaixo
    Serial.println("Sending " + message + "para " + destination);
    lastSendTime = millis();            // guarda horário do último envio de mensagem
    interval = random(2000) + 5000;    // 5 a 7 segundos de novo intervalo
    digitalWrite(25, HIGH); // turn the default LED on GPIO25 (HIGH is the voltage level)
    delay(200); // espera só um pouquinho
    digitalWrite(25, LOW); // turn the LED off by making the voltage LOW
  //}

  
}

void loop() {

/* slave não fica mandando pacotes à toa.
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
  // parsePacket: returns the packet size in bytes or 0 if no packet was received.
  String retorno = onReceive(LoRa.parsePacket());
  if (retorno != "0") {
    Serial.println( "retorno da função: " + retorno );
    Serial.println();
    tela.clear();
    tela.drawString(0, 0, retorno );
    tela.display();



    
  } // fim de 'if (retorno != 0)'

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
    void onReceive(int packetSize) { --> functisenderon to call when a packet is received.
      // ...
    }
*/
String onReceive(int packetSize) {        // 
  if (packetSize == 0) return("0");          // if there's no packet, return

  // read packet header bytes:
  // LoRa.read(): Reads the next byte from the packet.                pacote enviado        pacote recebido
  int recipient = LoRa.read();          // recipient address          destination       --> recipient
  byte sender = LoRa.read();            //  address             localAddress      --> sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID            msgCount          --> incomingMsgId
  byte incomingLength = LoRa.read();    // incoming msg length        outgoing.length() --> incomingLength

  String incoming = "";

  // LoRa.available(): Returns number of bytes available for reading.
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return("-1");                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return("-2");                             // skip rest of function
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
  
  return( incoming );
}

