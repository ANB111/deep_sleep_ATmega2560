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
