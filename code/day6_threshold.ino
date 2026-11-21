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

#define TEMP_THRESHOLD 30.0  // °C — change this to test

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(PIRPIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

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

  Serial.println("Smart Room Monitor — Day 6");
  Serial.println("Warming up PIR sensor (30 seconds)...");
  delay(30000);
  Serial.println("PIR ready.");
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int motionState = digitalRead(PIRPIN);

  // --- Threshold logic ---
  bool tempAlert = (!isnan(temperature) && temperature > TEMP_THRESHOLD);
  bool motionAlert = (motionState == HIGH);

  digitalWrite(RED_LED, tempAlert ? HIGH : LOW);
  digitalWrite(GREEN_LED, motionAlert ? HIGH : LOW);

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
    Serial.println(tempAlert ? "ON" : "OFF");
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
  display.print("Motion: ");
  display.print(motionAlert ? "DETECTED" : "None");
  display.print("  Temp: ");
  display.print(tempAlert ? "HOT" : "OK");

  display.display();

  delay(1000); // Faster loop so LEDs respond quickly
}
