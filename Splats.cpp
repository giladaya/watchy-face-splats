#include "Splats.h"
#include <Fonts/FreeSans12pt7b.h>
#include "fonts/lintsec36pt8b.h"
#include "fonts/lintsec24pt8b.h"

#define PI 3.1415926535897932384626433832795

#define LOW_VOLTAGE_THRESHOLD 3.75

// sync once every 6 hours
#define NTP_SYNC_INTERVAL 6 * 60
RTC_DATA_ATTR int ntpSyncCounter = 0;

Splats::Splats(){} //constructor

#define CLOSE_DROPS 5
#define GLOBAL_DROPS_SMALL 10
#define GLOBAL_DROPS_LARGE 10
#define LONG_RAYS 15
#define SHORT_RAYS 20

#define BG_COLOR GxEPD_WHITE
#define FG_COLOR GxEPD_BLACK

#define D_HEIGHT 200
#define D_WIDTH 200
#define MAIN_SPLAT_RAD 60

void Splats::drawPolarLine(int16_t cx, int16_t cy, int16_t r, float a, int16_t lineWidth, uint16_t color){
  float cosA = cos(a);
  float sinA = sin(a);
  float x2 = cx + cosA * r;
  float y2 = cy + sinA * r;
  
  float w2 = lineWidth / 2;
  
  display.fillTriangle(
    (cx - w2 * sinA), 
    (cy + w2 * cosA), 
    (cx - w2 * sinA) + (r * cosA), 
    (cy + w2 * cosA) + (r * sinA), 
    (cx + w2 * sinA) + (r * cosA), 
    (cy - w2 * cosA) + (r * sinA), 
    color);
    
  display.fillTriangle(
    (cx - w2 * sinA), (cy + w2 * cosA),
    (cx + w2 * sinA) + (r * cosA), (cy - w2 * cosA) + (r * sinA),
    (cx + w2 * sinA), (cy - w2 * cosA),
    color);
}

void Splats::drawRay(int16_t cx, int16_t cy, int16_t minR, int16_t maxR, uint16_t color){
  float r = random(minR, maxR + 1);
  float a = random(720) / 720.0 * 2 * PI;
  float x = cos(a) * r;
  float y = sin(a) * r;
  float lineWidth = random(1, 5);

  drawPolarLine(cx, cy , r, a, lineWidth, color);
  
  if (r > 0.9 * maxR) {
    // Longest rays have gap to drop
    int gap = random(3, 9);
    float ex1 = cos(a) * (r + gap);
    float ey1 = sin(a) * (r + gap);
    display.fillCircle(cx + ex1, cy + ey1, lineWidth * 0.75, color);
    
    float ex2 = cos(a) * (r + gap + lineWidth * 0.75);
    float ey2 = sin(a) * (r + gap + lineWidth * 0.75);
    display.fillCircle(cx + ex2, cy + ey2, lineWidth, color); 
  } else if (r > 0.75 * maxR) {
    // Long rays have a drop in the end
    display.fillCircle(cx + x, cy + y, lineWidth, color);
  }
  
  // Base
  float bx = cos(a) * (minR - 1);
  float by = sin(a) * (minR - 1);
  display.fillCircle(cx + bx, cy + by, lineWidth + 1, color);
}

// Draw a whole "splat"
void Splats::drawSplat(int16_t cx, int16_t cy, int16_t rad, int16_t longRays, int16_t shortRays, int16_t drops, uint16_t color) {
  // main blob
  display.fillCircle(cx, cy, rad, color);
 
  // long rays
  for (int i = 0; i < longRays; i++) {
    drawRay(cx, cy, rad, 1.8 * rad, color);
  }
  
  // short rays
  for (int i = 0; i < shortRays; i++) {
    drawRay(cx, cy, rad, 1.2 * rad, color);
  }

  // close drops
  for (int i = 0; i < drops; i++) {
    float r = random(rad, 2 * rad);
    float a = random(1000) / 1000.0 * 2.0 * PI;
    float cxi = cx + cos(a) * r;
    float cyi = cy + sin(a) * r;
    float lineWidth = random(1, 4);

    display.fillCircle(cxi, cyi, lineWidth + 1, color);
  }
}

// Draw a dripping drop
void Splats::drawDrop(int16_t cx, int16_t cy, int16_t radius, uint16_t color){
  int16_t dist = radius;

  display.fillCircle(cx, cy, radius + 1, color);
  display.fillCircle(cx, cy + dist, radius * 0.8, color);
}

void Splats::drawBatteryIndicator() {
  if (getBatteryVoltage() < LOW_VOLTAGE_THRESHOLD){
    display.drawRect(0, 185, 10, 15, GxEPD_BLACK);
    display.fillRect(2, 183, 6, 2, GxEPD_BLACK);
  }
}

void Splats::doNtpSync() {
  if (ntpSyncCounter >= NTP_SYNC_INTERVAL) {
    syncNTP();
    ntpSyncCounter = 0;
  } else {
    ntpSyncCounter++;
  }
}

void Splats::drawWatchFace() { //override this method to customize how the watch face looks

    int theHour = currentTime.Hour;
    int theMinute = currentTime.Minute;

    // display.setFont(&FreeSans12pt7b);
    display.setFont(&lintsec_24pt_Font);
    display.fillScreen(BG_COLOR);

    // main splat
    drawSplat(D_WIDTH / 2, D_HEIGHT / 2, MAIN_SPLAT_RAD, 20, 30, 5, FG_COLOR);

    float cx;
    float cy;
    cx = D_WIDTH * 0.7;
    cy = D_HEIGHT * 0.7;
    drawSplat(cx, cy, MAIN_SPLAT_RAD / 2.0, 15, 20, 5, FG_COLOR);

    // For text positioning
    int16_t x1, y1;
    uint16_t w, h;
    char buffer[3];

    // Minutes splat
    float mcx;
    float mcy;
    float angle_minuteHand  = 2.0 * PI / 60.0 * (theMinute - 15);
    mcx = D_WIDTH / 2 + MAIN_SPLAT_RAD * cos(angle_minuteHand);
    mcy = D_HEIGHT / 2 + MAIN_SPLAT_RAD * sin(angle_minuteHand);
    drawSplat(mcx, mcy, D_WIDTH * 0.125, 15, 20, 5, BG_COLOR);
    
    // Global drops
    // Small drops
    for (int i = 0; i < GLOBAL_DROPS_SMALL; i++) {
      float lineWidth = random(1, 4);
      float cxi = random(D_WIDTH);
      float cyi = random(D_HEIGHT);
      display.fillCircle(cxi, cyi, lineWidth + 1, FG_COLOR);
    }

    // Larger drops
    for (int i = 0; i < GLOBAL_DROPS_LARGE; i++) {
      float lineWidth = random(2, 6);
      float cxi = random(D_WIDTH);
      float cyi = random(D_HEIGHT);
      drawDrop(cxi, cyi, lineWidth + 1, FG_COLOR);
    }

    // draw minutes with font
    itoa(theMinute, buffer, 10);
    display.setTextColor(FG_COLOR);
    display.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    display.fillCircle(mcx, mcy, D_WIDTH * 0.125, BG_COLOR);
    display.setCursor(mcx - w / 2, mcy + h / 2);
    display.print(buffer);

    // draw hours with font
    cx = D_WIDTH / 2;
    cy = D_HEIGHT / 2;
    itoa(currentTime.Hour, buffer, 10);
    display.setFont(&lintsec_36pt_Font);
    display.setTextColor(BG_COLOR);
    display.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(cx - w / 2, cy + h / 2);
    display.print(buffer);

    // Hollow circles
    for (int i = 0; i < GLOBAL_DROPS_LARGE; i++) {
      float rad = random(2, 6);
      float cxi = random(D_WIDTH);
      float cyi = random(D_HEIGHT);
      display.drawCircle(cxi, cyi, rad, BG_COLOR);
    }

    drawBatteryIndicator();
    doNtpSync();
}
