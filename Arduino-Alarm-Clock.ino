#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// Hardware Pins
const int btnMenu = 8;   // Black wire (Menu)
const int btnUp = 9;     // Red wire (Up)
const int btnDown = 10;  // Green wire (Down)
const int btnToggle = 11;// Purple wire (Alarm Toggle)
const int ledPin = 2;    // Blue LED Alarm

// Variables
int menuMode = 0; // 0 = Main, 1 = Set Hour, 2 = Set Minute
int alarmHour = 12;
int alarmMinute = 0;
bool alarmArmed = false;

// Button States for debounce
bool lastMenu = HIGH;
bool lastUp = HIGH;
bool lastDown = HIGH;
bool lastToggle = HIGH;

void setup() {
  pinMode(btnMenu, INPUT_PULLUP);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  pinMode(btnToggle, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  rtc.begin();
}

void loop() {
  DateTime now = rtc.now();

  // Read buttons
  bool currMenu = digitalRead(btnMenu);
  bool currUp = digitalRead(btnUp);
  bool currDown = digitalRead(btnDown);
  bool currToggle = digitalRead(btnToggle);

  // Menu Button Logic (Switch between screens)
  if (currMenu == LOW && lastMenu == HIGH) {
    menuMode++;
    if (menuMode > 2) menuMode = 0;
    lcd.clear();
  }
  lastMenu = currMenu;

  // Toggle Alarm Button Logic (Only works on Main Screen)
  if (menuMode == 0 && currToggle == LOW && lastToggle == HIGH) {
    alarmArmed = !alarmArmed;
    lcd.clear();
  }
  lastToggle = currToggle;

  // Up/Down Button Logic for Setting the Alarm
  if (menuMode == 1) { // Set Hour
    if (currUp == LOW && lastUp == HIGH) alarmHour = (alarmHour + 1) % 24;
    if (currDown == LOW && lastDown == HIGH) alarmHour = (alarmHour == 0) ? 23 : alarmHour - 1;
  } else if (menuMode == 2) { // Set Minute
    if (currUp == LOW && lastUp == HIGH) alarmMinute = (alarmMinute + 1) % 60;
    if (currDown == LOW && lastDown == HIGH) alarmMinute = (alarmMinute == 0) ? 59 : alarmMinute - 1;
  }
  lastUp = currUp;
  lastDown = currDown;

  // --- Display Logic ---
  if (menuMode == 0) {
    // MAIN CLOCK SCREEN
    lcd.setCursor(1, 0); // Shifted right to prevent cut-off
    lcd.print("Time: ");
    print2Digits(now.hour());
    lcd.print(":");
    print2Digits(now.minute());
    lcd.print(":");
    print2Digits(now.second());

    lcd.setCursor(1, 1);
    if (alarmArmed) {
      lcd.print("Alarm: ");
      print2Digits(alarmHour);
      lcd.print(":");
      print2Digits(alarmMinute);
    } else {
      lcd.print("Alarm: OFF      ");
    }

    // ALARM TRIGGER
    if (alarmArmed && now.hour() == alarmHour && now.minute() == alarmMinute && now.second() < 15) {
      digitalWrite(ledPin, (millis() / 250) % 2); // Flash LED fast
    } else {
      digitalWrite(ledPin, LOW);
    }

  } else if (menuMode == 1) {
    // SET HOUR SCREEN
    lcd.setCursor(1, 0);
    lcd.print("Set Alarm Hour");
    lcd.setCursor(1, 1);
    lcd.print("-> ");
    print2Digits(alarmHour);
    lcd.print("           ");
  } else if (menuMode == 2) {
    // SET MINUTE SCREEN
    lcd.setCursor(1, 0);
    lcd.print("Set Alarm Min ");
    lcd.setCursor(1, 1);
    lcd.print("-> ");
    print2Digits(alarmMinute);
    lcd.print("           ");
  }

  delay(50); // Debounce delay
}

// Formatting tool for double digits
void print2Digits(int number) {
  if (number >= 0 && number < 10) lcd.print('0');
  lcd.print(number);
}