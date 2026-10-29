#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>

/* ====================================================================
 * Project: Low-Cost Weather and Atmospheric Pressure Monitoring Station
 * Platform: ESP32 DevKit v1 / Wokwi Simulator
 * Sensors:
 *   - BMP280: Barometric Pressure & High-Precision Temperature (I2C)
 *   - DHT22: Ambient Temperature & Relative Humidity (Digital Pin)
 * Display:
 *   - LCD1602 with I2C Backpack (Address: 0x27)
 *   - High-Speed Serial Monitor (115200 baud)
 * ==================================================================== */

// Pin Definitions
#define DHTPIN 23          // DHT22 Data pin connected to ESP32 GPIO 23
#define DHTTYPE DHT22      // DHT 22 (AM2302)
#define BARO_ANALOG_PIN 34 // GPIO 34 for Barometer / Pressure Sensor dial (Wokwi & Analog Transducer)

// Sea level standard atmospheric pressure in hPa (used for altitude calculation)
#define SEALEVELPRESSURE_HPA (1013.25)

// Instances
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp; // I2C interface
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Custom degree character for LCD
byte degreeChar[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// Timing variables (Non-blocking design)
unsigned long lastSensorReadTime = 0;
const unsigned long SENSOR_INTERVAL = 2000; // Read sensors every 2 seconds

unsigned long lastLcdSwitchTime = 0;
const unsigned long LCD_SWITCH_INTERVAL = 3000; // Change LCD screen every 3 seconds
int lcdScreenIndex = 0;

// Measurement variables
float temperatureC = 0.0;
float humidity = 0.0;
float pressureHpa = 0.0;
float pressureMmHg = 0.0;
float altitudeM = 0.0;
float dewPointC = 0.0;
bool bmpAvailable = false;
bool dhtAvailable = false;

// Pressure history for weather forecasting trend
float prevPressure = 0.0;
String weatherTrend = "CALCULATING";

// Helper: Calculate Dew Point (°C) using Magnus formula
float calculateDewPoint(float temp, float hum) {
  float a = 17.27;
  float b = 237.7;
  float alpha = ((a * temp) / (b + temp)) + log(hum / 100.0);
  return (b * alpha) / (a - alpha);
}

// Helper: Determine weather trend from pressure movement
String determineTrend(float currentP, float oldP) {
  if (oldP == 0.0) return "STABLE";
  float diff = currentP - oldP;
  if (diff > 0.5) return "RISING (FAIR)";
  else if (diff < -0.5) return "FALLING (RAIN)";
  else return "STEADY";
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n=================================================="));
  Serial.println(F(" Low-Cost Weather & Atmospheric Pressure Station  "));
  Serial.println(F(" Initializing components...                       "));
  Serial.println(F("=================================================="));

  // Initialize I2C communication
  Wire.begin(21, 22); // ESP32 SDA = GPIO 21, SCL = GPIO 22
  Wire.setTimeOut(1000);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeChar);

  lcd.setCursor(0, 0);
  lcd.print("Weather Station");
  lcd.setCursor(0, 1);
  lcd.print("Starting up...");

  // Initialize DHT22
  dht.begin();
  delay(1000);

  // Initialize BMP280 with fallback addressing (0x76 or 0x77)
  if (bmp.begin(0x76)) {
    bmpAvailable = true;
    Serial.println(F("[OK] BMP280 detected at address 0x76"));
  } else if (bmp.begin(0x77)) {
    bmpAvailable = true;
    Serial.println(F("[OK] BMP280 detected at address 0x77"));
  } else {
    Serial.println(F("[WARN] Could not find a valid BMP280 sensor! Check connections."));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BMP280 Warning!");
    lcd.setCursor(0, 1);
    lcd.print("Check I2C Wire");
    delay(2000);
  }

  // BMP280 Default sampling parameters
  if (bmpAvailable) {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  lcd.setCursor(0, 1);
  lcd.print("Reading data...");
  delay(1500);
  lcd.clear();
}

void readSensors() {
  // Read DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperatureC = t;
    dhtAvailable = true;
    dewPointC = calculateDewPoint(temperatureC, humidity);
  } else {
    dhtAvailable = false;
    Serial.println(F("[ERROR] Failed to read from DHT22 sensor!"));
  }

  // Read Atmospheric Pressure (via BMP280 if present, or Calibrated Barometer Dial / Sensor)
  if (bmpAvailable) {
    float rawPressurePa = bmp.readPressure();
    pressureHpa = rawPressurePa / 100.0F;
    pressureMmHg = pressureHpa * 0.750062;
    altitudeM = bmp.readAltitude(SEALEVELPRESSURE_HPA);

    // If DHT failed, use BMP280 temperature as backup
    if (!dhtAvailable) {
      temperatureC = bmp.readTemperature();
    }
  } else {
    // Wokwi simulation / Analog Barometric Transducer (MPX4115A / Potentiometer on GPIO 34)
    int potValue = analogRead(BARO_ANALOG_PIN);
    // Map 0 - 4095 (12-bit ADC) to standard atmospheric range: 950.0 hPa - 1050.0 hPa
    pressureHpa = 950.0 + ((float)potValue / 4095.0) * 100.0;
    pressureMmHg = pressureHpa * 0.750062;
    // Standard Barometric formula for altitude calculation: h = 44330 * (1 - (P / P0)^(1/5.255))
    altitudeM = 44330.0 * (1.0 - pow(pressureHpa / SEALEVELPRESSURE_HPA, 0.1903));
  }

  // Update Weather Forecast Trend
  if (prevPressure == 0.0) {
    prevPressure = pressureHpa;
  }
  weatherTrend = determineTrend(pressureHpa, prevPressure);
  prevPressure = pressureHpa;
}

void printSerialDashboard() {
  Serial.println(F("\n+---------------------------------------------------+"));
  Serial.println(F("|       ATMOSPHERIC & WEATHER METRICS REPORT        |"));
  Serial.println(F("+-------------------------+-------------------------+"));
  
  Serial.print(F("| Temperature:            | "));
  Serial.print(temperatureC, 1);
  Serial.println(F(" *C                    |"));

  Serial.print(F("| Relative Humidity:      | "));
  if (dhtAvailable) {
    Serial.print(humidity, 1);
    Serial.println(F(" %                     |"));
  } else {
    Serial.println(F("N/A                     |"));
  }

  Serial.print(F("| Barometric Pressure:    | "));
  if (pressureHpa > 0) {
    Serial.print(pressureHpa, 2);
    Serial.println(F(" hPa                 |"));
  } else {
    Serial.println(F("N/A                     |"));
  }

  Serial.print(F("| Pressure (mmHg):        | "));
  if (pressureHpa > 0) {
    Serial.print(pressureMmHg, 2);
    Serial.println(F(" mmHg                |"));
  } else {
    Serial.println(F("N/A                     |"));
  }

  Serial.print(F("| Estimated Altitude:     | "));
  if (pressureHpa > 0) {
    Serial.print(altitudeM, 1);
    Serial.println(F(" m                   |"));
  } else {
    Serial.println(F("N/A                     |"));
  }

  Serial.print(F("| Dew Point:              | "));
  if (dhtAvailable) {
    Serial.print(dewPointC, 1);
    Serial.println(F(" *C                    |"));
  } else {
    Serial.println(F("N/A                     |"));
  }

  Serial.print(F("| Barometric Trend:       | "));
  Serial.print(weatherTrend);
  int pad = 24 - weatherTrend.length();
  for (int i = 0; i < pad; i++) Serial.print(" ");
  Serial.println(F("|"));

  Serial.println(F("+-------------------------+-------------------------+"));
  Serial.print(F("[CSV]: "));
  Serial.print(millis() / 1000);
  Serial.print(F(","));
  Serial.print(temperatureC);
  Serial.print(F(","));
  Serial.print(humidity);
  Serial.print(F(","));
  Serial.print(pressureHpa);
  Serial.print(F(","));
  Serial.println(altitudeM);
}

void updateLcdDisplay() {
  lcd.clear();

  switch (lcdScreenIndex) {
    case 0:
      // Screen 1: Temperature & Humidity
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temperatureC, 1);
      lcd.write(0); // Degree symbol
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Humidity: ");
      if (dhtAvailable) {
        lcd.print(humidity, 0);
        lcd.print("%");
      } else {
        lcd.print("--%");
      }
      break;

    case 1:
      // Screen 2: Atmospheric Pressure & Altitude
      lcd.setCursor(0, 0);
      lcd.print("P: ");
      if (pressureHpa > 0) {
        lcd.print(pressureHpa, 1);
        lcd.print(" hPa");
      } else {
        lcd.print("No Sensor");
      }

      lcd.setCursor(0, 1);
      lcd.print("Alt: ");
      if (pressureHpa > 0) {
        lcd.print(altitudeM, 1);
        lcd.print("m");
      } else {
        lcd.print("-- m");
      }
      break;

    case 2:
      // Screen 3: Dew Point & Weather Trend
      lcd.setCursor(0, 0);
      lcd.print("DewPt: ");
      if (dhtAvailable) {
        lcd.print(dewPointC, 1);
        lcd.write(0);
        lcd.print("C");
      } else {
        lcd.print("--");
      }

      lcd.setCursor(0, 1);
      if (weatherTrend.startsWith("RISING")) {
        lcd.print("Forecast: FAIR");
      } else if (weatherTrend.startsWith("FALLING")) {
        lcd.print("Forecast: RAIN");
      } else {
        lcd.print("Trend: STEADY");
      }
      break;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Read sensors every 2 seconds
  if (currentMillis - lastSensorReadTime >= SENSOR_INTERVAL) {
    lastSensorReadTime = currentMillis;
    readSensors();
    printSerialDashboard();
  }

  // Switch LCD Screen mode every 3 seconds
  if (currentMillis - lastLcdSwitchTime >= LCD_SWITCH_INTERVAL) {
    lastLcdSwitchTime = currentMillis;
    updateLcdDisplay();
    lcdScreenIndex = (lcdScreenIndex + 1) % 3;
  }
}
