#include <Arduino.h>
#include "ingress.h"

char* Ingress::toCampName(uint8_t color)
{
  switch (color) {
    case (CAMP_RED): return "Red";
    case (CAMP_GREEN): return "Green";
    case (CAMP_BLUE): return "Blue";
    default: return "None";
  }
}

void Ingress::push(uint8_t pushColor)
{
  if (color != pushColor)
  {
    color = pushColor;
    lastPushTime = millis();
    contested = true;
  }
  else if ((millis() - lastPushTime) < CONTEST_MILLIS) 
  {
    contested = false;
  }
}

bool Ingress::isContested()
{
  if (color == CAMP_NONE) return true;
  
  if ((millis() - lastPushTime) < CONTEST_MILLIS) 
  {
    contested = false;
  }
  return contested;
}

uint8_t Ingress::getCamp()
{
  return color;
}
