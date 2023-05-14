#include "Arduino.h" 
#include "SmoothPWM.h"

void SmoothPWM::attach(int p, int pos)
{
  pin = p;
  analogWrite(pin, pos);

  currStep = 0;
  numSteps = 0;
}

void SmoothPWM::ramp(int from, int to, int steps)
{
  numSteps = steps;
  step = ((float)to - (float)from) / (float)steps;

  currPWM = (float)from;
  currStep = 0;
  
  analogWrite(pin, currPWM);
}

void SmoothPWM::stop()
{
  currPWM = 0;
  currStep = 0;
  
  analogWrite(pin, currPWM);
}

bool SmoothPWM::process()
{
  if (currStep >= numSteps)
      return true;
  
  currPWM += step;
  //Serial.println(currPWM);
  analogWrite(pin, currPWM);
    
  currStep ++;
  return (currStep >= numSteps);
}
