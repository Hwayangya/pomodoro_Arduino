#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTTON_SHORT_BREAK 4
#define BUTTON_START_PAUSE 7
#define BUZZER_PIN 2

#define RGB_RED_PIN 9
#define RGB_GREEN_PIN 11
#define RGB_BLUE_PIN 10

LiquidCrystal_I2C lcd(0x27, 16, 2);

void resetTimer(bool fullReset = false);
void displayRandomMessage();
void setColor(int r, int g, int b);

unsigned long previousMillis = 0;
unsigned long interval = 1000;
int countdown = 0;
bool isRunning = false;

int pomodoroCount = 0;
bool isShortBreak = false;
bool pomodoroJustEnded = false;

int colorIndex = 0;

const int colors[6][3] = {
  {255, 0, 0},     // 빨강
  {255, 20, 0},    // 주황
  {255, 100, 0},   // 노랑
  {0, 255, 0},     // 초록
  {0, 0, 255},     // 파랑
  {225, 0, 70},    // 보라
};

void setup() {
  pinMode(BUTTON_SHORT_BREAK, INPUT_PULLUP);
  pinMode(BUTTON_START_PAUSE, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();

  randomSeed(analogRead(A0));
  displayRandomMessage();

  setColor(0, 0, 0);  // 초기 RGB OFF
}

void loop() {
  static bool lastShortBreakState = HIGH;
  static bool lastStartPauseState = HIGH;

  bool shortBreakState = digitalRead(BUTTON_SHORT_BREAK);
  bool startPauseState = digitalRead(BUTTON_START_PAUSE);

  // 두 버튼 동시에 누르면 전체 리셋
  if (shortBreakState == LOW && startPauseState == LOW) {
    resetTimer(true);
    delay(500);
    return;
  }

  // 버튼 1: 포모도로 종료 직후에만 짧은 휴식 시작 (색상 변경 없음)
  if (shortBreakState == LOW && lastShortBreakState == HIGH) {
    if (pomodoroJustEnded) {
      countdown = 300; // 5분 짧은 휴식
      isRunning = true;
      isShortBreak = true;
      pomodoroJustEnded = false;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Short break 5m");
    }
  }
  lastShortBreakState = shortBreakState;

  // 버튼 2: 포모도로 시작 / 일시정지 / 재개
  if (startPauseState == LOW && lastStartPauseState == HIGH) {
    if (!isRunning && countdown == 0) {
      countdown = 1500; // 25분 → 테스트용으로 90초 설정
      isRunning = true;
      isShortBreak = false;
      pomodoroJustEnded = false;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pomodoro Start");
    } else {
      isRunning = !isRunning;
      lcd.setCursor(0, 1);
      lcd.print(isRunning ? "Resumed        " : "Paused         ");
    }
  }
  lastStartPauseState = startPauseState;

  // 타이머 실행
  if (isRunning && millis() - previousMillis >= interval) {
    previousMillis = millis();

    if (countdown > 0) {
      countdown--;

      lcd.setCursor(0, 1);
      lcd.print("Time: ");
      lcd.print(countdown / 60);
      lcd.print(":");
      if ((countdown % 60) < 10) lcd.print("0");
      lcd.print(countdown % 60);
      lcd.print("   ");
    }

    // 타이머 완료
    if (countdown == 0) {
      isRunning = false;

      tone(BUZZER_PIN, 1000);
      delay(500);
      noTone(BUZZER_PIN);

      lcd.clear();
      lcd.setCursor(0, 0);

      if (isShortBreak) {
        lcd.print("Break Done <3");

        // 휴식 끝 → 색상 변경
        setColor(colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]);
        colorIndex = (colorIndex + 1) % 6;

      } else {
        lcd.print("Pomodoro Done!");
        pomodoroCount++;
        lcd.setCursor(0, 1);
        lcd.print("Count: ");
        lcd.print(pomodoroCount);
        setColor(255, 100, 180);
        pomodoroJustEnded = true;
      }
    }
  }
}

void resetTimer(bool fullReset) {
  countdown = 0;
  isRunning = false;
  isShortBreak = false;
  pomodoroJustEnded = false;
  noTone(BUZZER_PIN);
  setColor(0, 0, 0);
  lcd.clear();

  if (fullReset) {
    pomodoroCount = 0;
    colorIndex = 0;
    displayRandomMessage();
  } else {
    lcd.print("Timer reset.");
  }
}

void displayRandomMessage() {
  lcd.setCursor(0, 0);
  switch (random(9)) {
    case 0: lcd.print("Lucky Day :D"); break;
    case 1: lcd.print("tomato study~"); break;
    case 2: lcd.print("sweetie!"); break;
    case 3: lcd.print("matcha time~"); break;
    case 4: lcd.print("hello!"); break;
    case 5: lcd.print("JUST DO IT! >:D"); break;
    case 6: lcd.print("Here we go!"); break;
    case 7: lcd.print("LDS,YJG,CJY,KMS"); break;
    case 8: lcd.print("Howdy! :D"); break;
  }
}

void setColor(int r, int g, int b) {
  analogWrite(RGB_RED_PIN, r);
  analogWrite(RGB_GREEN_PIN, g);
  analogWrite(RGB_BLUE_PIN, b);
}
