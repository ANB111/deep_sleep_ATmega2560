#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define LED_PIN PB7 // Pin 13 en Arduino Mega 2560 (PB7 en el ATmega2560)
#define INTERRUPT_PIN 2

void setup() {
    // Configurar el pin de interrupción como entrada
    DDRD &= ~(1 << INTERRUPT_PIN);
    // Habilitar resistencia de pull-up en el pin de interrupción
    PORTD |= (1 << INTERRUPT_PIN);

    // Configurar la interrupción externa para el flanco descendente (LOW LEVEL)
    EICRA |= (1 << ISC01); // ISC01=1, ISC00=0 para flanco descendente
    EIMSK |= (1 << INT0);  // Habilitar la interrupción externa 0

    // Deshabilitar el módulo ADC
    ADCSRA &= ~(1 << ADEN);

    // Deshabilitar el comparador analógico
    ACSR |= (1 << ACD);
}

void sleep() {
    // Configurar el microcontrolador para el modo de bajo consumo (Power-Down mode)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    // Habilitar interrupciones
    sei();

    // Entrar en modo de bajo consumo
    sleep_cpu();

    // Después de despertar, deshabilitar el modo de bajo consumo
    sleep_disable();
}

int main(void) {
    setup();
    // Configurar el pin del LED como salida
    DDRB |= (1 << LED_PIN);
    while (1) {
        sleep();
        // El código se ejecuta después de despertar va en ésta sección
        // Hacer que el LED parpadee durante la ejecución del código
        for (int i = 0; i < 25; ++i) {
            PORTB |= (1 << LED_PIN); // Encender el LED
            _delay_ms(15);          // Esperar 500 milisegundos (0.5 segundos)
            PORTB &= ~(1 << LED_PIN); // Apagar el LED
            _delay_ms(15);           // Esperar 500 milisegundos (0.5 segundos)
        }
    }

    return 0;
}

// Rutina de servicio de interrupción externa
ISR(INT0_vect) {
    // No se realiza ninguna acción, ya que su único propósito es despertar al microcontrolador
}




//codigo 2

/*
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

RTC_DS1307 rtc;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Configura los pines del display LCD según tu conexión

void setup() {
  Serial.begin(9600);
  
  lcd.begin(16, 2); // Configura las dimensiones del display LCD (columnas x filas)
  lcd.setCursor(0, 0);
  if (!rtc.begin()) {
    lcd.print("RTC ERROR");
    while (1);
  }

  if (!rtc.isrunning()) { //configurar fecha y hora si el rtc no está en funcionamiento
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

void print2digits(int number) {
  if (number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}

void loop() {
  DateTime now = rtc.now();

  // Mostrar la fecha en el display
  lcd.setCursor(0, 0);
  lcd.print(now.year());
  lcd.print("-");
  print2digits(now.month());
  lcd.print("-");
  print2digits(now.day());
  lcd.print(" ");
  lcd.setCursor(0, 1);
  // Mostrar la hora en el display
  print2digits(now.hour());
  lcd.print(":");
  print2digits(now.minute());
  lcd.print(":");
  print2digits(now.second());

  delay(1000);
  
}
*/

//CODIGO DE ARDUINO CMO SERVIDOR


#include <Wire.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <Adafruit_I2CDevice.h>



// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(169, 254, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  //Ethernet.init(53);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  Serial.println (Ethernet.subnetMask());
  Serial.println (Ethernet.gatewayIP ());
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
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
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}




#include <Wire.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <Adafruit_I2CDevice.h>



// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 111);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Ethernet.init(53);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
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
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>
#include <SD.h>

RTC_DS3231 rtc;

#define CLOCK_INTERRUPT_PIN 18
#define CHIP_SELECT_PIN 40
#define LED_PIN 13

volatile bool alarmTriggered = false;
volatile float temperature;

void onAlarm() {
  alarmTriggered = true;
}

void setup() {
  Serial.begin(9600);
  
  if (!rtc.begin()) {
    Serial.print("RTC ERROR");
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
    Serial.print("Error al configurar alarma");
  } else {
    //Serial.print("Alarma configurada correctamente");
  }

  if (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println("Error al iniciar la tarjeta SD");
    while (1);
  }
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Inicio correcto");
  delay(300);

}


void loop() {
  

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  detachInterrupt(0);
  digitalWrite(13, HIGH);

  DateTime now = rtc.now();


  if (alarmTriggered) {
    // Leer la temperatura
    temperature = rtc.getTemperature();

    File dataFile = SD.open("data.txt", FILE_WRITE);

    if (dataFile) {
      dataFile.print(now.timestamp());
      dataFile.print(" Temp: ");
      dataFile.print(temperature);
      dataFile.println(" C");
      dataFile.close();
      Serial.println("Datos guardados correctamente");
      delay(300);
    } else {
      Serial.println("Error al abrir el archivo para escritura");
    }

    rtc.clearAlarm(1);
    alarmTriggered = false;
    digitalWrite(13, LOW);
  }
}
