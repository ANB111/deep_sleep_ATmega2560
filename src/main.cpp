#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <LowPower.h>
#include <SD.h>

RTC_DS3231 rtc;

#define CLOCK_INTERRUPT_PIN 18
#define CHIP_SELECT_PIN 53
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
      delay(100);
    } else {
      //Serial.println("Error al abrir el archivo para escritura");
    }

    rtc.clearAlarm(1);
    alarmTriggered = false;
    digitalWrite(13, LOW);
  }
}
