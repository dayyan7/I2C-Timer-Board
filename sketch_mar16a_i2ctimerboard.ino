#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// screen dimensions for the 0.96" oled
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// create the display object using i2c
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// gpio pin numbers for every component on the board
#define PIN_POT        1    // potentiometer (minutes)
#define PIN_BTN_SEC    9    // seconds button
#define PIN_BTN_START  10   // start button
#define PIN_BTN_STOP   17   // stop button
#define PIN_LED_GREEN  5    // green led
#define PIN_LED_RED    6    // red led
#define PIN_LED_WHITE1 7    // white led left
#define PIN_LED_WHITE2 8    // white led right
#define PIN_BUZZER     18   // buzzer
#define PIN_SDA        11   // oled data line
#define PIN_SCL        12   // oled clock line

// how long to ignore a button after it gets pressed (prevents false reads)
#define DEBOUNCE_DELAY 200

// timestamps to track when each button was last pressed
unsigned long lastDebounceStart = 0;
unsigned long lastDebounceStop  = 0;
unsigned long lastDebounceSec   = 0;

// the board is either showing the clock (idle) or counting down (running)
enum State { IDLE, RUNNING };
State currentState = IDLE;

// stores whatever time the user has set
int setMinutes = 0;
int setSeconds = 0;

// timer tracking variables
unsigned long timerDuration  = 0;
unsigned long timerStartMs   = 0;
unsigned long remainingTime  = 0;

// the clock has no real time module so it counts up from a chosen start time
unsigned long clockBase = 0;
const int START_HOUR = 12;
const int START_MIN  = 0;
const int START_SEC  = 0;


// figures out the current hour, minute, second, and am/pm based on millis()
void getClockTime(int &h, int &m, int &s, bool &isPM) {

  // total seconds elapsed since boot, offset by the chosen start time
  unsigned long elapsed = (millis() - clockBase) / 1000
                        + START_HOUR * 3600
                        + START_MIN  * 60
                        + START_SEC;

  // break the total seconds down into clock values
  int totalHours = (elapsed / 3600) % 24;
  s    = elapsed % 60;
  m    = (elapsed / 60) % 60;

  // anything 12 or above is pm
  isPM = totalHours >= 12;

  // convert from 24 hour to 12 hour format
  h = totalHours % 12;

  // 0 should display as 12 (noon and midnight)
  if (h == 0) h = 12;
}


// turns an led on for a set amount of time then turns it off
void flashLED(int pin, int durationMs) {
  digitalWrite(pin, HIGH);
  delay(durationMs);
  digitalWrite(pin, LOW);
}


// beeps the buzzer three times when the countdown finishes
void soundBuzzer() {
  for (int i = 0; i < 3; i++) {
    // 1000hz tone for 300ms
    tone(PIN_BUZZER, 1000, 300);
    // short gap between beeps
    delay(400);
  }
  // make sure the buzzer fully stops
  noTone(PIN_BUZZER);
}


// prints a number with a leading zero if it is a single digit (ex: 9 becomes 09)
void printTwoDigit(int val) {
  if (val < 10) display.print("0");
  display.print(val);
}


// draws the idle clock screen with the set time preview at the bottom
void displayClock() {
  int h, m, s;
  bool isPM;

  // get the current clock values
  getClockTime(h, m, s, isPM);

  display.clearDisplay();

  // draw the clock in large text at the top
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 8);
  printTwoDigit(h);
  display.print(":");
  printTwoDigit(m);
  display.print(":");
  printTwoDigit(s);

  // add am or pm right after the time
  display.setTextSize(1);
  display.print(isPM ? " PM" : " AM");

  // horizontal divider line across the middle of the screen
  display.drawLine(0, 34, 128, 34, SSD1306_WHITE);

  // show the currently set timer value in the bottom half
  display.setTextSize(1);
  display.setCursor(10, 42);
  display.print("Set: ");
  printTwoDigit(setMinutes);
  display.print("m  ");
  printTwoDigit(setSeconds);
  display.print("s");

  // remind the user how to start
  display.setCursor(10, 54);
  display.print("START to begin");

  // push everything to the screen at once
  display.display();
}


// draws the active countdown screen
void displayTimer() {

  // convert remaining milliseconds into minutes and seconds
  int mins = (remainingTime / 1000) / 60;
  int secs = (remainingTime / 1000) % 60;

  display.clearDisplay();

  // small label at the top
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(40, 4);
  display.print("TIMER");

  // countdown in large text in the center
  display.setTextSize(3);
  display.setCursor(14, 20);
  printTwoDigit(mins);
  display.print(":");
  printTwoDigit(secs);

  // remind the user how to stop
  display.setTextSize(1);
  display.setCursor(28, 56);
  display.print("STOP to cancel");

  display.display();
}


// draws a done screen when the countdown hits zero
void displayDone() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(24, 24);
  display.print("DONE!");
  display.display();
}


void setup() {

  // buttons read incoming signals
  pinMode(PIN_BTN_SEC,    INPUT_PULLUP);
  pinMode(PIN_BTN_START,  INPUT_PULLUP);
  pinMode(PIN_BTN_STOP,   INPUT_PULLUP);

  // leds and buzzer send outgoing signals
  pinMode(PIN_LED_GREEN,  OUTPUT);
  pinMode(PIN_LED_RED,    OUTPUT);
  pinMode(PIN_LED_WHITE1, OUTPUT);
  pinMode(PIN_LED_WHITE2, OUTPUT);
  pinMode(PIN_BUZZER,     OUTPUT);

  // white leds turn on immediately and stay on as long as the board is powered
  digitalWrite(PIN_LED_WHITE1, HIGH);
  digitalWrite(PIN_LED_WHITE2, HIGH);

  // tell the esp32 which pins to use for i2c
  Wire.begin(PIN_SDA, PIN_SCL);

  // try address 0x3C first, then 0x3D (oleds ship with either one)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {

      // if neither address works, blink the white leds as an error signal
      while (true) {
        digitalWrite(PIN_LED_WHITE1, HIGH);
        digitalWrite(PIN_LED_WHITE2, HIGH);
        delay(300);
        digitalWrite(PIN_LED_WHITE1, LOW);
        digitalWrite(PIN_LED_WHITE2, LOW);
        delay(300);
      }
    }
  }

  // clear the screen and record the startup time for the clock
  display.clearDisplay();
  display.display();
  clockBase = millis();
}


void loop() {

  // snapshot the current time once at the top of every loop cycle
  unsigned long now = millis();

  // read the potentiometer and scale it to a minute value between 0 and 99
  int potValue = analogRead(PIN_POT);
  setMinutes   = map(potValue, 0, 4095, 0, 99);

  // seconds button — only works in idle, increments seconds and wraps at 59
  if (digitalRead(PIN_BTN_SEC) == LOW && (now - lastDebounceSec > DEBOUNCE_DELAY)) {
    lastDebounceSec = now;
    if (currentState == IDLE) {
      setSeconds = (setSeconds + 1) % 60;
    }
  }

  // start button — only fires if at least some time has been set
  if (digitalRead(PIN_BTN_START) == LOW && (now - lastDebounceStart > DEBOUNCE_DELAY)) {
    lastDebounceStart = now;
    if (currentState == IDLE && (setMinutes > 0 || setSeconds > 0)) {

      // convert the set time to milliseconds for the countdown
      timerDuration = ((unsigned long)setMinutes * 60 + setSeconds) * 1000;
      timerStartMs  = millis();
      currentState  = RUNNING;

      // brief green flash confirms the timer started
      flashLED(PIN_LED_GREEN, 200);
    }
  }

  // stop button — cancels the countdown and returns to idle
  if (digitalRead(PIN_BTN_STOP) == LOW && (now - lastDebounceStop > DEBOUNCE_DELAY)) {
    lastDebounceStop = now;
    if (currentState == RUNNING) {
      currentState = IDLE;

      // brief red flash confirms the timer stopped
      flashLED(PIN_LED_RED, 200);
    }
  }

  // if a timer is running, update the countdown display every loop cycle
  if (currentState == RUNNING) {
    unsigned long elapsed = millis() - timerStartMs;

    // countdown finished
    if (elapsed >= timerDuration) {
      currentState = IDLE;
      displayDone();
      soundBuzzer();

      // hold the done screen for 2 seconds before returning to the clock
      delay(2000);

    } else {
      // update remaining time and refresh the display
      remainingTime = timerDuration - elapsed;
      displayTimer();
    }

  } else {
    // nothing running, show the clock
    displayClock();
  }
}
