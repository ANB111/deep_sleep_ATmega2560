#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

RTC_DS3231 rtc;
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

#define CLOCK_INTERRUPT_PIN 2

volatile bool alarmTriggered = false;

void onAlarm() {
  alarmTriggered = true;
}

void setup() {
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  if (!rtc.begin()) {
    lcd.print("RTC ERROR");
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
  } else {
    Serial.println("Alarma configurada correctamente");
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

  lcd.setCursor(0, 0);
  lcd.print(now.year());
  lcd.print("-");
  print2digits(now.month());
  lcd.print("-");
  print2digits(now.day());
  lcd.print(" ");
  lcd.setCursor(0, 1);
  print2digits(now.hour());
  lcd.print(":");
  print2digits(now.minute());
  lcd.print(":");
  print2digits(now.second());

  if (alarmTriggered) {
    // Leer la temperatura
    float temperature = rtc.getTemperature();

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C    ");
    lcd.setCursor(0, 1);
    lcd.print("                ");

    delay(5000);

    lcd.setCursor(0, 0);
    lcd.print("                ");
    rtc.clearAlarm(1);
    alarmTriggered = false;
  }
}
