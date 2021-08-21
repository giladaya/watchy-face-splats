#ifndef WATCHY_Splats_H
#define WATCHY_Splats_H

#include <Watchy.h>

class Splats : public Watchy{
    public:
        Splats();
    void drawPolarLine(int16_t cx, int16_t cy, int16_t r, float a, int16_t lineWidth, uint16_t color);
    void drawRay(int16_t cx, int16_t cy, int16_t minR, int16_t maxR, uint16_t color);
    // Draw a whole "splat"
    void drawSplat(int16_t cx, int16_t cy, int16_t rad, int16_t longRays, int16_t shortRays, int16_t drops, uint16_t color);
    // Draw a dripping drop
    void drawDrop(int16_t cx, int16_t cy, int16_t radius, uint16_t color);
    void drawBatteryIndicator();
    void doNtpSync();
    void drawWatchFace();
};

#endif
