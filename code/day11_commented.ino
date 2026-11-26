/*
 * Smart Room Monitor — Simulation-First
 * --------------------------------------
 * Author:  [Your Name]
 * Day:     11 of 30 (YouthMakers Innovation Hub #YIH30Days)
 * Board:   Arduino Uno (simulated on Wokwi)
 *
 * What it does:
 *   - Reads temperature & humidity (DHT22)
 *   - Detects motion (PIR sensor)
 *   - Shows readings on a 0.96" OLED (SSD1306, I2C)
 *   - Red LED: temperature above TEMP_THRESHOLD
 *   - Green LED: motion detected
 *   - Buzzer: different beep patterns per alert state
 *   - Button: toggles mute (debounced)
 *
 * Wiring:
 *   DHT22 data  -> pin 2
 *   PIR signal  -> pin 3
 *   Button      -> pin 4 (to GND, INPUT_PULLUP)
 *   Red LED     -> pin 8 (via 220Ω)
 *   Green LED   -> pin 9 (via 220Ω)
 *   Buzzer (+)  -> pin 10
 *   OLED SDA    -> A4
 *   OLED SCL    -> A5
 *
 * Note: All strings are wrapped in F() to save RAM.
 *       The Uno only has 2KB, and the OLED buffer takes 1KB.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// --- Display configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Pin assignments ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define PIRPIN 3
#define BUTTON 4
#define RED_LED 8
#define GREEN_LED 9
#define BUZZER 10

// --- Tunable constants ---
#define TEMP_THRESHOLD 30.0    // °C — red LED + temp alert above this
#define DEBOUNCE_DELAY 50      // ms — button must be stable this long
#define PIR_WARMUP_MS 5000     // ms — use 30000 on real hardware

DHT dht(DHTPIN, DHTTYPE);

// --- Global state ---
bool muted = false;                  // true when buzzer is silenced
bool lastButtonState = HIGH;         // previous raw button reading
unsigned long lastDebounceTime = 0;  // last time button state changed

float temperature = 0.0;             // last valid DHT22 temperature
float humidity = 0.0;                // last valid DHT22 humidity
bool sensorReadOk = false;           // false if DHT22 returned NaN
bool tempAlert = false;              // temperature above threshold
bool motionAlert = false;            // PIR currently detecting motion

// Buzzer mode string — assigned inside updateBuzzer().
// Cannot use F() at global scope on AVR (compile error), so NULL here.
const __FlashStringHelper* buzzMode = NULL;

// --- Function prototypes ---
void handleButton();
void readSensors();
void updateAlerts();
void updateBuzzer();
void updateSerial();
void updateOLED();

void setup() {
  Serial.begin(9600);

  // Initialize OLED first — it claims 1KB of RAM for its buffer.
  // If this fails, nothing else matters, so halt.
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    for (;;);
  }

  dht.begin();
  pinMode(PIRPIN, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);   // internal pull-up: unpressed = HIGH
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  noTone(BUZZER);                  // ensure buzzer is silent at boot

  // Boot screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Smart Room Monitor"));
  display.println(F("Warming up PIR..."));
  display.display();

  Serial.println(F("Smart Room Monitor - Day 11"));
  Serial.println(F("Warming up PIR sensor..."));
  delay(PIR_WARMUP_MS);
  Serial.println(F("PIR ready."));
}

void loop() {
  // Each function does exactly one thing.
  // Order matters: read -> decide -> act -> display.
  handleButton();
  readSensors();
  updateAlerts();
  updateBuzzer();
  updateSerial();
  updateOLED();
  delay(1000);
}

/*
 * Reads the button and toggles mute on a debounced press.
 *
 * Debouncing: mechanical buttons "bounce" — they make and break
 * contact several times in a few milliseconds. Without debouncing,
 * one press registers as many presses. We only accept a change if
 * the reading has been stable for DEBOUNCE_DELAY ms.
 */
void handleButton() {
  bool reading = digitalRead(BUTTON);

  // If the reading changed, reset the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only act if the reading has been stable long enough
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading == LOW) {           // LOW = pressed (INPUT_PULLUP)
      muted = !muted;
      Serial.print(F("Mute: "));
      Serial.println(muted ? F("ON") : F("OFF"));
    }
  }

  lastButtonState = reading;
}

/*
 * Reads DHT22 (temperature + humidity) and PIR (motion).
 * Updates global state. Does NOT set LEDs or buzzer — that's
 * updateAlerts() and updateBuzzer().
 *
 * DHT22 needs ~2s between reads. We read every loop (1s delay),
 * so every other read returns NaN. sensorReadOk tracks that.
 */
void readSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  sensorReadOk = !(isnan(humidity) || isnan(temperature));
  motionAlert = (digitalRead(PIRPIN) == HIGH);
}

/*
 * Decides alert states and drives the LEDs.
 * tempAlert requires a valid sensor reading — if the DHT failed,
 * we don't want a false "too hot" alert.
 */
void updateAlerts() {
  tempAlert = (sensorReadOk && temperature > TEMP_THRESHOLD);
  digitalWrite(RED_LED, tempAlert);
  digitalWrite(GREEN_LED, motionAlert);
}

/*
 * Drives the buzzer based on current alert state.
 * Mute overrides everything — if muted, silence regardless.
 *
 * Beep patterns:
 *   Both alerts -> 1000Hz, 200ms (urgent, rapid)
 *   Temp only   -> 1500Hz, 500ms (high, sustained)
 *   Motion only ->  800Hz, 300ms (low, short)
 *   Nothing     -> silence
 *
 * Also sets buzzMode (flash string) so Serial and OLED can show it.
 */
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

/*
 * Prints one status line to the Serial Monitor.
 * Format: Temp: 28.5 C | Hum: 45 % | M:N | Buzzer: OFF
 * If the DHT read failed, prints a single error line instead.
 */
void updateSerial() {
  if (!sensorReadOk) {
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

/*
 * Redraws the OLED. Called once per loop.
 *
 * Layout:
 *   Row 0-1:  Title "SMART ROOM MONITOR"
 *   Row 2-3:  Temperature (size 2, large)
 *   Row 4-5:  Humidity (size 2, large)
 *   Row 6:    M:Y/N  B:<buzzMode>
 *
 * Must call display.display() at the end or nothing shows.
 */
void updateOLED() {
  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("SMART ROOM MONITOR"));
  display.println(F("-----------------"));

  display.setTextSize(2);
  display.setCursor(0, 16);
  if (sensorReadOk) {
    display.print(temperature, 1);
    display.print(F(" C"));
  } else {
    display.print(F("ERR"));
  }

  display.setCursor(0, 36);
  if (sensorReadOk) {
    display.print(humidity, 0);
    display.print(F(" %"));
  } else {
    display.print(F("ERR"));
  }

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(F("M:"));
  display.print(motionAlert ? F("Y") : F("N"));
  display.print(F(" B:"));
  display.print(buzzMode);

  display.display();
}
