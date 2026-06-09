/*
 * AJAX - Running Cat Companion (Arduino Uno version)
 * Waveshare 1.51" Transparent OLED + Arduino Uno
 *
 * ⚠️  IMPORTANT — VOLTAGE WARNING:
 * The Uno runs at 5V but the display is 3.3V.
 * You MUST use a logic level converter on the SPI lines
 * (CLK, DIN, CS, RST, DC) or you will damage the display.
 * Only VCC gets 3.3V — use the Uno's 3.3V pin for that.
 *
 * WIRING (via logic level converter on all signal lines):
 * Display VCC → Uno 3.3V  (direct, no converter needed)
 * Display GND → Uno GND
 * Display CLK → LLC → D13
 * Display DIN → LLC → D11
 * Display CS  → LLC → D10
 * Display RST → LLC → D8
 * Display DC  → LLC → D7
 *
 * BUTTON wiring (no converter needed):
 * One leg → D2
 * Other leg → GND
 *
 * HOW TO USE:
 *   Press button once from idle  → 5 second countdown → run starts
 *   Press button once during run → run stops → summary screen
 *   Summary screen auto-returns to idle after 10 seconds
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

// ── Display ───────────────────────────────────────────────────────────────────
#define OLED_CS 10
#define OLED_DC 7
#define OLED_RST 8

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);
#define W 128
#define H 64

// ── Button ────────────────────────────────────────────────────────────────────
#define BUTTON_PIN 2

// ── Buzzer ────────────────────────────────────────────────────────────────────
#define BUZZER_PIN 6

void beep(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

void playStartSound() {
  beep(440, 100);  // A4
  delay(80);
  beep(523, 100);  // C5
  delay(80);
  beep(659, 200);  // E5 — GO!
}

void playStopSound() {
  beep(659, 100);
  delay(60);
  beep(440, 200);
}

// ── App states ────────────────────────────────────────────────────────────────
enum AppState { IDLE,
                COUNTDOWN,
                RUNNING,
                STOPPING,
                SUMMARY };
AppState appState = IDLE;

// ── Timers ────────────────────────────────────────────────────────────────────
unsigned long countdownStart = 0;
unsigned long runStart = 0;
unsigned long runElapsedMs = 0;
unsigned long finalRunTime = 0;
unsigned long stateEnteredAt = 0;

// ── Button debounce ───────────────────────────────────────────────────────────
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
#define DEBOUNCE_MS 50

// ── Animation ─────────────────────────────────────────────────────────────────
int frame = 0;
int runFrame = 0;
int blinkTimer = 0;
int tailPhase = 0;
bool tailDir = true;
int tailTimer = 0;
int bounceY = 0;

#define CAT_X 4
#define CAT_Y 14

// ── Helpers ───────────────────────────────────────────────────────────────────
void drawCorner(int x, int y, int s, bool fx, bool fy) {
  int dx = fx ? -1 : 1, dy = fy ? -1 : 1;
  u8g2.drawLine(x, y, x + dx * s, y);
  u8g2.drawLine(x, y, x, y + dy * s);
}
void drawBrackets(int x, int y, int w, int h, int s) {
  drawCorner(x, y, s, false, false);
  drawCorner(x + w - 1, y, s, true, false);
  drawCorner(x, y + h - 1, s, false, true);
  drawCorner(x + w - 1, y + h - 1, s, true, true);
}
void formatTime(unsigned long ms, char* buf) {
  unsigned long s = ms / 1000;
  sprintf(buf, "%02lu:%02lu", s / 60, s % 60);
}

// ── Fat pixel helpers ─────────────────────────────────────────────────────────
void px(int x, int y) {
  u8g2.drawBox(x * 2, y * 2, 2, 2);
}
void pxBox(int x, int y, int w, int h) {
  u8g2.drawBox(x * 2, y * 2, w * 2, h * 2);
}

// ── Sitting cat ───────────────────────────────────────────────────────────────
void drawSittingCat(int eyeState, int earState, int tailOff) {
  int ox = CAT_X;
  int oy = CAT_Y - 8;

  // Ears
  u8g2.drawBox(ox + 3 * 2, oy + 0 * 2, 2 * 2, 1 * 2);
  u8g2.drawBox(ox + 3 * 2, oy + 0 * 2, 1 * 2, 2 * 2);
  u8g2.drawBox(ox + 9 * 2, oy + 0 * 2, 2 * 2, 1 * 2);
  u8g2.drawBox(ox + 10 * 2, oy + 0 * 2, 1 * 2, 2 * 2);
  if (earState == 1) {
    u8g2.drawBox(ox + 3 * 2 + 1, oy + 0 * 2 + 1, 2, 2);
    u8g2.drawBox(ox + 10 * 2 + 1, oy + 0 * 2 + 1, 2, 2);
  }

  // Head
  u8g2.drawBox(ox + 2 * 2, oy + 1 * 2, 10 * 2, 7 * 2);

  // Eyes
  int elx = ox + 4 * 2, erx = ox + 9 * 2, ey = oy + 3 * 2;
  switch (eyeState) {
    case 0:
      u8g2.drawBox(elx, ey, 4, 4);
      u8g2.drawBox(erx, ey, 4, 4);
      break;
    case 1:
      u8g2.drawBox(elx, ey + 2, 4, 2);
      u8g2.drawBox(erx, ey + 2, 4, 2);
      break;
    case 2:
      u8g2.drawBox(elx, ey + 3, 4, 1);
      u8g2.drawBox(erx, ey + 3, 4, 1);
      break;
    case 3:
      u8g2.drawPixel(elx, ey + 3);
      u8g2.drawPixel(elx + 1, ey + 2);
      u8g2.drawPixel(elx + 2, ey + 2);
      u8g2.drawPixel(elx + 3, ey + 3);
      u8g2.drawPixel(erx, ey + 3);
      u8g2.drawPixel(erx + 1, ey + 2);
      u8g2.drawPixel(erx + 2, ey + 2);
      u8g2.drawPixel(erx + 3, ey + 3);
      break;
  }

  // Nose
  u8g2.drawBox(ox + 6 * 2 + 1, oy + 5 * 2, 4, 2);
  u8g2.drawBox(ox + 6 * 2 + 2, oy + 5 * 2 + 2, 2, 2);

  // Whiskers
  u8g2.drawHLine(ox, oy + 5 * 2 + 1, 8);
  u8g2.drawHLine(ox, oy + 6 * 2 + 1, 8);
  u8g2.drawHLine(ox + 10 * 2, oy + 5 * 2 + 1, 8);
  u8g2.drawHLine(ox + 10 * 2, oy + 6 * 2 + 1, 8);

  // Body
  u8g2.drawBox(ox + 1 * 2, oy + 8 * 2, 12 * 2, 5 * 2);

  // Feet
  u8g2.drawBox(ox + 2 * 2, oy + 13 * 2, 3 * 2, 2 * 2);
  u8g2.drawBox(ox + 6 * 2, oy + 13 * 2, 3 * 2, 2 * 2);
  u8g2.drawBox(ox + 10 * 2, oy + 13 * 2, 3 * 2, 2 * 2);

  // Tail
  int tx = ox + 14 * 2;
  int ty = oy + (9 + tailOff) * 2;
  u8g2.drawBox(tx, ty, 4, 4);
  u8g2.drawBox(tx + 4, ty - 2, 4, 4);
  u8g2.drawBox(tx + 8, ty - 4, 4, 4);
  u8g2.drawBox(tx + 8, ty - 8, 4, 4);
  u8g2.drawBox(tx + 4, ty - 10, 4, 4);
  u8g2.drawBox(tx, ty - 8, 4, 4);
}

// ── Running cat ───────────────────────────────────────────────────────────────
void drawRunningCat(int rf, int bY) {
  int ox = CAT_X + 2;
  int oy = CAT_Y - 6 + bY;

  bool tongue = (rf == 1);
  bool tucked = (rf == 1);
  bool landing = (rf == 3);
  bool strideA = (rf == 0);

  // Tail
  int twag = (rf < 2) ? -2 : 2;
  int tx = ox, ty = oy + 8 * 2 + twag * 2;
  u8g2.drawBox(tx - 4, ty, 4, 4);
  u8g2.drawBox(tx - 8, ty - 4, 4, 4);
  u8g2.drawBox(tx - 10, ty - 8, 4, 4);
  u8g2.drawBox(tx - 10, ty - 12, 4, 4);

  // Body
  u8g2.drawBox(ox + 1 * 2, oy + 7 * 2, 14 * 2, 5 * 2);

  // Head
  int hx = ox + 12 * 2;
  int hy = oy + 1 * 2;
  u8g2.drawBox(hx, hy, 9 * 2, 8 * 2);

  // Ears
  u8g2.drawBox(hx + 1 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
  u8g2.drawBox(hx + 5 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
  if (rf % 2 == 0) {
    u8g2.drawBox(hx + 0 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
    u8g2.drawBox(hx + 5 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
  }

  // Eyes
  int eyY = hy + 2 * 2;
  if (landing) {
    u8g2.drawBox(hx + 1 * 2, eyY + 2, 4, 1);
    u8g2.drawBox(hx + 5 * 2, eyY + 2, 4, 1);
  } else {
    u8g2.drawBox(hx + 1 * 2, eyY, 4, 4);
    u8g2.drawBox(hx + 5 * 2, eyY, 4, 4);
  }

  // Nose + whiskers
  u8g2.drawBox(hx + 3 * 2 + 1, hy + 4 * 2, 4, 2);
  u8g2.drawHLine(hx, hy + 4 * 2 + 1, 6);
  u8g2.drawHLine(hx, hy + 5 * 2 + 1, 6);

  // Tongue
  if (tongue) u8g2.drawBox(hx + 3 * 2, hy + 6 * 2, 4, 4);

  // Legs
  if (tucked) {
    u8g2.drawBox(ox + 3 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 7 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 10 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 13 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
  } else if (strideA) {
    u8g2.drawBox(ox + 14 * 2, oy + 12 * 2, 3 * 2, 3 * 2);
    u8g2.drawBox(ox + 11 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 5 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 1 * 2, oy + 12 * 2, 3 * 2, 3 * 2);
  } else if (rf == 2) {
    u8g2.drawBox(ox + 14 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 10 * 2, oy + 12 * 2, 3 * 2, 3 * 2);
    u8g2.drawBox(ox + 6 * 2, oy + 12 * 2, 3 * 2, 3 * 2);
    u8g2.drawBox(ox + 2 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
  } else {
    u8g2.drawBox(ox + 12 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
    u8g2.drawBox(ox + 8 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
    u8g2.drawBox(ox + 4 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
    u8g2.drawBox(ox + 0 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
  }

  // Dust puffs
  if (landing) {
    u8g2.drawBox(ox + 2 * 2, oy + 15 * 2 + 2, 2, 2);
    u8g2.drawBox(ox + 5 * 2, oy + 15 * 2 + 4, 2, 2);
    u8g2.drawBox(ox + 9 * 2, oy + 15 * 2 + 4, 2, 2);
    u8g2.drawBox(ox + 12 * 2, oy + 15 * 2 + 2, 2, 2);
  }
}

// ── Panting cat ───────────────────────────────────────────────────────────────
void drawPantingCat() {
  tailTimer++;
  if (tailTimer > 18) {
    tailTimer = 0;
    tailPhase += tailDir ? 1 : -1;
    if (tailPhase >= 3) tailDir = false;
    if (tailPhase <= -3) tailDir = true;
  }
  int ox = CAT_X;
  int oy = CAT_Y - 8;
  drawSittingCat(0, 0, tailPhase);
  u8g2.drawFrame(ox + 5 * 2, oy + 6 * 2, 4 * 2, 2 * 2);
  u8g2.drawBox(ox + 6 * 2, oy + 7 * 2, 2 * 2, 2 * 2);
  if ((frame / 4) % 2 == 0) {
    u8g2.drawBox(ox + 14 * 2, oy + 1 * 2, 2, 4);
    u8g2.drawBox(ox + 15 * 2, oy + 3 * 2, 2, 4);
  }
}

// ── Speed lines ───────────────────────────────────────────────────────────────
void drawSpeedLines() {
  static int offA = 0, offB = 0, offC = 0;
  offA = (offA + 3) % 40;
  offB = (offB + 5) % 60;
  offC = (offC + 2) % 30;
  for (int i = 0; i < 4; i++) {
    u8g2.drawHLine(W - ((offA + i * 30) % W), 10, 4);
    u8g2.drawHLine(W - ((offB + i * 45) % W), 30, 6);
    u8g2.drawHLine(W - ((offC + i * 20) % W), 50, 10);
  }
}

// ── Render screens ────────────────────────────────────────────────────────────
void renderIdle() {
  blinkTimer++;
  int eyeState = 0;
  if (blinkTimer > 78 && blinkTimer < 82) eyeState = 1;
  else if (blinkTimer > 81 && blinkTimer < 85) eyeState = 2;
  else if (blinkTimer > 84 && blinkTimer < 90) eyeState = 3;
  else if (blinkTimer >= 90) blinkTimer = 0;

  tailTimer++;
  if (tailTimer > 20) {
    tailTimer = 0;
    tailPhase += tailDir ? 1 : -1;
    if (tailPhase >= 2) tailDir = false;
    if (tailPhase <= -2) tailDir = true;
  }

  drawSittingCat(eyeState, 0, tailPhase);
  u8g2.drawVLine(44, 0, H);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(52, 14, "AJAX");
  u8g2.drawHLine(46, 17, W - 46);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(50, 30, "Press button");
  u8g2.drawStr(50, 42, "to start");
  u8g2.drawStr(50, 54, "a run!");
}

void renderCountdown() {
  int secondsLeft = 5 - (int)((millis() - countdownStart) / 1000);
  if (secondsLeft < 0) secondsLeft = 0;

  drawSittingCat(1, 0, 0);
  u8g2.drawVLine(44, 0, H);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(52, 10, "GET READY!");
  u8g2.drawHLine(46, 14, W - 46);

  if (secondsLeft > 0) {
    u8g2.setFont(u8g2_font_logisoso28_tn);
    char nbuf[3];
    sprintf(nbuf, "%d", secondsLeft);
    u8g2.drawStr((secondsLeft == 1) ? 78 : 68, 58, nbuf);
  } else {
    u8g2.setFont(u8g2_font_8x13B_tr);
    u8g2.drawStr(54, 38, "GO!!!");
    u8g2.drawStr(50, 55, "AJAX!!!");
  }
}

void renderRunning() {
  if (frame % 5 == 0) runFrame = (runFrame + 1) % 4;
  bounceY = (runFrame == 1) ? -4 : (runFrame == 3) ? 2
                                                   : 0;
  drawSpeedLines();
  drawRunningCat(runFrame, bounceY);
  u8g2.drawHLine(0, H - 16, W);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(2, H - 6, "TIME");
  u8g2.setFont(u8g2_font_8x13B_tr);
  char tbuf[8];
  formatTime(runElapsedMs, tbuf);
  u8g2.drawStr(28, H - 4, tbuf);
}

void renderStopping() {
  drawPantingCat();
  u8g2.drawVLine(44, 0, H);
  u8g2.setFont(u8g2_font_8x13B_tr);
  u8g2.drawStr(52, 28, "RUN");
  u8g2.drawStr(52, 44, "OVER!");
  if ((frame / 3) % 2 == 0) drawBrackets(46, 2, W - 48, H - 4, 4);
}

void renderSummary() {
  drawPantingCat();
  u8g2.drawVLine(44, 0, H);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(48, 12, "NICE RUN!");
  u8g2.drawHLine(46, 15, W - 46);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(48, 27, "TIME");
  char tbuf[8];
  formatTime(finalRunTime, tbuf);
  u8g2.setFont(u8g2_font_8x13B_tr);
  u8g2.drawStr(48, 42, tbuf);
  u8g2.drawHLine(46, 46, W - 46);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(48, 57, "Good Job!!");
}

// ── Button read with debounce ─────────────────────────────────────────────────
bool buttonJustPressed() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  lastButtonState = reading;
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading == LOW) {
      lastDebounceTime = millis() + 300;  // prevent double-fire
      return true;
    }
  }
  return false;
}

// ── State machine ─────────────────────────────────────────────────────────────
void updateAppState() {
  unsigned long now = millis();
  bool pressed = buttonJustPressed();

  switch (appState) {
    case IDLE:
      if (pressed) {
        appState = COUNTDOWN;
        countdownStart = now;
        stateEnteredAt = now;
      }
      break;

    case COUNTDOWN:
      if (now - countdownStart >= 6000) {
        appState = RUNNING;
        runStart = now;
        runElapsedMs = 0;
        stateEnteredAt = now;
        playStartSound();  // three rising beeps on GO!
      }
      break;

    case RUNNING:
      runElapsedMs = now - runStart;
      if (pressed) {
        finalRunTime = runElapsedMs;
        appState = STOPPING;
        stateEnteredAt = now;
        playStopSound();  // two descending beeps
      }
      break;

    case STOPPING:
      if (now - stateEnteredAt > 1500) {
        appState = SUMMARY;
        stateEnteredAt = now;
      }
      break;

    case SUMMARY:
      if (now - stateEnteredAt > 10000) {
        appState = IDLE;
        stateEnteredAt = now;
      }
      break;
  }
}

// ── Setup / Loop ──────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  u8g2.begin();
  u8g2.setContrast(110);
  appState = IDLE;
  stateEnteredAt = millis();
  Serial.println(F("Ajax ready! Press button to start."));
}

void loop() {
  frame++;
  updateAppState();

  u8g2.clearBuffer();
  switch (appState) {
    case IDLE: renderIdle(); break;
    case COUNTDOWN: renderCountdown(); break;
    case RUNNING: renderRunning(); break;
    case STOPPING: renderStopping(); break;
    case SUMMARY: renderSummary(); break;
  }
  u8g2.sendBuffer();
  delay(20);
}

/*
 * ── Attribution ───────────────────────────────────────────────
 * Project:  AJAX Running Companion
 * Author:   Jaime Serrano
 * Date:     2026
 *
 * Built with assistance from Claude (Anthropic)
 *           https://claude.ai
 *
 * Libraries used:
 *   U8g2  — Oliver Kraus (olikraus)
 *           https://github.com/olikraus/u8g2
 *           Licensed under BSD 2-Clause
 *
 * Hardware:
 *   Arduino Uno
 *   Waveshare 1.51" Transparent OLED (SSD1309)
 *   Passive buzzer
 *   6x6mm tactile push button
 * ─────────────────────────────────────────────────────────────
 */