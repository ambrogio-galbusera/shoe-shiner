#include "SmoothServo.h"
#include "SmoothPWM.h"

//#define TEST_MODE

#define SERVO_FRONT_PIN     9
#define SERVO_AFT_PIN       11
#define SERVO_LEFT_PIN      3
#define SERVO_RIGHT_PIN     10

#define FRONT_UP          0
#define FRONT_DOWN        160
#define AFT_UP            0
#define AFT_DOWN          160
#define RIGHT_UP          0
#define RIGHT_DOWN        112
#define LEFT_UP           0
#define LEFT_DOWN         112

#define DELAY_MS          10
#define MOVE_DURATION_MS  3000
#define RAMP_DURATION_MS  1000
#define NUM_STEPS         (MOVE_DURATION_MS / DELAY_MS)
#define NUM_PWM_STEPS     1/*(RAMP_DURATION_MS / DELAY_MS)*/
#define NUM_SAMPLES       20

#define LOADCELL_PIN        A0

#define SWITCH1_PIN         12
#define SWITCH2_PIN         13

#define M1_DIRECTION_PIN    4
#define M1_SPEED_PIN        5
#define M2_DIRECTION_PIN    7
#define M2_SPEED_PIN        6

#define M1_PWM              100
#define M1_RPM_BY_10        165L
#define ROTATION_MS         ((60L*1000L*10L) / M1_RPM_BY_10)
#define HALFROTATION_MS     (ROTATION_MS / 2L)
#define MORE_HALFROTATION_MS (ROTATION_MS *4L / 8L)
#define CLEANING_ROTS       1L
#define CLEANING_MS         (CLEANING_ROTS * HALFROTATION_MS)

SmoothServo servoFront;
SmoothServo servoAft;
SmoothServo servoLeft;
SmoothServo servoRight;
SmoothPWM motor;

enum AppStatusEnum {
  AppStatus_Idle,  
  AppStatus_Delay,  
  AppStatus_AftUp,  
  AppStatus_FrontUp,  
  AppStatus_SidesUp,  
  AppStatus_Cleaning,  
  AppStatus_SidesDown,
  AppStatus_FrontDown,
  AppStatus_AftDown,
  AppStatus_Reverting,  
};

typedef struct {
  long startMs;
  int numSamples;  
} IdleData;

typedef struct {
  long startMs;
} DelayData;

typedef struct {
  long startMs;
} RevertingData;

typedef struct {
  int clockwise;
  int numReps;  
  long startMs;
} CleaningData;

typedef union
{
  IdleData idle;
  DelayData delay;
  CleaningData cleaning;
  RevertingData reverting;
} AppData;

AppStatusEnum appStatus;
AppData appData;

typedef AppStatusEnum (*statusInitFnc)(AppData* data);
typedef AppStatusEnum (*statusUpdateFnc)(AppData* data);

void appIdleInit(AppData* data);
AppStatusEnum appIdleUpdate(AppData* data);

void appDelayInit(AppData* data);
AppStatusEnum appDelayUpdate(AppData* data);

void appAftUpInit(AppData* data);
AppStatusEnum appAftUpUpdate(AppData* data);

void appFrontUpInit(AppData* data);
AppStatusEnum appFrontUpUpdate(AppData* data);

void appSidesUpInit(AppData* data);
AppStatusEnum appSidesUpUpdate(AppData* data);

void appCleaningInit(AppData* data);
AppStatusEnum appCleaningUpdate(AppData* data);

void appAftDownInit(AppData* data);
AppStatusEnum appAftDownUpdate(AppData* data);

void appFrontDownInit(AppData* data);
AppStatusEnum appFrontDownUpdate(AppData* data);

void appSidesDownInit(AppData* data);
AppStatusEnum appSidesDownUpdate(AppData* data);

void appRevertingInit(AppData* data);
AppStatusEnum appRevertingUpdate(AppData* data);

statusInitFnc initFunctions[] = {
  appIdleInit,
  appDelayInit,
  appAftUpInit,
  appFrontUpInit,
  appSidesUpInit,
  appCleaningInit,
  appSidesDownInit,
  appFrontDownInit,
  appAftDownInit,
  appRevertingInit,
};

statusUpdateFnc updateFunctions[] = {
  appIdleUpdate,
  appDelayUpdate,
  appAftUpUpdate,
  appFrontUpUpdate,
  appSidesUpUpdate,
  appCleaningUpdate,
  appSidesDownUpdate,
  appFrontDownUpdate,
  appAftDownUpdate,
  appRevertingUpdate,
};


void setupPins()
{
  pinMode(SWITCH1_PIN, INPUT);  
  pinMode(SWITCH2_PIN, INPUT);  

  pinMode(M1_DIRECTION_PIN, OUTPUT);
  pinMode(M1_SPEED_PIN, OUTPUT);
  pinMode(M2_DIRECTION_PIN, OUTPUT);
  pinMode(M2_SPEED_PIN, OUTPUT);  
}

void setupServos()
{
  servoFront.attach(SERVO_FRONT_PIN, FRONT_DOWN);
  servoAft.attach(SERVO_AFT_PIN, AFT_DOWN);
  servoLeft.attach(SERVO_LEFT_PIN, LEFT_DOWN);
  servoRight.attach(SERVO_RIGHT_PIN, RIGHT_DOWN);
}

void setup() {
  Serial.begin(115200);

  setupPins();
  setupServos();

  motor.attach(M1_SPEED_PIN, 0);
  
#ifdef TEST_MODE
  //servoFront.move(FRONT_DOWN, FRONT_UP, 60);  
  //servoAft.move(AFT_DOWN, AFT_UP, 60);  
  //servoRight.move(RIGHT_DOWN, RIGHT_UP, 60);  
  //servoLeft.move(RIGHT_DOWN, RIGHT_UP, 60);  
#else
  appStatus = AppStatus_Idle;
  initFunctions[appStatus](&appData);
#endif
}

void appIdleInit(AppData* data)
{
  IdleData* idleData = &data->idle;

  idleData->startMs = millis();
  idleData->numSamples = 0;
}

AppStatusEnum appIdleUpdate(AppData* data)
{
  IdleData* idleData = &data->idle;
  
  int value = analogRead(LOADCELL_PIN);
  //Serial.println(value);

  if ((value > 800) && (value < 900))
  {
    idleData->numSamples ++;
  }
  else 
  {
    idleData->numSamples = 0;
  }
  
  if (idleData->numSamples > NUM_SAMPLES)
    return AppStatus_AftUp;

  long delta = millis() - idleData->startMs;
  if (delta > 20000)
    return AppStatus_Delay;
  
  return AppStatus_Idle;
}

void appDelayInit(AppData* data)
{
  DelayData* delayData = &data->delay;

  delayData->startMs = millis();
}

AppStatusEnum appDelayUpdate(AppData* data)
{
  DelayData* delayData = &data->delay;
  
  long delta = millis() - delayData->startMs;
  if (delta > 10000)
    return AppStatus_AftUp;
  
  return AppStatus_Delay;
}

void appAftUpInit(AppData* data)
{
  servoAft.move(AFT_DOWN, AFT_UP, NUM_STEPS);
}

AppStatusEnum appAftUpUpdate(AppData* data)
{
  if (servoAft.process())
    return AppStatus_FrontUp;

  return AppStatus_AftUp;
}

void appFrontUpInit(AppData* data)
{
  servoFront.move(FRONT_DOWN, FRONT_UP, NUM_STEPS);
}

AppStatusEnum appFrontUpUpdate(AppData* data)
{
  if (servoFront.process())
    return AppStatus_SidesUp;

  return AppStatus_FrontUp;
}

void appSidesUpInit(AppData* data)
{
  servoLeft.move(LEFT_DOWN, LEFT_UP, NUM_STEPS);
  servoRight.move(RIGHT_DOWN, RIGHT_UP, NUM_STEPS);
}

AppStatusEnum appSidesUpUpdate(AppData* data)
{
  bool l = servoLeft.process();
  bool r = servoRight.process();
  if (l && r)
    return AppStatus_Cleaning;

  return AppStatus_SidesUp;
}

void appCleaningInit(AppData* data)
{
  CleaningData* cleaning = &data->cleaning;
  cleaning->clockwise = 0;
  cleaning->numReps = 0;
  cleaning->startMs = millis();

  // motor left until end of switch
  digitalWrite(M1_DIRECTION_PIN, cleaning->clockwise? HIGH : LOW);
  motor.ramp(0, M1_PWM, NUM_PWM_STEPS);
}

AppStatusEnum appCleaningUpdate(AppData* data)
{
  CleaningData* cleaning = &data->cleaning;

  /*
  if (!digitalRead(SWITCH1_PIN))
  {
    if (!cleaning->clockwise)
    {
      cleaning->numReps ++;
      if (cleaning->numReps >= 3)
      {
        analogWrite(M1_SPEED_PIN, 0);
        return AppStatus_SidesDown;
      }
    }
    
    // change direction
    cleaning->clockwise = !cleaning->clockwise;
    digitalWrite(M1_DIRECTION_PIN, cleaning->clockwise? HIGH : LOW);
  }
  */

  motor.process();
  
  long delta = millis() - cleaning->startMs;
  if (delta > ROTATION_MS)
  {
    Serial.print("Clockwise: ");
    Serial.print(cleaning->clockwise);
    Serial.print("; reps: ");
    Serial.println(cleaning->numReps);

    if (!cleaning->clockwise)
    {
      cleaning->numReps ++;
      if (cleaning->numReps >= 3)
      {
        motor.stop();
        return AppStatus_SidesDown;
      }
    }
    
    // change direction
    cleaning->startMs = millis();
    cleaning->clockwise = !cleaning->clockwise;
    digitalWrite(M1_DIRECTION_PIN, cleaning->clockwise? HIGH : LOW);
  }

  return AppStatus_Cleaning;
}

void appSidesDownInit(AppData* data)
{
  servoLeft.move(LEFT_UP, LEFT_DOWN, NUM_STEPS);
  servoRight.move(RIGHT_UP, RIGHT_DOWN, NUM_STEPS);
}

AppStatusEnum appSidesDownUpdate(AppData* data)
{
  bool l = servoLeft.process();
  bool r = servoRight.process();
  
  if (l && r)
    return AppStatus_Reverting;

  return AppStatus_SidesDown;
}

void appRevertingInit(AppData* data)
{
  RevertingData* reverting = &data->reverting;
  reverting->startMs = millis();

  // motor left until end of switch
  digitalWrite(M1_DIRECTION_PIN, HIGH);
  motor.ramp(0, M1_PWM, NUM_PWM_STEPS);
}

AppStatusEnum appRevertingUpdate(AppData* data)
{
  RevertingData* reverting = &data->reverting;

  motor.process();

  long delta = millis() - reverting->startMs;
  if (delta > MORE_HALFROTATION_MS)
  {
    motor.stop();
    return AppStatus_AftDown;
  }
    
  return AppStatus_Reverting;
}

void appAftDownInit(AppData* data)
{
  servoAft.move(AFT_UP, AFT_DOWN, NUM_STEPS);
}

AppStatusEnum appAftDownUpdate(AppData* data)
{
  if (servoAft.process())
    return AppStatus_FrontDown;

  return AppStatus_AftDown;
}

void appFrontDownInit(AppData* data)
{
  servoFront.move(FRONT_UP, FRONT_DOWN, NUM_STEPS);
}

AppStatusEnum appFrontDownUpdate(AppData* data)
{
  if (servoFront.process())
    return AppStatus_Idle;

  return AppStatus_FrontDown;
}

void loop() {
#ifdef TEST_MODE
  /*
  if (servoFront.process())
  {
    servoFront.move(FRONT_OPEN, FRONT_CLOSED, 60);
  }
  if (servoAft.process())
  {
    servoAft.move(AFT_OPEN, AFT_CLOSED, 60);
  }
  if (servoRight.process())
  {
    servoRight.move(RIGHT_OPEN, RIGHT_CLOSED, 60);
  }
  if (servoLeft.process())
  {
    servoLeft.move(LEFT_OPEN, LEFT_CLOSED, 60);
  }
  */
  delay(500);
#else
  AppStatusEnum newStatus = updateFunctions[appStatus](&appData);
  if (newStatus != appStatus)
  {
    Serial.print("Transitioning from ");
    Serial.print(appStatus);
    Serial.print(" to ");
    Serial.print(newStatus);
    Serial.println();

    appStatus = newStatus;  
    initFunctions[appStatus](&appData);
  }
  
  delay(DELAY_MS);
#endif  
}
