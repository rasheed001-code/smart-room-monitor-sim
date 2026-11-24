#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 2
#define DHTTYPE DHT22
#define PIRPIN 3
#define RED_LED 8
#define GREEN_LED 9
#define BUZZER 10
#define BUTTON 4

#define TEMP_THRESHOLD 30.0

DHT dht(DHTPIN, DHTTYPE);

bool muted = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    for (;;);
  }

  dht.begin();
  pinMode(PIRPIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  noTone(BUZZER);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Smart Room Monitor"));
  display.println(F("Warming up PIR..."));
  display.display();

  Serial.println(F("Smart Room Monitor - Day 9"));
  Serial.println(F("Warming up PIR sensor..."));
  delay(5000);
  Serial.println(F("PIR ready."));
}

void loop() {
  // --- Button debounce ---
  bool reading = digitalRead(BUTTON);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading == LOW) {
      muted = !muted;
      Serial.print(F("Mute: "));
      Serial.println(muted ? F("ON") : F("OFF"));
    }
  }
  lastButtonState = reading;

  // --- Sensors ---
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  bool dhtOk = !(isnan(humidity) || isnan(temperature));

  bool tempAlert = (dhtOk && temperature > TEMP_THRESHOLD);
  bool motionAlert = (digitalRead(PIRPIN) == HIGH);

  digitalWrite(RED_LED, tempAlert);
  digitalWrite(GREEN_LED, motionAlert);

  // --- Buzzer (respects mute) ---
  if (muted) {
    noTone(BUZZER);
  } else if (tempAlert && motionAlert)  tone(BUZZER, 1000, 200);
  else if (tempAlert)                   tone(BUZZER, 1500, 500);
  else if (motionAlert)                 tone(BUZZER, 800, 300);
  else                                  noTone(BUZZER);

  const __FlashStringHelper* buzzMode =
      muted ? F("MUTED") :
      (tempAlert && motionAlert) ? F("URGENT") :
      tempAlert ? F("TEMP") :
      motionAlert ? F("MOTION") : F("OFF");

  // --- Serial ---
  if (!dhtOk) {
    Serial.println(F("DHT read failed!"));
  } else {
    Serial.print(F("Temp: "));
    Serial.print(temperature);
    Serial.print(F(" C | Hum: "));
    Serial.print(humidity);
    Serial.print(F(" % | M:"));
    Serial.print(motionAlert ? F("Y") : F("N"));
    Serial.print(F(" | Buzzer: "));
    Serial.println(buzzMode);
  }

  // --- OLED ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("SMART ROOM MONITOR"));
  display.println(F("-----------------"));

  display.setTextSize(2);
  display.setCursor(0, 16);
  if (dhtOk) { display.print(temperature, 1); display.print(F(" C")); }
  else       { display.print(F("ERR")); }

  display.setCursor(0, 36);
  if (dhtOk) { display.print(humidity, 0); display.print(F(" %")); }
  else       { display.print(F("ERR")); }

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(F("M:"));
  display.print(motionAlert ? F("Y") : F("N"));
  display.print(F(" B:"));
  display.print(buzzMode);

  display.display();
  delay(1000);
}
