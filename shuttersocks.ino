#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22     // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
sensors_event_t event;
sensor_t sensor;

int RELAY_SING_PIN = 3;
float relative_humidity_cutoff = 80;
float atomize_efficiency = 0.6; // increased rh per second
float atomize_sec;
float atomize_sec_max = 10.0;
float atomize_sec_min = 0.5;
float equilibration_minutes = 3.0;

void lcd_print_temp(float temp) {
    lcd.setCursor(0, 0);
    lcd.print(temp, 1);
    lcd.print(char(223));
    lcd.print(F("C"));
}

void lcd_print_rh(float rh) {
    lcd.setCursor(8, 0);
    lcd.print(rh, 1);
    lcd.print(F("% RH"));
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_SING_PIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(RELAY_SING_PIN, HIGH);
  
  Serial.begin(9600);

  dht.begin();
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));

  lcd.init();
  lcd.backlight();
}

void get_sensor_data(DHT_Unified dht, sensors_event_t event) {
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    lcd_print_temp(event.temperature);
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    lcd_print_rh(event.relative_humidity);
  }
}

void atomize(float rh) {
  if (rh < relative_humidity_cutoff) {
    atomize_sec = (relative_humidity_cutoff - rh)/atomize_efficiency;
    if (atomize_sec > atomize_sec_max) {
      atomize_sec = atomize_sec_max;
    }
    else if (atomize_sec < atomize_sec_min) {
      atomize_sec = atomize_sec_min;
    }
    digitalWrite(RELAY_SING_PIN, LOW);
    delay(atomize_sec * 1000);
  }
  digitalWrite(RELAY_SING_PIN, HIGH);
}

void equilibrate(float equilibration_minutes) {
  lcd.setCursor(0, 1);
  lcd.print(F("Waiting 5 min..."));
  delay(equilibration_minutes * 1000 * 60);
}

void loop() {
  get_sensor_data(dht, event);
  atomize(event.relative_humidity);
  equilibrate(equilibration_minutes);
  delay(100);
}
