# Low-Cost Weather and Atmospheric Pressure Monitoring Station 🌦️⚡
> **कम लागत वाला मौसम और वायुमंडलीय दबाव निगरानी स्टेशन**
> Designed for Virtual Simulation on [Wokwi.com](https://wokwi.com/) & Physical Hardware Implementation.

---

## 📌 Project Overview (परियोजना का विवरण)

This project is a **low-cost weather and atmospheric pressure monitoring station** built on IoT and embedded systems principles. It reads real-time environmental data (temperature, humidity, and barometric pressure), then uses it to estimate **altitude** from air pressure and calculate the **dew point**.

Results are displayed in two ways at the same time:

- **16x2 I2C LCD**: a compact on-device display for quick readings
- **Serial Monitor**: a CLI dashboard, plus **CSV output** for logging and later analysis in tools like Excel or Python.

### Key Metrics Monitored:
1. **Atmospheric Pressure**: In `hPa` (hectopascals) and `mmHg` (millimeters of mercury).
2. **Ambient Temperature**: Dual-sensor precision (°C).
3. **Relative Humidity**: Percentage (`%RH`).
4. **Estimated Altitude**: Calculated from the international barometric formula against sea level.
5. **Dew Point**: Calculated using the Magnus-Tetens approximation.
6. **Weather Trend / Forecast**: Barometric tendency tracking (Rising -> Clear/Fair, Falling -> Rain/Storm warning, Steady).

---

## 💰 Low-Cost Bill of Materials (BOM)
| Component | Function | Approx. Market Cost (INR) | Approx. Cost (USD) |
| :--- | :--- | :--- | :--- |
| **ESP32 DevKit V1** (or Arduino Uno) | Core Microcontroller / WiFi ready | ₹350 - ₹400 | ~$4.50 |
| **BMP280 Sensor** | Barometric Pressure & Temperature | ₹120 - ₹150 | ~$1.50 |
| **DHT22 Sensor** | Ambient Temperature & Humidity | ₹140 - ₹180 | ~$2.00 |
| **16x2 LCD with I2C Backpack** | Visual Display (2 wires for data) | ₹150 - ₹180 | ~$2.00 |
| **Jumper Wires & Breadboard** | Interconnections | ₹50 - ₹80 | ~$0.80 |
| **Total Estimated Cost** | **Complete Hardware Station** | **~₹810 - ₹990** | **~$10.80** |
| **Wokwi Simulator Cost** | **Virtual Simulation** | **₹0.00 (Free)** | **$0.00** |

---

## 🔌 Circuit Pin Connections (पिन कनेक्शन विवरण)

### 1. ESP32 Configuration (Primary / Default)
| Component Pin | ESP32 Pin | Wire Color | Description |
| :--- | :--- | :--- | :--- |
| **BMP280 VCC** | `3V3` | Red | 3.3V Power |
| **BMP280 GND** | `GND` | Black | Ground |
| **BMP280 SDA** | `GPIO 21` | Blue | I2C Data Line |
| **BMP280 SCL** | `GPIO 22` | Yellow | I2C Clock Line |
| **DHT22 VCC** | `3V3` (or `VIN`) | Red | Power |
| **DHT22 GND** | `GND` | Black | Ground |
| **DHT22 SDA/Data** | `GPIO 23` | Orange | Digital Data Pin |
| **LCD1602 VCC** | `3V3` (or `VIN`) | Red | Power |
| **LCD1602 GND** | `GND` | Black | Ground |
| **LCD1602 SDA** | `GPIO 21` | Blue | I2C Data Line (Shared) |
| **LCD1602 SCL** | `GPIO 22` | Yellow | I2C Clock Line (Shared) |

---

### 2. Arduino Uno Configuration (Alternative)
Located in folder: `arduino_uno/`
- **BMP280**: VCC -> `3.3V`, GND -> `GND`, SDA -> `A4`, SCL -> `A5`
- **DHT22**: VCC -> `5V`, GND -> `GND`, Data -> `Pin 2`
- **LCD1602 (I2C)**: VCC -> `5V`, GND -> `GND`, SDA -> `A4`, SCL -> `A5`

---

## 🖥️ Output Demonstrations

### 1. 16x2 LCD Display (Cycles every 3 seconds)
```text
Screen 1 (Temp & Humidity):
+----------------+
|Temp: 28.5°C    |
|Humidity: 65%   |
+----------------+

Screen 2 (Pressure & Altitude):
+----------------+
|P: 1013.2hPa    |
|Alt: 15.4m      |
+----------------+

Screen 3 (Dew Point & Trend):
+----------------+
|DewPt: 21.3°C   |
|Trend: STEADY   |
+----------------+
```

### 2. Serial Monitor CLI Dashboard (Baud: 115200)
```text
+---------------------------------------------------+
|       ATMOSPHERIC & WEATHER METRICS REPORT        |
+-------------------------+-------------------------+
| Temperature:            | 28.5 *C                 |
| Relative Humidity:      | 65.0 %                  |
| Barometric Pressure:    | 1013.25 hPa             |
| Pressure (mmHg):        | 760.00 mmHg             |
| Estimated Altitude:     | 15.4 m                  |
| Dew Point:              | 21.3 *C                 |
| Barometric Trend:       | STEADY                  |
+-------------------------+-------------------------+
[CSV]: 12,28.50,65.00,1013.25,15.42
```

---

## 🚀 How to Run on Wokwi Simulator (Wokwi me kaise chalayein)

### Quick Setup (Under 1 Minute):
1. Browser me **[https://wokwi.com/](https://wokwi.com/)** open karein.
2. **ESP32** (ya **Arduino Uno**) choose karein.
3. **Code Tab (`sketch.ino`)**:
   - `weather-station/sketch.ino` ka pura code copy karke paste karein.
4. **Diagram Tab (`diagram.json`)**:
   - Wokwi me `diagram.json` tab par jayein aur `weather-station/diagram.json` ka content paste karein. (Isse circuit diagram automatically draw ho jayega!)
5. **Library Manager (`libraries.txt`)**:
   - "Library Manager" tab me jayein aur yeh 4 libraries add karein:
     - `Adafruit BMP280 Library`
     - `DHT sensor library`
     - `Adafruit Unified Sensor`
     - `LiquidCrystal I2C`
6. Green **▶ Start Simulation** button dabayein!
7. Simulation me sensor slider ko move karke real-time temperature, pressure aur humidity change hote hue LCD aur Serial Monitor par dekhein.

---

## 📤 How to Upload Project to GitHub (GitHub me kaise upload karein)

Aap is project ko do tarike se GitHub par upload kar sakte hain:

### Method 1: Using Terminal / Git Commands (Recommended)

Terminal / Command Prompt open karein aur yeh commands run karein:

```bash
# 1. Project folder me status check karein
git status

# 2. weather-station files ko git staging me add karein
git add weather-station/

# 3. Commit message likhein
git commit -m "Add Low-Cost Weather and Atmospheric Pressure Monitoring Station for Wokwi"

# 4. GitHub main branch par push karein
git push origin main
```

---

### Method 2: If Creating a Brand New Separate GitHub Repository
Agar aap is project ke liye bilkul nayi repository banana chahte hain:
1. [GitHub](https://github.com/new) par jayein aur nayi repo banayein: `Low-Cost-Weather-Monitoring-Station`.
2. Terminal me commands run karein:
```bash
cd weather-station
git init
git add .
git commit -m "Initial commit: Low-Cost Weather and Pressure Station"
git branch -M main
git remote add origin https://github.com/<YOUR-USERNAME>/<YOUR-REPO-NAME>.git
git push -u origin main
```

---

### Method 3: GitHub Website Direct Upload (Bina Command ke)
1. Browser me apni GitHub repository kholein.
2. **Add file** -> **Upload files** par click karein.
3. `weather-station` folder ke andar ki sabhi files (`sketch.ino`, `diagram.json`, `libraries.txt`, `README.md`) ko drag & drop karein.
4. Niche **Commit changes** par click karein.

---

## 📚 Libraries & Dependencies
- [Adafruit BMP280 Library](https://github.com/adafruit/Adafruit_BMP280_Library)
- [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_Sensor)
- [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
- [LiquidCrystal I2C](https://github.com/johnrickman/LiquidCrystal_I2C)

---

## 👨‍💻 Author & License
Open-Source Educational Project released under MIT License.
