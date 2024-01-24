#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>
#include <SD.h>
#include <SPI.h>
#include <Ethernet2.h>

// Variables globales y configuraciones
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 101);
EthernetServer server(80);
RTC_DS3231 rtc;

#define CLOCK_INTERRUPT_PIN 18
#define SERVER_INTERRUPT_PIN 19
#define CHIP_SELECT_PIN 40

volatile bool alarmTriggered = false;
volatile bool pulsadorServidor = false;

void onAlarm() {
  alarmTriggered = true;
}
void onPulsador() {
  pulsadorServidor = true;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Ethernet.init(53);
  Ethernet.begin(mac);
  server.begin();

  if (!rtc.begin()) {
    Serial.println("RTC ERROR");
    while (1);
  }
  
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(SERVER_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(SERVER_INTERRUPT_PIN), onPulsador, FALLING);
  
  rtc.clearAlarm(1);
  rtc.writeSqwPinMode(DS3231_OFF);
  rtc.disableAlarm(2);

  DateTime alarmTime = rtc.now();
  alarmTime = alarmTime + TimeSpan(0, 0, 1, 0);


  if (!rtc.setAlarm1(alarmTime, DS3231_A1_PerSecond)) {
    Serial.print("Error al configurar alarma");
  }

  if (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println("Error al iniciar la tarjeta SD");
    while (1);
  }

  Serial.println("Servidor en:");
  Serial.println(Ethernet.localIP());
}



void sendTimeMessage(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Fecha y Hora</title></head>");
  client.println("<body>");
  client.print("<h1>Fecha y hora actual: ");
  client.print(rtc.now().timestamp());
  client.println("</h1>");
  client.println("<a href=\"/\">Return</a>");
  client.println("</body>");
  client.println("</html>");
}

void sendTemperatureMessage(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Temperatura</title></head>");
  client.println("<body>");
  client.print("<h1>Temperatura: ");
  client.print(rtc.getTemperature());
  client.println(" °C</h1>");
  client.println("<a href=\"/\">Return</a>");
  client.println("</body>");
  client.println("</html>");
}

void sendDefaultMessage(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>INICIO</title></head>");
  client.println("<body>");
  client.println("<h1>Bienvenido al servidor Arduino</h1>");
  client.println("<h2>Paginas Disponibles:</h2>");
  client.println("<ul>");
  client.println("<li><a href=\"/data\">Descargar Datos</a></li>");
  client.println("<li><a href=\"/time\">Hora del Arduino</a></li>");
  client.println("<li><a href=\"/temperature\">Temperatura Page</a></li>");
  client.println("<li><a href=\"/update-time\">Actualizar Hora</a></li>");
  client.println("</ul>");

  // Agregar el formulario con el botón para salir
  client.println("<form action=\"/exit\" method=\"post\">");
  client.println("<input type=\"submit\" value=\"Salir\">");
  client.println("</form>");

  client.println("</body>");
  client.println("</html>");
}


void sendUpdatePage(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Actualizar Fecha y Hora</title></head>");
  client.println("<body>");
  client.println("<h1>Actualizar Fecha y Hora</h1>");
  client.println("<form action=\"/update-time\" method=\"get\">");
  client.println("Ingrese la nueva fehca y hora: <input type=\"datetime-local\" name=\"time\"><br>"); // Utilizamos datetime-local para la entrada de fecha y hora
  client.println("<input type=\"submit\" value=\"Actualizar\">");
  client.println("</form>");
  client.println("<a href=\"/\">Return</a>");
  client.println("</body>");
  client.println("</html>");
}

void updateTime(String request) {
  String timeString = "";

  int timeStart = request.indexOf("time="); // Buscar el inicio del parámetro de tiempo
  if (timeStart != -1) {
    timeString = request.substring(timeStart + 5); // Extraer el valor de tiempo después de "time="TC
    timeString.replace("%3A", ":");// Reemplazar los carcateres "%3A" por ":"

    const char* timeCharArray = timeString.c_str(); // Convertir el String a un array de caracteres
    int year, month, day, hour, minute;

    if (sscanf(timeCharArray, "%d-%d-%dT%d:%d", &year, &month, &day, &hour, &minute) == 5) {
      DateTime newTime(year, month, day, hour, minute, 0);
      rtc.adjust(newTime);
      Serial.println("RTC actualizado");
    }
  }
}

void acquireAndSaveData() {
  //adquirir datos y guardarlos en la memoria sd

  float temperature = rtc.getTemperature();

  Serial.print(rtc.now().timestamp());
  Serial.print(" Temp: ");
  Serial.print(temperature);
  Serial.println(" C");

  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(rtc.now().timestamp());
    dataFile.print(" Temp: ");
    dataFile.print(temperature);
    dataFile.println(" C");
    dataFile.close();
  } else {
      Serial.println("Error al abrir el archivo para escritura");
  }

}



void sendDownloadPage(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Archivos Disponibles</title></head>");
  client.println("<body>");
  client.println("<h1>Archivos Disponibles:</h1>");

  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }

    if (entry.isDirectory()) {
      client.print("<p>Directorio: ");
    } else {
      client.print("<p>");
      client.print(entry.name());
      client.print(" <a href='/download/");
      client.print(entry.name());
      client.print("'>Descargar</a>");
      client.print(" <form action='/delete' method='post' style='display:inline;'>");
      client.print("<input type='hidden' name='filename' value='");
      client.print(entry.name());
      client.print("'>");
      client.print("<input type='submit' value='Eliminar'>");
      client.print("</form>");
      client.println("</p>");
    }
    entry.close();
  }
  root.close();

  client.println("<a href=\"/\">Return</a>");
  client.println("</body>");
  client.println("</html>");

  client.stop();
}

void handleDeleteRequest(EthernetClient client, String request) {
  int filenameStart = request.indexOf("filename=") + 9;
  int filenameEnd = request.indexOf(" HTTP/1.1");
  String filenameToDelete = request.substring(filenameStart, filenameEnd);

  if (SD.remove(filenameToDelete.c_str())) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<h1>Archivo eliminado: " + filenameToDelete + "</h1>");
  } else {
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>Error al eliminar el archivo</h1>");
  }

  client.stop();
}


void handleDownloadRequest(EthernetClient client, String request) {
  int filenameStart = request.indexOf("/download/") + 10;
  int filenameEnd = request.indexOf(" HTTP/1.1");
  String filename = request.substring(filenameStart, filenameEnd);

  File downloadFile = SD.open(filename);
  if (downloadFile) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/octet-stream");
    client.println("Content-Disposition: attachment; filename=" + filename);
    client.println("Connection: close");
    client.println();

    // Enviar el contenido del archivo
    while (downloadFile.available()) {
      char data = downloadFile.read();
      client.write(data);
    }

    // Cerrar el archivo
    downloadFile.close();
  } else {
    // Si no se puede abrir el archivo, enviar una respuesta de error
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>Error 404: Archivo no encontrado</h1>");
  }

  client.stop();
}

void enableServer() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("New client");
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\r');
        client.flush();
        Serial.println(request);

        if (request.indexOf("POST /exit") != -1) {
          // Modificar la variable para salir del bucle
          pulsadorServidor = false;
          break;
        } else if (request.indexOf("GET /data") != -1) {
          sendDownloadPage(client);
        } else if (request.indexOf("GET /time") != -1) {
          sendTimeMessage(client);
        } else if (request.indexOf("GET /temperature") != -1) {
          sendTemperatureMessage(client);
        } else if (request.indexOf("GET /update-time") != -1) {
          sendUpdatePage(client);
          updateTime(request);
        } else if (request.indexOf("GET /download/") != -1) {
          handleDownloadRequest(client, request);
        } else if (request.indexOf("POST /delete") != -1) {
          handleDeleteRequest(client, request);
        } else {
          sendDefaultMessage(client);
        }
        break;
      }
    }
    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}


void loop() {
  detachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN));  // Deshabilitar la interrupción del pin CLOCK_INTERRUPT_PIN

  if (alarmTriggered) {
    acquireAndSaveData();
    
    alarmTriggered = false;
  }

  while (pulsadorServidor) {
    enableServer(); // Habilitar el servidor
  }

  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING); 
  rtc.clearAlarm(1);
  // Dormir hasta que ocurra una interrupción
  Serial.println("A dormir...");
  delay(300);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

