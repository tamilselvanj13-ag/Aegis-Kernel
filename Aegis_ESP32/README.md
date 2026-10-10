# Aegis_Kernal : TinyML Process Telemetry & Threat Classifier on ESP32

Real-time, sub-microsecond AI threat detection running directly on embedded microcontrollers (**ESP32 / ESP32-S3 / ESP8266**) using the **Arduino IDE (.ino)** framework.

---

## ⚡ Key Highlights
- **Sub-Microsecond Latency**: Evaluates complex 8-dimensional telemetry vectors in **< 1.5 µs** on a 240 MHz Xtensa core.
- **Zero Dynamic Heap Overhead**: 0 bytes dynamic memory (`malloc`/`new`) allocation; 100% stack/register execution for mission-critical embedded stability.
- **On-Device Shannon Entropy**: Computes character frequency distribution directly on the microcontroller to identify packed, obfuscated, and Base64-encoded payloads.
- **Interactive Serial CLI**: Full command console at 115200 baud for testing, live evaluation, benchmarking, and real-time process monitoring.
- **Hardware Threat Indicator**: Direct visual alert signaling via onboard LED (GPIO 2) or external LEDs.

---

## 🛠️ Project Structure

```
Aegis_ESP32/
├── Aegis_ESP32.ino          # Main Arduino firmware sketch (Serial CLI + Entropy + LED control)
├── aegis_model.h            # Transpiled zero-dependency TinyML C inference engine
├── export_esp32_model.py    # Python training & C header transpilation pipeline
├── test_engine.c            # Standalone host C verification & benchmark script
├── dataset.csv              # Process telemetry training dataset
└── README.md                # Documentation & flashing guide
```

---

## 🔌 Hardware Wiring & Pinout (ESP32 V1 Dev Kit)

| Module / Component | Pin on Module | ESP32 V1 Pin | Notes / Protocol |
|---|---|---|---|
| **0.96" OLED (SSD1306)** | `VCC` | `3.3V` (or `VIN`) | Power |
| | `GND` | `GND` | Ground |
| | `SDA` | `GPIO 21` | Hardware I2C SDA |
| | `SCL` | `GPIO 22` | Hardware I2C SCL |
| **Buzzer (Active/Passive)**| `Positive (+)` | `GPIO 4` | Digital Alarm Tone Trigger |
| | `Negative (-)` | `GND` | Ground |
| **Onboard Blue LED** | `Built-in` | `GPIO 2` | Visual Threat Indicator |

---

## 📚 Required Arduino IDE Libraries

In the Arduino IDE, go to **Tools > Manage Libraries...** (or `Ctrl + Shift + I`) and install:
1. **`Adafruit SSD1306`** by Adafruit (Select *"Install All"* to also get `Adafruit GFX Library`).
2. **`Adafruit GFX Library`** by Adafruit.

*(Note: If the OLED is not connected, the firmware automatically detects it and smoothly runs in Serial & LED/Buzzer mode without hanging).*

---

## 🚨 Threat Feedback Matrix

| Threat Level | OLED Visual Badge | Buzzer Sound | Onboard LED (GPIO 2) |
|---|---|---|---|
| **`[SAFE]`** | Outline Box: `[ SAFE ]` | Silent | OFF / Single Pulse |
| **`[SUSPICIOUS]`** | Outline Box: `[! SUSPICIOUS !]` | 2 Short Warning Chirps | Double Blink |
| **`[MALICIOUS]`** | Solid Inverted: `*** MALICIOUS ***` | 4 Rapid Alarm Pulses | 5x Rapid Flash + Solid ON |
| **`[CRITICAL]`** | Solid Inverted: `!! CRITICAL C2 !!` | Continuous Siren Alarm | 5x Rapid Flash + Solid ON |

---

## 🚀 How to Flash with Arduino IDE

### 1. Requirements:
- [Arduino IDE](https://www.arduino.cc/en/software) (2.x or 1.8.x)
- ESP32 Board Package installed via Board Manager (`esp32` by Espressif Systems)

### 2. Configuration:
1. Open `Aegis_ESP32.ino` in Arduino IDE.
2. Under **Tools > Board**, select **ESP32 Dev Module** (or your specific ESP32-S3 / NodeMCU-32S board).
3. Under **Tools > CPU Frequency**, select **240MHz (WiFi/BT)**.
4. Under **Tools > Upload Speed**, select **921600** (or 115200).
5. Select the correct **Port** for your connected ESP32.

### 3. Flash & Monitor:
1. Click **Upload** (Ctrl + U / Cmd + U).
2. Open **Serial Monitor** (Ctrl + Shift + M).
3. Set Baud Rate to **`115200`** and Line Ending to **`Newline`**.

---

## 💻 Serial Monitor Commands

Once opened, you can interact with the microcontroller in real time:

| Command | Action | Example |
|---|---|---|
| `TEST` | Runs the full pre-configured attack vector test suite with microsecond latency metrics | `TEST` |
| `BENCH` | Runs 10,000 continuous inferences and computes average latency & throughput | `BENCH` |
| `EVAL <cmdline>` | Computes on-device Shannon entropy and classifies custom command strings | `EVAL /tmp/x86_dropper -e aW1wb3J0` |
| `VEC <8 params>` | Ingests raw 8D numerical vector `temp,len,entropy,root,web,dead,memfd,ld` | `VEC 1,120,4.65,1,0,1,0,1` |
| `HELP` | Displays available interactive commands | `HELP` |

---

## 🐍 Retraining & Updating Model Rules

If you collect new process telemetry samples from Linux `/proc`:
1. Place your updated `dataset.csv` inside this folder.
2. Run:
   ```bash
   python export_esp32_model.py
   ```
3. The script will retrain the decision tree and automatically regenerate `aegis_model.h` ready for re-flashing!
