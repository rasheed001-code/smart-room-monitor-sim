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
#define DEBOUNCE_DELAY 50

DHT dht(DHTPIN, DHTTYPE);

// --- Global state ---
bool muted = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

float temperature = 0.0;
float humidity = 0.0;
bool dhtOk = false;
bool tempAlert = false;
bool motionAlert = false;
const __FlashStringHelper* buzzMode = NULL;   // assigned inside updateBuzzer()

// --- Function prototypes ---
void handleButton();
void readSensors();
void updateAlerts();
void updateBuzzer();
void updateSerial();
void updateOLED();

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

  Serial.println(F("Smart Room Monitor - Day 10"));
  Serial.println(F("Warming up PIR sensor..."));
  delay(5000);
  Serial.println(F("PIR ready."));
}

void loop() {
  handleButton();
  readSensors();
  updateAlerts();
  updateBuzzer();
  updateSerial();
  updateOLED();
  delay(1000);
}

// --- Reads button, debounces, toggles mute ---
void handleButton() {
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
}

// --- Reads DHT22 and PIR, updates sensor state ---
void readSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  dhtOk = !(isnan(humidity) || isnan(temperature));
  motionAlert = (digitalRead(PIRPIN) == HIGH);
}

// --- Decides alerts, sets LEDs ---
void updateAlerts() {
  tempAlert = (dhtOk && temperature > TEMP_THRESHOLD);
  digitalWrite(RED_LED, tempAlert);
  digitalWrite(GREEN_LED, motionAlert);
}

// --- Controls buzzer based on alerts + mute ---
void updateBuzzer() {
  if (muted) {
    buzzMode = F("MUTED");
    noTone(BUZZER);
    return;
  }

  if (tempAlert && motionAlert) {
    buzzMode = F("URGENT");
    tone(BUZZER, 1000, 200);
  } else if (tempAlert) {
    buzzMode = F("TEMP");
    tone(BUZZER, 1500, 500);
  } else if (motionAlert) {
    buzzMode = F("MOTION");
    tone(BUZZER, 800, 300);
  } else {
    buzzMode = F("OFF");
    noTone(BUZZER);
  }
}

// --- Prints one line to Serial Monitor ---
void updateSerial() {
  if (!dhtOk) {
    Serial.println(F("DHT read failed!"));
    return;
  }
  Serial.print(F("Temp: "));
  Serial.print(temperature);
  Serial.print(F(" C | Hum: "));
  Serial.print(humidity);
  Serial.print(F(" % | M:"));
  Serial.print(motionAlert ? F("Y") : F("N"));
  Serial.print(F(" | Buzzer: "));
  Serial.println(buzzMode);
}

// --- Redraws the OLED display ---
void updateOLED() {
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
}
