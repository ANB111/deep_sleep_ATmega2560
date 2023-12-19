#include <Wire.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <RTClib.h>
#include <LowPower.h>
#include <SdFat.h>

// Define los pines para el bus SPI por software
const uint8_t SOFT_MISO_PIN = 9;
const uint8_t SOFT_MOSI_PIN = 8;
const uint8_t SOFT_SCK_PIN = 10;
const uint8_t SD_CS_PIN = 38; // Pin de chip select para la tarjeta SD

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 111);
EthernetServer server(80);

RTC_DS3231 rtc;

#define CLOCK_INTERRUPT_PIN 18

volatile bool alarmTriggered = false;
volatile float temperature;

void onAlarm() {
  alarmTriggered = true;
}

SdFat softSD; // Instancia de SdFat para utilizar el bus SPI por software

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Ethernet.init(53);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.println("server is at ");
  Serial.println(Ethernet.localIP());

  if (!rtc.begin()) {
    Serial.println("RTC ERROR");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
  rtc.clearAlarm(1);
  rtc.writeSqwPinMode(DS3231_OFF);
  rtc.disableAlarm(2);
  DateTime alarmTime(2023, 11, 28, 10, 40, 0);

  if (!rtc.setAlarm1(alarmTime, DS3231_A1_Second)) {
    Serial.println("Error al configurar alarma");
  }

  // Inicialización del bus SPI por software para la tarjeta SD
  if (!softSD.begin(SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_FULL_SPEED))) {
    Serial.println("Error al inicializar la tarjeta SD");
    while (1);
  }

  Serial.println("Inicio correcto");
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("client disconnected");
  }

  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  detachInterrupt(0);
  DateTime now = rtc.now();

  if (alarmTriggered) {
    temperature = rtc.getTemperature();
    File dataFile = softSD.open("data.txt", FILE_WRITE);

    if (dataFile) {
      dataFile.print(now.timestamp());
      dataFile.print(" Temp: ");
      dataFile.print(temperature);
      dataFile.println(" C");
      dataFile.close();
      Serial.println("Datos guardados correctamente");
    } else {
      Serial.println("Error al abrir el archivo para escritura");
    }

    rtc.clearAlarm(1);
    alarmTriggered = false;
  }
}
