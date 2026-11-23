# Smart Room Monitor (Simulation-First)

A simulated embedded system built on Wokwi that monitors temperature, humidity, and motion — with OLED display and LED alerts. No physical hardware required.

Built as part of the **YouthMakers Innovation Hub 30-Day Challenge** (#YIH30Days).

---

## The Problem

I wanted to build a room monitor, but I had no physical components where I am. Most embedded tutorials assume you have an Arduino, sensors, and a breadboard. I didn't.

So I'm building it in simulation first. The code is real. The logic is real. The only thing virtual is the wire.

---

## What It Does

- Reads temperature and humidity (DHT22)
- Detects motion (PIR sensor)
- Displays live readings on a 0.96" OLED (SSD1306)
- Turns on a **red LED** when temperature exceeds 30°C
- Turns on a **green LED** when motion is detected
- (Coming in Week 2) Buzzer alerts + mute button

---

## Tech Stack

| Tool | Purpose |
|------|---------|
| [Wokwi](https://wokwi.com) | Browser-based Arduino simulator |
| Arduino C++ | Firmware |
| GitHub | Code + documentation |
| Adafruit DHT / SSD1306 / GFX libraries | Sensor + display drivers |

---

## Circuit (Simulated)

| Component | Arduino Pin |
|-----------|-------------|
| DHT22 Data | 2 |
| PIR Signal | 3 |
| Red LED | 8 |
| Green LED | 9 |
| OLED SDA | A4 |
| OLED SCL | A5 |
| All VCC | 5V |
| All GND | GND |

*(Wiring diagram screenshot: `screenshots/day7_hero.png`)*

---

## How to Run

1. Open the Wokwi project: (https://wokwi.com/projects/481109413513101313)
2. Click the green **Run** button.
3. Wait 30 seconds for the PIR sensor to warm up.
4. Click the DHT22 sensor to change temperature/humidity.
5. Click the PIR sensor to trigger motion.
6. Watch the OLED, LEDs, and Serial Monitor respond.

---

## Progress Log

### Week 1 (Days 1–7) — Foundations & First Readings
- **Day 1:** Blink LED on simulated Arduino Uno
- **Day 2:** Project brief written, GitHub repo created
- **Day 3:** DHT22 temperature/humidity reading to Serial Monitor
- **Day 4:** PIR motion detection integrated
- **Day 5:** OLED display showing live sensor data
- **Day 6:** Threshold logic — red LED (temp alert), green LED (motion alert)
- **Day 7:** Week 1 recap — full system working

**Week 1 status:** ✅ Working simulated system. Code is functional but not yet refactored (single loop, no functions). Week 2 will fix that.

### Week 2 (Days 8–14) — Coming
- Buzzer alerts
- Mute button (debouncing)
- Code refactor into functions
- Comments on every function

### Week 3 (Days 15–21) — Coming
- Hardware polish / wiring cleanup
- Full README + wiring diagram
- Stretch goal: ESP32 WiFi + dashboard

### Week 4 (Days 22–30) — Coming
- Final testing
- Demo video
- Portfolio page
- Final showcase post

---

## What I've Learned So Far

- PIR sensors need 30–60 seconds to warm up. Don't trust them immediately.
- DHT22 needs 2 seconds between reads. Read it too fast and you get `NaN`.
- I2C only needs two wires (SDA + SCL) — that's why OLEDs are so popular.
- `display.display()` is required after every OLED update, or nothing shows.
- Ternary operators (`condition ? HIGH : LOW`) make LED logic one line instead of five.
- Simulation-first development is a real workflow, not a compromise. The code ports to hardware unchanged.

---

## What's Next

- Week 2: buzzer, button, refactor, comments
- Week 3: polish, document, ESP32 WiFi stretch goal
- Week 4: final demo, portfolio page, showcase

---

## Links

- **Wokwi simulation:** (https://wokwi.com/projects/481109413513101313)
- **LinkedIn journey:** https://www.linkedin.com/in/abdulrasheed-sanni-9882b1383
- **Challenge:** [YouthMakers Innovation Hub](https://www.linkedin.com/company/youthmakers-hub/) — #YIH30Days

---

## Author

**Sanni, Abdulrasheed Moyosore**
Embedded systems learner. Building in public for 30 days.
https://www.linkedin.com/in/abdulrasheed-sanni-9882b1383 · https://github.com/rasheed001-code/smart-room-monitor-sim

---

*We make. We build. We innovate.* ⚡
