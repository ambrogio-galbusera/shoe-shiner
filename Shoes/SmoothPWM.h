#ifndef SMOOTHPWM_H
#define SMOOTHPWM_H

#include "Arduino.h" 
#include "Servo.h"

class SmoothPWM
{
public:
  void attach(int p, int pos);
  void ramp(int from, int to, int steps);
  void stop();
  bool process();
  
private:
    int pin;
    float currPWM;
    int currStep;
    int numSteps;
    float step;
};

#endif
