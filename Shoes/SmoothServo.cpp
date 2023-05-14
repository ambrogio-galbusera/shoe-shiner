#include "Arduino.h" 
#include "SmoothServo.h"

void SmoothServo::attach(int pin, int pos)
{
  servo.attach(pin);
  servo.write(pos);

  currStep = 0;
  numSteps = 0;
}

void SmoothServo::move(int from, int to, int steps)
{
  numSteps = steps;
  step = ((float)to - (float)from) / (float)steps;

  currPos = (float)from;
  currStep = 0;
  
  servo.write(currPos);
}

bool SmoothServo::process()
{
  if (currStep >= numSteps)
      return true;
  
  currPos += step;
  //Serial.println(currPos);
  servo.write((int)currPos);
    
  currStep ++;
  return (currStep >= numSteps);
}
