#include "Splats.h"

Splats watchy; //instantiate your watchface

void setup() {
  watchy.init(); //call init in setup
  randomSeed(watchy.getBatteryVoltage());
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
