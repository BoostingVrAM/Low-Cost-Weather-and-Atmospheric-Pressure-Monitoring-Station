#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>

/* ====================================================================
 * Project: Low-Cost Weather & Atmospheric Pressure Monitoring Station
 * Platform: Arduino Uno / Wokwi Simulator
 * Sensors:
 *   - BMP280: Barometric Pressure & Temperature (I2C: A4=SDA, A5=SCL)
 *   - DHT22: Humidity & Temperature (Digital Pin 2)
 * Display:
 *   - LCD1602 with I2C (Address: 0x27)
 *   - Serial Monitor (9600 baud)
 * ==================================================================== */

#define DHTPIN 2
#define DHTTYPE DHT22
#define SEALEVELPRESSURE_HPA (1013.25)

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
LiquidCrystal_I2C lcd(0x27, 16, 2);

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

unsigned long lastSensorTime = 0;
const unsigned long SENSOR_INTERVAL = 2000;

unsigned long lastLcdTime = 0;
const unsigned long LCD_INTERVAL = 3000;
int lcdPage = 0;

float tempC = 0.0;
float hum = 0.0;
float pressureHpa = 0.0;
float altitudeM = 0.0;
float dewPointC = 0.0;
bool bmpOk = false;
bool dhtOk = false;
float lastPressure = 0.0;
String trend = "STEADY";

float calcDewPoint(float t, float h) {
  float a = 17.27;
  float b = 237.7;
  float alpha = ((a * t) / (b + t)) + log(h / 100.0);
  return (b * alpha) / (a - alpha);
}

void setup() {
  Serial.begin(9600);
  delay(200);

  Serial.println(F("=================================================="));
  Serial.println(F(" Arduino Uno Low-Cost Weather & Pressure Station  "));
  Serial.println(F("=================================================="));

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeChar);

  lcd.setCursor(0, 0);
  lcd.print("Weather Station");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  dht.begin();
  delay(1000);

  if (bmp.begin(0x76) || bmp.begin(0x77)) {
    bmpOk = true;
    Serial.println(F("[OK] BMP280 connected."));
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
  } else {
    Serial.println(F("[WARN] BMP280 not found!"));
  }

  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorTime >= SENSOR_INTERVAL) {
    lastSensorTime = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      hum = h;
      tempC = t;
      dhtOk = true;
      dewPointC = calcDewPoint(tempC, hum);
    } else {
      dhtOk = false;
    }

    if (bmpOk) {
      pressureHpa = bmp.readPressure() / 100.0F;
      altitudeM = bmp.readAltitude(SEALEVELPRESSURE_HPA);
      if (!dhtOk) tempC = bmp.readTemperature();

      if (lastPressure > 0) {
        float diff = pressureHpa - lastPressure;
        if (diff > 0.5) trend = "RISING";
        else if (diff < -0.5) trend = "FALLING";
        else trend = "STEADY";
      }
      lastPressure = pressureHpa;
    }

    // Print to Serial Monitor
    Serial.println(F("-----------------------------------"));
    Serial.print(F("Temperature: ")); Serial.print(tempC, 1); Serial.println(F(" *C"));
    Serial.print(F("Humidity:    ")); Serial.print(hum, 1); Serial.println(F(" %"));
    Serial.print(F("Pressure:    ")); Serial.print(pressureHpa, 2); Serial.println(F(" hPa"));
    Serial.print(F("Altitude:    ")); Serial.print(altitudeM, 1); Serial.println(F(" m"));
    Serial.print(F("Dew Point:   ")); Serial.print(dewPointC, 1); Serial.println(F(" *C"));
    Serial.print(F("Trend:       ")); Serial.println(trend);
  }

  if (now - lastLcdTime >= LCD_INTERVAL) {
    lastLcdTime = now;
    lcd.clear();

    if (lcdPage == 0) {
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(tempC, 1);
      lcd.write(0);
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Humidity: ");
      lcd.print(hum, 0);
      lcd.print("%");
      lcdPage = 1;
    } else if (lcdPage == 1) {
      lcd.setCursor(0, 0);
      lcd.print("Pres: ");
      lcd.print(pressureHpa, 1);
      lcd.print("hPa");

      lcd.setCursor(0, 1);
      lcd.print("Alt: ");
      lcd.print(altitudeM, 1);
      lcd.print("m");
      lcdPage = 2;
    } else {
      lcd.setCursor(0, 0);
      lcd.print("DewPt: ");
      lcd.print(dewPointC, 1);
      lcd.write(0);
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Trend: ");
      lcd.print(trend);
      lcdPage = 0;
    }
  }
}
