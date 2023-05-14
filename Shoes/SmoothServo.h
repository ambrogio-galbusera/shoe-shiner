#ifndef SMOOTHSERVO_H
#define SMOOTHSERVO_H

#include "Arduino.h" 
#include "Servo.h"

class SmoothServo
{
public:
  void attach(int pin, int pos);
  void move(int from, int to, int steps);
  bool process();
  
private:
    Servo servo;
    float currPos;
    int currStep;
    int numSteps;
    float step;
};

#endif
