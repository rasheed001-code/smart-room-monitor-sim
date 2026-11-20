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

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(PIRPIN, INPUT);

  // Initialize OLED
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

  Serial.println("Smart Room Monitor — Day 5");
  Serial.println("Warming up PIR sensor (30 seconds)...");
  delay(30000);
  Serial.println("PIR ready.");
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int motionState = digitalRead(PIRPIN);

  // Serial output (keep for debugging)
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT read failed!");
  } else {
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.print(" %");
    Serial.print(" | Motion: ");
    Serial.println(motionState == HIGH ? "YES" : "NO");
  }

  // OLED output
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
  display.print(motionState == HIGH ? "DETECTED" : "None");

  display.display();

  delay(2000);
}
