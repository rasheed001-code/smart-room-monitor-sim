#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT22
#define PIRPIN 3

DHT dht(DHTPIN, DHTTYPE);

int motionState = LOW;

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(PIRPIN, INPUT);

  Serial.println("Smart Room Monitor — Day 4");
  Serial.println("Warming up PIR sensor (30 seconds)...");
  delay(30000); // PIR needs 30s to stabilize
  Serial.println("PIR ready.");
}

void loop() {
  // Read DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT read failed!");
  } else {
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.print(" %");
  }

  // Read PIR
  motionState = digitalRead(PIRPIN);
  if (motionState == HIGH) {
    Serial.println("  |  MOTION DETECTED");
  } else {
    Serial.println("  |  No motion");
  }

  delay(2000); // Match DHT22 read interval
}
