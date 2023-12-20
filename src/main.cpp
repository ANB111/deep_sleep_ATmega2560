#include <Wire.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <RTClib.h>
#include <SdFat.h>
#include <LowPower.h>


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 111);
EthernetServer server(80);
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Ethernet.init(53);
  Ethernet.begin(mac, ip);
  server.begin();

  if (!rtc.begin()) {
    Serial.println("RTC ERROR");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Server is at:");
  Serial.println(Ethernet.localIP());
}

void sendHelloMessage(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Hello Page</title></head>");
  client.println("<body>");
  client.println("<h1>Hello from Arduino!</h1>");
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
  client.println("<head><title>Time Page</title></head>");
  client.println("<body>");
  client.print("<h1>Current time: ");
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
  client.println("<head><title>Temperature Page</title></head>");
  client.println("<body>");
  client.print("<h1>Temperature: ");
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
  client.println("<li><a href=\"/hello\">Hello Page</a></li>");
  client.println("<li><a href=\"/time\">Time Page</a></li>");
  client.println("<li><a href=\"/temperature\">Temperature Page</a></li>");
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
  client.println("<head><title>Update Time Page</title></head>");
  client.println("<body>");
  client.println("<h1>Update Time</h1>");
  client.println("<form action=\"/update-time\" method=\"get\">");
  client.println("Enter new time: <input type=\"datetime-local\" name=\"time\"><br>"); // Utilizamos datetime-local para la entrada de fecha y hora
  client.println("<input type=\"submit\" value=\"Submit\">");
  client.println("</form>");
  client.println("<a href=\"/\">Return</a>");
  client.println("</body>");
  client.println("</html>");
}

void updateTime(String request) {
  String timeString = "";
  int timeStart = request.indexOf("time="); // Buscamos el inicio del parámetro de tiempo
  if (timeStart != -1) {
    timeString = request.substring(timeStart + 5); // Extraemos el valor de tiempo después del "time="
    timeString.replace("+", " "); // Reemplazamos el símbolo "+" con un espacio para que sea legible para la librería RTC
    const char* timeCharArray = timeString.c_str(); // Convertimos el String a un array de caracteres
    int year, month, day, hour, minute, second;
    if (sscanf(timeCharArray, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
      DateTime newTime(year, month, day, hour, minute, second);
      rtc.adjust(newTime);
      Serial.println("RTC updated");
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

        if (request.indexOf("GET /hello") != -1) {
          sendHelloMessage(client);
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
