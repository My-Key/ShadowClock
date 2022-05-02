#ifndef BOTWATCHY_H
#define BOTWATCHY_H

#include <Watchy.h>
#include "Vector3.h"
#include "../include/Normal.h"
#include "../include/Diffuse.h"
#include "../include/Gloss.h"
#include "../include/BlueNoise200.h"

class BowlWatchy : public Watchy
{
  using Watchy::Watchy;
public:
  BowlWatchy();
  void drawWatchFace();
  void drawTime();

  int getBatteryFill(int steps);
};

#endif
