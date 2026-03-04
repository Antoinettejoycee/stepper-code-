#include <Wire.h>
#include "rgb_lcd.h"
#include "Ultrasonic.h"
#include <Stepper.h>
#include <WiFiS3.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// -------- LCD --------
rgb_lcd lcd;

// -------- ULTRASONIC --------
Ultrasonic ultrasonic(7); // Same pin as original

// -------- STEPPER --------
const int stepsPerRevolution = 200;
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

// -------- BUZZER --------
int buzzerPin = 8; // D8 pin
bool alarmTriggered = false;

// -------- WIFI --------
#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// -------- NTP --------
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC, update every 60s

// -------- ALARM TIME --------
int alarmHour = 14;
int alarmMinute = 31;

// -------- TIMERS --------
unsigned long lastLCDUpdate = 0;
unsigned long lastStepperMove = 0;
unsigned long lastUltrasonicRead = 0;
unsigned long lastBuzzerToggle = 0;
bool buzzerOn = false;
int buzzerCycles = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);

  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  myStepper.setSpeed(100);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  timeClient.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  timeClient.update(); // Update NTP time

  // --- Update LCD every second ---
  if (currentMillis - lastLCDUpdate >= 1000) {
    lastLCDUpdate = currentMillis;

    // Get UTC time
    int hourUTC = timeClient.getHours();
    int minute = timeClient.getMinutes();
    int month = timeClient.getMonth();
    int day = timeClient.getDay();
    int hour = hourUTC;

    // DST adjustment
    if ((month > 3 && month < 10) ||
        (month == 3 && day - timeClient.getDayOfWeek() >= 25) ||
        (month == 10 && day - timeClient.getDayOfWeek() < 25)) {
      hour += 1;
    }
    hour %= 24;

    // Display time
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    if (hour < 10) lcd.print("0");
    lcd.print(hour);
    lcd.print(":");
    if (minute < 10) lcd.print("0");
    lcd.print(minute);
    lcd.print("  ");
  }

  // --- Read ultrasonic every 500ms ---
  if (currentMillis - lastUltrasonicRead >= 500) {
    lastUltrasonicRead = currentMillis;
    long distanceCM = ultrasonic.MeasureInCentimeters();
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distanceCM);
    lcd.print(" cm   ");
    Serial.print("Distance: ");
    Serial.println(distanceCM);
  }

  // --- Move stepper every 200ms ---
  if (currentMillis - lastStepperMove >= 200) {
    lastStepperMove = currentMillis;
    myStepper.step(1); // One step at a time for smooth movement
  }

  // --- Check alarm and handle buzzer ---
  int hourUTC = timeClient.getHours();
  int minute = timeClient.getMinutes();
  int month = timeClient.getMonth();
  int day = timeClient.getDay();
  int hour = hourUTC;

  if ((month > 3 && month < 10) ||
      (month == 3 && day - timeClient.getDayOfWeek() >= 25) ||
      (month == 10 && day - timeClient.getDayOfWeek() < 25)) {
    hour += 1;
  }
  hour %= 24;

  if (hour == alarmHour && minute == alarmMinute && !alarmTriggered) {
    alarmTriggered = true;
    buzzerOn = true;
    buzzerCycles = 0;
    lastBuzzerToggle = currentMillis;
  }

  // Handle buzzer toggling without delay
  if (buzzerOn && currentMillis - lastBuzzerToggle >= 500) {
    lastBuzzerToggle = currentMillis;
    buzzerOn = !buzzerOn;
    digitalWrite(buzzerPin, buzzerOn ? HIGH : LOW);
    if (!buzzerOn) buzzerCycles++;
    if (buzzerCycles >= 5) digitalWrite(buzzerPin, LOW); // Stop after 5 cycles
  }
}