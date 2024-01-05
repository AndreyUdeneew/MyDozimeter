// Простой дозиметр на Ардуино с датчиком температуры и влажности. Версия 1.0, 2022 г.
// (с) Волков Дмитрий dimavolk@mail.ru

// #include "GyverPWM.h"
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <SPI.h>              // Arduino SPI library#include <Adafruit_GFX.h>    // Core graphics library

volatile int counter = 0;             // переменная-счётчик импульсов
int doza = 0;                         // переменная значение фона радиации
uint32_t sec = millis() / 1000ul;     // полное количество секунд
uint32_t ch_sec = millis() / 1000ul;  // полное количество секунд
boolean event = LOW;
float voltage = 0;

#define TFT_CS 9    // define chip select pin
#define TFT_DC 8    // define data/command pin
#define TFT_RST 12  // define reset pin, or set to -1 and connect to Arduino RESET pin
#define TFT_MOSI 11
#define TFT_SCLK 10
#define TFT_BL 25
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define NORMA 67      // количество импульсов в минуту при нормальном фоне, для СТС-5 = 27, для СБМ-20 = 67, для других счетчиков см.паспорт
#define FON_NORMA 15  // нормальный радиационный фон 15мкР/ч

void setup() {
  tft.setSPISpeed(40000000);
  tft.initR(INITR_MINI160x80);
  tft.setRotation(3);
  tft.invertDisplay(true);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 5);
  tft.setTextSize(1);
  tft.print(voltage);

  pinMode(A0, INPUT);
  voltage = (float)(analogRead(0) * 5.0) / 1024;

  tft.print("Meas...");

  delay(500);

  pinMode(9, OUTPUT);  // ШИМ на D9
  // PWM_16KHZ_D9(DUTY);  // ШИМ 16 кГц на пине D9, заполнение DUTY из 1023
  pinMode(3, OUTPUT);  // пищалка на D3
  digitalWrite(3, 0);
  // подключили катод датчика на D2
  pinMode(2, INPUT_PULLUP);
  // FALLING - при пролете частицы будет сигнал 0, его и ловим
  attachInterrupt(0, btnIsr, FALLING);
}

void btnIsr() {
  counter++;     // + частица
  event = HIGH;  // частица пролетела
}

void loop() {
  if (event) {           // если пролетела частица обновляем данные температуры и влажности и пищим
    event = LOW;         // сразу сбрасываем флаг частицы
    digitalWrite(3, 1);  // пищим
    digitalWrite(3, 0);  // не пищим
  }
  ch_sec = millis() / 1000ul;
  if ((ch_sec - sec) > 59) {
    doza = round(counter * FON_NORMA / NORMA);
    counter = 0;
    sec = ch_sec;
    tft.setCursor(0, 1);
    // sprintf(myStr, "%d", int(doza));
    tft.setCursor(0, 1);
    tft.print("R background=");
    tft.print(doza);
    tft.print("uR/h");
    voltage = (float)(analogRead(0) * 5.0) / 1024;  // проверяем АКБ
    if (voltage < 3.2) {
      tft.fillRoundRect(0, 0, 100, 40, 0, ST77XX_BLACK);
      tft.setCursor(0, 0);
      tft.print("Accum LOW!!!");
      tft.print(int(voltage * 10));
      tft.setCursor(0, 10);
      tft.print("U=");
      // tft.print(myStr[0]);
      tft.print(",");
      // tft.print(myStr[1]);
      tft.print("V");
      while (1) {};
    }
  }
}