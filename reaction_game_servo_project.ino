/*
  Игра за реакция с логване в CSV и сервомотор
  Arduino Uno + Data Logger Shield (SD + RTC) + сервомотор

  Свързване:
    LED     -> пин 6 (през 220 ома към GND)
    Бутон   -> пин 2 и GND (използва се INPUT_PULLUP)
    Серво   -> сигнал (оранжев/жълт) на пин 9, червен на 5V, кафяв/черен на GND

  Библиотеки (Library Manager): "RTClib" на Adafruit. SD и Servo са вградени.
  Ако shield-ът ти е с DS3231 или PCF8523, смени реда с "RTC_DS1307 rtc;"
  на RTC_DS3231 или RTC_PCF8523.

  Поведение на сервото:
    реакция под GOOD_MS  -> "кимва" няколко пъти
    фалстарт             -> завърта се на другата страна и се връща
*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Servo.h>
#include "RTClib.h"

RTC_DS1307 rtc;            // <-- смени според твоя часовник
Servo servo;

const int BUTTON_PIN = 2;
const int LED_PIN = 6;
const int SERVO_PIN = 9;
const int SD_CS = 10;
const char FILENAME[] = "REACT.CSV";

const unsigned long MIN_WAIT = 2000;     // мин. пауза преди LED, ms
const unsigned long MAX_WAIT = 5000;     // макс. пауза преди LED, ms
const unsigned long TIMEOUT_MS = 3000;   // колко чакаме натискане след LED
const unsigned long GOOD_MS = 250;       // под това време = награда

const int SERVO_REST = 90;               // неутрално положение
const int SERVO_NOD_UP = 60;
const int SERVO_NOD_DOWN = 120;
const int SERVO_PUNISH = 170;            // "другата страна"

bool pressed() {
  return digitalRead(BUTTON_PIN) == LOW;
}

void waitForRelease() {
  while (pressed()) { }
  delay(30);  // дебаунс
}

void logResult(const char* result) {
  DateTime now = rtc.now();
  char line[64];
  snprintf(line, sizeof(line), "%04d-%02d-%02d,%02d:%02d:%02d,%s",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second(), result);

  Serial.println(line);

  File f = SD.open(FILENAME, FILE_WRITE);
  if (f) {
    f.println(line);
    f.close();
  } else {
    Serial.println(F("Грешка при запис на SD!"));
  }
}

// Сервото се закача само докато се движи
void nod() {
  servo.attach(SERVO_PIN);
  for (int i = 0; i < 3; i++) {
    servo.write(SERVO_NOD_UP);
    delay(200);
    servo.write(SERVO_NOD_DOWN);
    delay(200);
  }
  servo.write(SERVO_REST);
  delay(300);
  servo.detach();
}

void punish() {
  servo.attach(SERVO_PIN);
  servo.write(SERVO_PUNISH);
  delay(1200);
  servo.write(SERVO_REST);
  delay(400);
  servo.detach();
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Начално положение на сервото
  servo.attach(SERVO_PIN);
  servo.write(SERVO_REST);
  delay(500);
  servo.detach();

  if (!rtc.begin()) {
    Serial.println(F("Не намирам RTC часовника!"));
    while (true) { }
  }
  // Ако часовникът не е настройван, сложи часа от момента на компилиране
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!SD.begin(SD_CS)) {
    Serial.println(F("Не мога да инициализирам SD картата!"));
    while (true) { }
  }

  if (!SD.exists(FILENAME)) {
    File f = SD.open(FILENAME, FILE_WRITE);
    if (f) {
      f.println(F("date,time,reaction_ms"));
      f.close();
    }
  }

  randomSeed(analogRead(A0));  // A0 е свободен, шумът дава случайност
  Serial.println(F("Готово! Натисни бутона, за да започне рунд."));
}

void loop() {
  // 1. Чакаме натискане, за да започне рундът
  while (!pressed()) { }
  waitForRelease();

  // 2. Случайна пауза; ако натиснеш рано, това е фалстарт
  unsigned long waitTime = random(MIN_WAIT, MAX_WAIT);
  unsigned long start = millis();
  while (millis() - start < waitTime) {
    if (pressed()) {
      logResult("false_start");   // първо записваме, после движим сервото
      punish();
      waitForRelease();
      delay(500);
      return;
    }
  }

  // 3. Светваме LED и мерим времето
  digitalWrite(LED_PIN, HIGH);
  unsigned long t0 = micros();
  unsigned long tStart = millis();

  while (!pressed()) {
    if (millis() - tStart > TIMEOUT_MS) {
      digitalWrite(LED_PIN, LOW);
      logResult("timeout");
      delay(1000);
      return;
    }
  }
  unsigned long reaction = (micros() - t0) / 1000;  // в ms
  digitalWrite(LED_PIN, LOW);

  char buf[12];
  ultoa(reaction, buf, 10);
  logResult(buf);

  if (reaction < GOOD_MS) {
    nod();
  }

  waitForRelease();
  delay(1000);
}
