#pragma once

void scheduledSprayRoutine();
void stepperGoHome(byte x, byte y, byte z, byte s);
void domeGoHome();
void valveGoHome();
void moveDome(int targetPosition);
void moveDome(int targetPosition, int speed, int accel, int current);
void moveDome(int stepsToMove, int direction);
void moveDome(int stepsToMove, int direction, int speed, int accel, int current);
void moveToPosition(int stepperpin, int targetPosition, int speed, int accel, int current);
void setCurrent(int pin, int current);
void executeSquare(int mysquare);
void makeRain(float flow);

void stepperOneStepHalfPeriod(byte step, byte dir, byte enable, int *spcnt, int halfFreq);
void stepperDomeOneStepHalfPeriod(int hf);
void stepperValveOneStepHalfPeriod(int hf);
void setDomeDirection(int direction);
void stepperDomeDirCW();
void stepperDomeDirCCW();
void toggleStepperValveDir();
void valveStepperOneStep();

void crazyDomeStepperFromDebugA();