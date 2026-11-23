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

#define TEMP_THRESHOLD 30.0

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(PIRPIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  noTone(BUZZER); // ensure buzzer is silent at boot

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Smart Room Monitor");
  display.println("Warming up PIR...");
  display.display();

  Serial.println("Smart Room Monitor — Day 8");
  Serial.println("Warming up PIR sensor (30 seconds)...");
  delay(30000);
  Serial.println("PIR ready.");
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int motionState = digitalRead(PIRPIN);

  bool tempAlert = (!isnan(temperature) && temperature > TEMP_THRESHOLD);
  bool motionAlert = (motionState == HIGH);

  digitalWrite(RED_LED, tempAlert ? HIGH : LOW);
  digitalWrite(GREEN_LED, motionAlert ? HIGH : LOW);

  // --- Buzzer logic ---
  if (tempAlert && motionAlert) {
    // Both alerts: rapid urgent beep
    tone(BUZZER, 1000, 200);
  } else if (tempAlert) {
    // Temp alert only: high-pitched continuous-ish beep
    tone(BUZZER, 1500, 500);
  } else if (motionAlert) {
    // Motion alert only: lower, shorter beep
    tone(BUZZER, 800, 300);
  } else {
    noTone(BUZZER);
  }

  // --- Serial output ---
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT read failed!");
  } else {
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.print(" % | Motion: ");
    Serial.print(motionAlert ? "YES" : "NO");
    Serial.print(" | TempAlert: ");
    Serial.print(tempAlert ? "ON" : "OFF");
    Serial.print(" | Buzzer: ");
    if (tempAlert && motionAlert) Serial.println("URGENT");
    else if (tempAlert) Serial.println("TEMP");
    else if (motionAlert) Serial.println("MOTION");
    else Serial.println("OFF");
  }

  // --- OLED output ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("SMART ROOM MONITOR");
  display.println("-----------------");

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(temperature, 1);
  display.print(" C");

  display.setCursor(0, 36);
  display.print(humidity, 0);
  display.print(" %");

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("M:");
  display.print(motionAlert ? "YES" : "NO");
  display.print(" T:");
  display.print(tempAlert ? "HOT" : "OK");
  display.print(" B:");
  if (tempAlert && motionAlert) display.print("URG");
  else if (tempAlert) display.print("TMP");
  else if (motionAlert) display.print("MOT");
  else display.print("OFF");

  display.display();

  delay(1000);
}
