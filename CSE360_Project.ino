#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_TCS34725.h>

const int led = 5; // color sensor led

#define sensorPower 12    // Pin where soil moisture sensor is connected
#define sensorPin A0      // Pin where soil moisture sensor is connected
#define DHTPIN 2          // Pin where the DHT11 is connected
#define DHTTYPE DHT11     // DHT sensor type
#define PIR_PIN 3         // Pin where the PIR sensor is connected
#define smokeAnalogPin A1 // Pin where smoke detector analog output is connected
#define smokeDigitalPin A2 // Pin where smoke detector digital output is connected

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

const int buzzer = 6;
const int motor = 7;
const int trigPin = 9;
const int echoPin = 10;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

long duration;
int distance;
int waterthreshold = 5;

void setup() {
  dht.begin();
  pinMode(7, OUTPUT);   // LED
  pinMode(8, OUTPUT);   // LED
  pinMode(PIR_PIN, INPUT);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW); // Keeping soil moisture sensor OFF initially
  pinMode(buzzer, OUTPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(smokeDigitalPin, INPUT);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Water Level: ");
  pinMode(led, OUTPUT);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  // Read temperature and humidity from DHT11 sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  Serial.print("Red: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("Green: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("Blue: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("Color: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
  analogWrite(led, c);
  delay(100);

  // Ultrasonic
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  lcd.setCursor(0, 1);
  lcd.print("       ");
  lcd.setCursor(0, 1);
  lcd.print(distance);
  lcd.print(" cm");

  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= waterthreshold) {
    digitalWrite(8, HIGH);
    if (distance <= 5) {
      Serial.println("Water level: OK");
    } else {
      Serial.println("Need water");
    }
  } else {
    digitalWrite(8, LOW);
  }

  // Read soil moisture level
  int moistureLevel = readSensor();

  if ((moistureLevel < 2000) && (humidity < 100)) {
    // If moisture level is less than 50, turn on the motor (relay)
    digitalWrite(motor, HIGH); // Turn on the relay (motor)
    delay(5000); // Wait for 5 seconds
    digitalWrite(motor, LOW); // Turn off the relay (motor)
  } else {
    // If moisture level is not less than 50, keep the motor off
    digitalWrite(motor, LOW); // Make sure the relay (motor) is off
    delay(5000); // Wait for 5 seconds before checking again
  }

  // Check for gas/smoke detection
  int smokeAnalogValue = analogRead(smokeAnalogPin);
  int smokeDigitalValue = digitalRead(smokeDigitalPin);

  if (smokeAnalogValue > 5000) {
    Serial.println("Gas detected!");
  } else {
    Serial.println("No gas detected.");
  }

  if (!isnan(humidity) && !isnan(temperature)) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\tHumidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    digitalWrite(7, HIGH);

    int motionDetected = digitalRead(PIR_PIN);

    if (motionDetected == HIGH) {
      Serial.println("Motion Detected!");
      digitalWrite(8, HIGH); // RED LED HIGH
      tone(buzzer, 1000);
      delay(1000);
      noTone(buzzer);
      delay(100);
    } else {
      Serial.println("No motion detected.");
    }
  } else {
    Serial.println("Failed to read from DHT sensor!");
    digitalWrite(6, HIGH);
  }

  // Print moisture level
  Serial.print("Moisture Level: ");
  Serial.println(moistureLevel);
  delay(2000); // Wait for 2 seconds before reading again
}

// This function returns the analog soil moisture measurement
int readSensor() {
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  delay(10);                         // Allow power to settle
  int val = analogRead(sensorPin);   // Read the analog value from sensor
  digitalWrite(sensorPower, LOW);    // Turn the sensor OFF
  return val;                        // Return analog moisture value
}
