#include <Wire.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <RTClib.h>
#include <LowPower.h>



byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 111);
EthernetServer server(80);
RTC_DS3231 rtc;

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

  Serial.println("Servidor en:");
  Serial.println(Ethernet.localIP());
}

void sendData(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Descarga de datos</title></head>");
  client.println("<body>");
  client.println("<h1>Descargar Datos del Servidor!</h1>");
  client.println("<a href=\"/\">Return</a>");
  client.println("</body>");
  client.println("</html>");
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


void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("New client");
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\r');
        client.flush();
        Serial.println(request);

        if (request.indexOf("GET /data") != -1) {
          sendData(client);
        } else if (request.indexOf("GET /time") != -1) {
          sendTimeMessage(client);
        } else if (request.indexOf("GET /temperature") != -1) {
          sendTemperatureMessage(client);
        } else if (request.indexOf("GET /update-time") != -1) {
          sendUpdatePage(client);
          updateTime(request);
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
