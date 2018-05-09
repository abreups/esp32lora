# esp32lora - ESP32 com protocolo LoRa

## Autor: Paulo S. Abreu

### Protocolo SPI - Serial Peripheral Interface

Usado na comunicação entre 1 Mestre (Master) e vários Escravos (Slaves).

3 linhas de comunicação são comuns a todos os dispositivos (Mestre e Escravos):
- MISO (Master In Slave Out / Entrada Mestre Saída Escravo) - linha usada pelo Escravo para enviar dados para o Mestre.
- MOSI (Master Out Slave In / Saída Mestre Entrada Escravo) - linha usada pelo Mestre para enviar dados aos Escravos.
- SCLK (Serial Clock / Clock Serial) - Os pulsos de clock usados para sincronizar a transmissão dos dados gerada pelo Mestre.

e 1 linha de comunicação é específica para cada dispositivo (Escravos):
- SS (Slave Select / Seleção de Escravo) - o pino em cada dispositivo que o Mestre pode usar para habilitar e desabilitar cada dispositivo.

![exemplo](https://upload.wikimedia.org/wikipedia/commons/e/ed/SPI_single_slave.svg "1 Mestre com 1 Escravo")

SPI.begin(,,,)
SPI.begin(5,19,27,18);

### Referências:
1. Protocolo SPI: https://www.arduino.cc/en/Reference/SPI
2. Protocolo SPI: https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Mode_Numbers
3. Exemplo de transmissão LoRa: https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaDuplex/LoRaDuplex.ino
4. Exemplo de LoRa com chipset WiFi LoRa 32: https://www.hackster.io/rayburne/lora-and-the-esp32-6ce9ba
5. Pinagem do chipset Wi-Fi LoRa 32: http://www.heltec.cn/download/WIFI_LoRa_32_Diagram.pdf
6. Biblioteca LoRa: https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
7. 
