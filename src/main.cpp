#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>
#include <SD.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <Udp.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Dirección MAC del Arduino
IPAddress ip(192, 168, 0, 111); // Dirección IP del Arduino
unsigned int localPort = 9; // Puerto de escucha para WoL (puerto 9)

EthernetUDP Udp;


void setup() {
  Serial.begin(9600);
  delay(1000);

  Ethernet.init(53); // Inicializar el chip Ethernet con el pin 53 como CS
  Ethernet.begin(mac, ip);
  Serial.println("Conectado a la red");
  Serial.println(Ethernet.localIP());

}

bool receiveMagicPacket() {
  byte buffer[102];
  memset(buffer, 0, sizeof(buffer));

  // Configura el socket UDP
  Udp.begin(localPort);

  // Espera a recibir datos
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    Udp.read(buffer, sizeof(buffer));

    // Verifica si los primeros 6 bytes del paquete son 0xFF seguidos por la dirección MAC repetida 16 veces
    for (int i = 0; i < 6; i++) {
      if (buffer[i] != 0xFF) return false;
    }

    // Verifica si la dirección MAC coincide con la del Arduino
    for (int i = 0; i < 6; i++) {
      if (buffer[6 + i] != mac[i]) return false;
    }

    // Si se cumple, se considera un Magic Packet válido
    return true;
  }

  return false;
}

void loop() {
  

  // Despierta el Arduino si se recibe un Magic Packet
  if (receiveMagicPacket()) {
    Serial.println("Despertado por Magic Packet");
    // Realiza las acciones necesarias
  }
  //Serial.println("Entrando en modo Power Down");
  delay(1000);
  // Entra en modo de bajo consumo
  //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}