#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>
#include <SD.h>
#include <SPI.h>
#include <Ethernet2.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Dirección MAC del Arduino
IPAddress ip(192, 168, 0, 111); // Dirección IP del Arduino
unsigned int localPort = 9; // Puerto de escucha para WoL (puerto 9)
int wakePin = 7; // Pin para controlar el despertar del Arduino

EthernetUDP Udp;

void setup() {
  pinMode(wakePin, OUTPUT);
  digitalWrite(wakePin, LOW); // Inicialmente, mantener el pin en bajo consumo
  Ethernet.init(53); // Inicializar el chip Ethernet con el pin 53 como CS
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  
  Serial.begin(9600);
  Serial.println(Ethernet.localIP());
  Serial.println("Esperando paquete WoL...");
}

void loop() {
  byte buffer[64];
  int packetSize = Udp.parsePacket();
  if (packetSize >= 6) { // Mínimo tamaño del paquete WoL

    Udp.read(buffer, 64);
    
    // Verificar si es un paquete de WoL (primeros 6 bytes)
    if (buffer[0] == 0xFF && buffer[1] == 0xFF && buffer[2] == 0xFF &&
        buffer[3] == 0xFF && buffer[4] == 0xFF && buffer[5] == 0xFF) {
      Serial.println("Paquete WoL recibido. Encendiendo...");
      digitalWrite(wakePin, HIGH); // Activar el pin de despertar
      delay(100); // Mantener activo durante un breve tiempo
      digitalWrite(wakePin, LOW); // Volver a estado de bajo consumo
    }
  }
}