#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

RTC_DS3231 rtc; // Cambio de RTC_DS1307 a RTC_DS3231
LiquidCrystal lcd(12, 11, 6, 5, 4, 3); // Configura los pines del display LCD según tu conexión

#define CLOCK_INTERRUPT_PIN 2

void onAlarm() {

}

void setup() {
  Serial.begin(9600);
  
  lcd.begin(16, 2); // Configura las dimensiones del display LCD (columnas x filas)
  lcd.setCursor(0, 0);
  if (!rtc.begin()) {
    lcd.print("RTC ERROR");
    while (1);
  }

  if(rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);
  
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  rtc.writeSqwPinMode(DS3231_OFF);

  rtc.disableAlarm(2);

  // Configurar la fecha y hora de la alarma (por ejemplo: 28 de noviembre de 2023 a las 12:00:00)
  DateTime alarmTime(2023, 11, 28, 10, 40, 0); // Año, mes, día, hora, minuto, segundo

  if(!rtc.setAlarm1(
    alarmTime, //indica cuando se activa  la alarma 
    DS3231_A1_Second //  induca la periodicidad de la alarma, en este caso se activa cada minuto (comparando los segundos)
    )) {
      Serial.println("Error, alarm wasn't set!");
    }else {
      Serial.println("Alarm will happen in 10 seconds!");
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

  // Verificar si la alarma está activa
  if (rtc.alarmFired(1)) {
    lcd.setCursor(0, 0);
    lcd.print("     ALARMA     ");
    lcd.setCursor(0, 1);
    lcd.print("    ACTIVADA    ");
    delay(5000); // Mostrar "ALARMA" por 5 segundos
    lcd.setCursor(0, 0);
    lcd.print("                "); // Borrar el mensaje después de 5 segundos
    lcd.setCursor(0, 1);
    lcd.print("                ");
    rtc.clearAlarm(1); // Limpiar la bandera de alarma
  }
}
