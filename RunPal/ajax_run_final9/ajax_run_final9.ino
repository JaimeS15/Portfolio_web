/*
 * AJAX - Running Cat Companion
 * Waveshare 1.51" Transparent OLED (HORIZONTAL) + Arduino Nano 33 BLE Sense Rev2
 * with Edge Impulse TFLite keyword spotting
 *
 * WIRING:
 * VCC → 3.3V   GND → GND
 * CLK → D13    DIN → D11
 * CS  → D10    RST → D8
 * DC  → D7
 * BUTTON → D2 (other leg to GND)
 *
 * Canvas: 128px wide × 64px tall (horizontal)
 *
 * HOW TO USE:
 *   Press button once   → wake up / power on
 *   Say "Hey Ajax"      → 5 second countdown → run timer starts
 *   Say "stop run"      → timer freezes → summary screen
 *   Hold button 2 sec   → sleep (low power off)
 */

#define EIDSP_QUANTIZE_FILTERBANK 0

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <PDM.h>
#include <speech_recognition_test_inferencing.h>

// ── Display ───────────────────────────────────────────────────────────────────
#define OLED_CS 10
#define OLED_DC 7
#define OLED_RST 8

// R0 = horizontal, 128px wide × 64px tall
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);
#define W 128
#define H 64

// ── Edge Impulse audio buffer ─────────────────────────────────────────────────
typedef struct {
  int16_t *buffer;
  uint8_t buf_ready;
  uint32_t buf_count;
  uint32_t n_samples;
} inference_t;

static inference_t inference;
static signed short sampleBuffer[2048];

// ── Thresholds ────────────────────────────────────────────────────────────────
#define HEY_AJAX_THRESHOLD 0.80f
#define STOP_RUN_THRESHOLD 0.80f

// ── Buzzer ────────────────────────────────────────────────────────────────────
#define BUZZER_PIN 6

// ── Button ────────────────────────────────────────────────────────────────────
#define BUTTON_PIN 2  // tactile button — other leg to GND

// tone() works on Nano 33 BLE via mbed — plays frequency for duration ms
void beep(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

// Three rising beeps — classic "ready set go" feel
void playStartSound() {
  beep(440, 100);  // A4 — short low
  delay(80);
  beep(523, 100);  // C5 — mid
  delay(80);
  beep(659, 200);  // E5 — high, longer = GO!
}

// Two short descending beeps — run ended
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
unsigned long finalRunTime = 0;  // frozen the instant stop fires
unsigned long stateEnteredAt = 0;

// ── Cat animation ─────────────────────────────────────────────────────────────
int frame = 0;
int runFrame = 0;
int blinkTimer = 0;
int tailPhase = 0;
bool tailDir = true;
int tailTimer = 0;
int bounceY = 0;

// Cat sits on the left half of the horizontal canvas
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
void formatTime(unsigned long ms, char *buf) {
  unsigned long s = ms / 1000;
  sprintf(buf, "%02lu:%02lu", s / 60, s % 60);
}

// ── PIXEL ART CAT ─────────────────────────────────────────────────────────────
// Style: chunky 8-bit pixel art — wide low body, big square head,
//        stubby block legs, small triangular ears, dot eyes
//        All shapes are filled rectangles (drawBox) for true pixel-art look
//
// Pixel grid: each "pixel" = 2x2 block for visibility on small OLED
// Helper: draw one fat pixel
void px(int x, int y) {
  u8g2.drawBox(x * 2, y * 2, 2, 2);
}
// Draw a block of fat pixels  w/h in fat-pixel units
void pxBox(int x, int y, int w, int h) {
  u8g2.drawBox(x * 2, y * 2, w * 2, h * 2);
}

/*
 * Sitting/idle pixel cat — drawn on a 16×20 fat-pixel grid
 * Matches reference: wide rectangular body, big square head,
 * two small ear triangles, dot eyes, whisker lines, stubby feet,
 * tail curling to the right.
 *
 * eyeState: 0=open(dots) 1=half(lines) 2=closed(lines) 3=happy(^)
 * tailOff: -1/0/+1 tail sway direction
 */
void drawSittingCat(int eyeState, int earState, int tailOff) {
  // Origin mapped so cat fits in left panel (CAT_X, CAT_Y)
  // Fat pixel (0,0) maps to real pixel (CAT_X, CAT_Y)
  // Total size: 16 fat px wide × 20 fat px tall = 32×40 real px

  int ox = CAT_X;      // real pixel origin x
  int oy = CAT_Y - 8;  // shifted up to use more vertical space

  // ── Ears (small triangles, 2 fat px wide) ──
  // Left ear: col 3-4, rows 0-1
  u8g2.drawBox(ox + 3 * 2, oy + 0 * 2, 2 * 2, 1 * 2);  // base
  u8g2.drawBox(ox + 3 * 2, oy + 0 * 2, 1 * 2, 2 * 2);  // left side only (triangle)
  // Right ear: col 9-10
  u8g2.drawBox(ox + 9 * 2, oy + 0 * 2, 2 * 2, 1 * 2);
  u8g2.drawBox(ox + 10 * 2, oy + 0 * 2, 1 * 2, 2 * 2);
  // Inner ear marks
  if (earState == 1) {
    u8g2.drawBox(ox + 3 * 2 + 1, oy + 0 * 2 + 1, 2, 2);
    u8g2.drawBox(ox + 10 * 2 + 1, oy + 0 * 2 + 1, 2, 2);
  }

  // ── Big square head: cols 2-12, rows 1-7 ──
  u8g2.drawBox(ox + 2 * 2, oy + 1 * 2, 10 * 2, 7 * 2);

  // ── Eyes ──
  int elx = ox + 4 * 2, erx = ox + 9 * 2, ey = oy + 3 * 2;
  switch (eyeState) {
    case 0:  // open — 2x2 dot eyes
      u8g2.drawBox(elx, ey, 4, 4);
      u8g2.drawBox(erx, ey, 4, 4);
      break;
    case 1:  // half — horizontal bar
      u8g2.drawBox(elx, ey + 2, 4, 2);
      u8g2.drawBox(erx, ey + 2, 4, 2);
      break;
    case 2:  // closed — thin line
      u8g2.drawBox(elx, ey + 3, 4, 1);
      u8g2.drawBox(erx, ey + 3, 4, 1);
      break;
    case 3:  // happy ^ shape
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

  // ── Nose — 2 pixel wide T shape ──
  u8g2.drawBox(ox + 6 * 2 + 1, oy + 5 * 2, 4, 2);      // top of nose
  u8g2.drawBox(ox + 6 * 2 + 2, oy + 5 * 2 + 2, 2, 2);  // bottom dot

  // ── Whiskers — single pixel lines ──
  // Left whiskers
  u8g2.drawHLine(ox, oy + 5 * 2 + 1, 8);
  u8g2.drawHLine(ox, oy + 6 * 2 + 1, 8);
  // Right whiskers
  u8g2.drawHLine(ox + 10 * 2, oy + 5 * 2 + 1, 8);
  u8g2.drawHLine(ox + 10 * 2, oy + 6 * 2 + 1, 8);

  // ── Body: wide rectangle cols 1-13, rows 8-13 ──
  u8g2.drawBox(ox + 1 * 2, oy + 8 * 2, 12 * 2, 5 * 2);

  // ── Stubby feet: 3 fat px wide, 2 fat px tall ──
  u8g2.drawBox(ox + 2 * 2, oy + 13 * 2, 3 * 2, 2 * 2);
  u8g2.drawBox(ox + 6 * 2, oy + 13 * 2, 3 * 2, 2 * 2);
  u8g2.drawBox(ox + 10 * 2, oy + 13 * 2, 3 * 2, 2 * 2);

  // ── Tail: pixel steps curling right, sways with tailOff ──
  int tx = ox + 14 * 2;
  int ty = oy + (9 + tailOff) * 2;
  u8g2.drawBox(tx, ty, 4, 4);           // base segment
  u8g2.drawBox(tx + 4, ty - 2, 4, 4);   // up-right
  u8g2.drawBox(tx + 8, ty - 4, 4, 4);   // up-right
  u8g2.drawBox(tx + 8, ty - 8, 4, 4);   // up
  u8g2.drawBox(tx + 4, ty - 10, 4, 4);  // left-up (curl back)
  u8g2.drawBox(tx, ty - 8, 4, 4);       // tip
}

/*
 * Running pixel cat — 4 frame cycle
 * Body stays wide/low (like reference). Head on right side.
 * Legs are simple pixel blocks that alternate positions.
 * Tail streams back. Tongue pops on airborne frames.
 *
 * Frame 0: stride A  — front legs extended
 * Frame 1: airborne  — legs tucked, tongue out
 * Frame 2: stride B  — opposite extension
 * Frame 3: landing   — legs compressed, dust puffs
 */
void drawRunningCat(int rf, int bY) {
  int ox = CAT_X + 2;
  int oy = CAT_Y - 6 + bY;

  bool tongue = (rf == 1);
  bool tucked = (rf == 1);
  bool landing = (rf == 3);
  bool strideA = (rf == 0);

  // ── Tail streams behind — pixel staircase ──
  int twag = (rf < 2) ? -2 : 2;
  int tx = ox, ty = oy + 8 * 2 + twag * 2;
  u8g2.drawBox(tx - 4, ty, 4, 4);
  u8g2.drawBox(tx - 8, ty - 4, 4, 4);
  u8g2.drawBox(tx - 10, ty - 8, 4, 4);
  u8g2.drawBox(tx - 10, ty - 12, 4, 4);  // tip curls up

  // ── Body — wide horizontal block ──
  u8g2.drawBox(ox + 1 * 2, oy + 7 * 2, 14 * 2, 5 * 2);

  // ── Head on right side — square, slightly raised ──
  int hx = ox + 12 * 2;
  int hy = oy + 1 * 2;
  u8g2.drawBox(hx, hy, 9 * 2, 8 * 2);

  // ── Ears — small pixel triangles on top of head ──
  u8g2.drawBox(hx + 1 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
  u8g2.drawBox(hx + 5 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
  if (rf % 2 == 0) {  // ears lay back on even frames (wind effect)
    u8g2.drawBox(hx + 0 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
    u8g2.drawBox(hx + 5 * 2, hy - 1 * 2, 2 * 2, 1 * 2);
  }

  // ── Eyes — squint on landing, open on stride ──
  int eyY = hy + 2 * 2;
  if (landing) {
    // Squint — thin lines
    u8g2.drawBox(hx + 1 * 2, eyY + 2, 4, 1);
    u8g2.drawBox(hx + 5 * 2, eyY + 2, 4, 1);
  } else {
    // Open dot eyes
    u8g2.drawBox(hx + 1 * 2, eyY, 4, 4);
    u8g2.drawBox(hx + 5 * 2, eyY, 4, 4);
  }

  // ── Nose ──
  u8g2.drawBox(hx + 3 * 2 + 1, hy + 4 * 2, 4, 2);

  // ── Whiskers (swept back — running into wind) ──
  u8g2.drawHLine(hx, hy + 4 * 2 + 1, 6);
  u8g2.drawHLine(hx, hy + 5 * 2 + 1, 6);

  // ── Tongue on airborne frame ──
  if (tongue) {
    u8g2.drawBox(hx + 3 * 2, hy + 6 * 2, 4, 4);  // pixel tongue block
  }

  // ── Legs — pixel blocks, 4 positions ──
  if (tucked) {
    // All legs pulled up under body
    u8g2.drawBox(ox + 3 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 7 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 10 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 13 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
  } else if (strideA) {
    // Front right extended forward, rear left extended back
    u8g2.drawBox(ox + 14 * 2, oy + 12 * 2, 3 * 2, 3 * 2);  // front right — forward
    u8g2.drawBox(ox + 11 * 2, oy + 12 * 2, 3 * 2, 2 * 2);  // front left  — mid
    u8g2.drawBox(ox + 5 * 2, oy + 12 * 2, 3 * 2, 2 * 2);   // rear right  — mid
    u8g2.drawBox(ox + 1 * 2, oy + 12 * 2, 3 * 2, 3 * 2);   // rear left   — back
  } else if (rf == 2) {
    // Opposite stride
    u8g2.drawBox(ox + 14 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
    u8g2.drawBox(ox + 10 * 2, oy + 12 * 2, 3 * 2, 3 * 2);
    u8g2.drawBox(ox + 6 * 2, oy + 12 * 2, 3 * 2, 3 * 2);
    u8g2.drawBox(ox + 2 * 2, oy + 12 * 2, 3 * 2, 2 * 2);
  } else {
    // Landing — all down, wider pads
    u8g2.drawBox(ox + 12 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
    u8g2.drawBox(ox + 8 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
    u8g2.drawBox(ox + 4 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
    u8g2.drawBox(ox + 0 * 2, oy + 12 * 2, 4 * 2, 3 * 2);
  }

  // ── Pixel dust puffs on landing ──
  if (landing) {
    u8g2.drawBox(ox + 2 * 2, oy + 15 * 2 + 2, 2, 2);
    u8g2.drawBox(ox + 5 * 2, oy + 15 * 2 + 4, 2, 2);
    u8g2.drawBox(ox + 9 * 2, oy + 15 * 2 + 4, 2, 2);
    u8g2.drawBox(ox + 12 * 2, oy + 15 * 2 + 2, 2, 2);
  }
}

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

  // Draw base sitting cat with open eyes
  drawSittingCat(0, 0, tailPhase);

  // Override mouth area — open panting mouth + tongue block
  u8g2.drawBox(ox + 5 * 2, oy + 6 * 2, 4 * 2, 2 * 2);  // open mouth (dark box — invert trick: just outline)
  u8g2.drawFrame(ox + 5 * 2, oy + 6 * 2, 4 * 2, 2 * 2);
  u8g2.drawBox(ox + 6 * 2, oy + 7 * 2, 2 * 2, 2 * 2);  // tongue

  // Sweat drops (alternating)
  if ((frame / 4) % 2 == 0) {
    u8g2.drawBox(ox + 14 * 2, oy + 1 * 2, 2, 4);
    u8g2.drawBox(ox + 15 * 2, oy + 3 * 2, 2, 4);
  }
}

void drawSpeedLines() {
  // Pixel-art style speed lines — short horizontal dashes scrolling right to left
  int o1 = (frame * 5) % 82;
  int o2 = (frame * 3) % 82;
  int o3 = (frame * 7) % 82;
  for (int i = 0; i < 4; i++) {
    int lx, ly;
    // Layer 1 — fast, short (3px)
    lx = 46 + (o1 + i * 20) % 78;
    ly = 4 + i * 5;
    if (lx < W - 2) u8g2.drawBox(lx, ly, 4, 1);
    // Layer 2 — medium (6px)
    lx = 46 + (o2 + i * 18) % 78;
    ly = 22 + i * 5;
    if (lx < W - 4) u8g2.drawBox(lx, ly, 6, 1);
    // Layer 3 — slow, long (10px)
    lx = 46 + (o3 + i * 24) % 78;
    ly = 40 + i * 4;
    if (lx < W - 6) u8g2.drawBox(lx, ly, 10, 1);
  }
}

// ── Screen renderers ──────────────────────────────────────────────────────────

void renderIdle() {
  tailTimer++;
  if (tailTimer > 18) {
    tailTimer = 0;
    tailPhase += tailDir ? 1 : -1;
    if (tailPhase >= 3) tailDir = false;
    if (tailPhase <= -3) tailDir = true;
  }
  blinkTimer++;
  int eyeS = 0;
  if (blinkTimer > 80) {
    int p = blinkTimer - 80;
    if (p < 3) eyeS = 1;        // half
    else if (p < 6) eyeS = 2;   // closed
    else if (p < 9) eyeS = 1;   // half
    else if (p < 12) eyeS = 3;  // happy ^ on reopen
    else {
      eyeS = 0;
      blinkTimer = 0;
    }
  }

  drawSittingCat(eyeS, 0, tailPhase);
  u8g2.drawVLine(44, 0, H);

  u8g2.setFont(u8g2_font_7x13B_tr);
  u8g2.drawStr(50, 14, "AJAX");
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(50, 28, "Say:");
  u8g2.drawStr(50, 38, "HEY AJAX");
  u8g2.drawStr(50, 48, "START A RUN");
  u8g2.drawStr(50, 58, "to begin!");
}

void renderCountdown() {
  unsigned long elapsed = millis() - countdownStart;
  int secondsLeft = 5 - (int)(elapsed / 1000);
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

  // Pixel-art bounce — sharper, more game-like
  bounceY = (runFrame == 1) ? -4 : (runFrame == 3) ? 2
                                                   : 0;

  drawSpeedLines();
  drawRunningCat(runFrame, bounceY);

  // Timer bar pinned to bottom
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
  formatTime(finalRunTime, tbuf);  // frozen time
  u8g2.setFont(u8g2_font_8x13B_tr);
  u8g2.drawStr(48, 42, tbuf);
  u8g2.drawHLine(46, 46, W - 46);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(48, 57, "Good Job!!");
}

// ── Edge Impulse PDM ──────────────────────────────────────────────────────────

static void pdm_data_ready_inference_callback(void) {
  int bytesAvailable = PDM.available();
  int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);
  if (inference.buf_ready == 0) {
    for (int i = 0; i < bytesRead >> 1; i++) {
      inference.buffer[inference.buf_count++] = sampleBuffer[i];
      if (inference.buf_count >= inference.n_samples) {
        inference.buf_count = 0;
        inference.buf_ready = 1;
        break;
      }
    }
  }
}

static bool microphone_inference_start(uint32_t n_samples) {
  inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));
  if (inference.buffer == NULL) return false;
  inference.buf_count = 0;
  inference.n_samples = n_samples;
  inference.buf_ready = 0;
  PDM.onReceive(&pdm_data_ready_inference_callback);
  PDM.setBufferSize(4096);
  if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
    ei_printf("Failed to start PDM!\n");
    free(inference.buffer);
    return false;
  }
  PDM.setGain(127);
  return true;
}

static bool microphone_inference_record(void) {
  inference.buf_ready = 0;
  inference.buf_count = 0;
  while (inference.buf_ready == 0) { delay(10); }
  return true;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
  numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);
  return 0;
}

// ── TFLite inference ──────────────────────────────────────────────────────────

void runInference() {
  if (!microphone_inference_record()) return;

  signal_t signal;
  signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
  signal.get_data = &microphone_audio_signal_get_data;

  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR r = run_classifier(&signal, &result, false);
  if (r != EI_IMPULSE_OK) return;

  float hey_ajax_score = 0.0f;
  float stop_run_score = 0.0f;

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if (strcmp(result.classification[ix].label, "hey_ajax") == 0)
      hey_ajax_score = result.classification[ix].value;
    if (strcmp(result.classification[ix].label, "stop_run") == 0)
      stop_run_score = result.classification[ix].value;
  }

  Serial.print(F("hey_ajax: "));
  Serial.println(hey_ajax_score);
  Serial.print(F("stop_run: "));
  Serial.println(stop_run_score);

  // Start
  if (hey_ajax_score >= HEY_AJAX_THRESHOLD && appState == IDLE) {
    Serial.println(F("Start detected → COUNTDOWN"));
    appState = COUNTDOWN;
    countdownStart = millis();
    stateEnteredAt = millis();
  }

  // Stop — freeze timer at this exact moment
  if (stop_run_score >= STOP_RUN_THRESHOLD && appState == RUNNING) {
    Serial.println(F("Stop detected → STOPPING"));
    finalRunTime = runElapsedMs;  // lock the time right here
    appState = STOPPING;
    stateEnteredAt = millis();
    playStopSound();  // 🎵 two descending beeps
  }
}

// ── State transitions ─────────────────────────────────────────────────────────

void updateAppState() {
  unsigned long now = millis();

  switch (appState) {
    case COUNTDOWN:
      if (now - countdownStart >= 6000) {
        appState = RUNNING;
        runStart = now;
        runElapsedMs = 0;
        stateEnteredAt = now;
        Serial.println(F("RUNNING"));
        playStartSound();  // 🎵 three rising beeps on GO!
      }
      break;

    case RUNNING:
      runElapsedMs = now - runStart;
      // Button press manually stops the run
      if (digitalRead(BUTTON_PIN) == LOW) {
        finalRunTime = runElapsedMs;
        appState = STOPPING;
        stateEnteredAt = now;
        playStopSound();
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

    default: break;
  }
}

// ── Button state ──────────────────────────────────────────────────────────────
unsigned long buttonPressedAt = 0;
bool buttonHeld = false;

// Turn off display and wait for button press to wake
void goToSleep() {}

// ── Setup / Loop ──────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // button: LOW = pressed

  u8g2.begin();
  u8g2.setContrast(110);

  if (!microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT)) {
    Serial.println(F("Failed to allocate audio buffer!"));
    while (true)
      ;
  }

  appState = IDLE;
  stateEnteredAt = millis();
  Serial.println(F("Ajax ready! Say 'Hey Ajax start a run'"));
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

  // Inference runs during IDLE and RUNNING only
  // Countdown/stopping/summary use delay(40) for smooth animation
  if (appState == IDLE || appState == RUNNING) {
    runInference();
  } else {
    delay(40);
  }
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif

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
 *   Edge Impulse Arduino library
 *           https://www.edgeimpulse.com
 *
 * Hardware:
 *   Arduino Nano 33 BLE Sense Rev2
 *   Waveshare 1.51" Transparent OLED (SSD1309)
 *   Passive buzzer
 *   6x6mm tactile push button
 *   Adafruit 3.7V 500mAh LiPo battery
 *   TP4056 USB charging module
 * ─────────────────────────────────────────────────────────────
 */