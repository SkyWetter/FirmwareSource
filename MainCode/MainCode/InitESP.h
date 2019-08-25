#pragma once

void initPins();
void initSerial();
void initRainBow();
void initThreads();
//void initSleepClock();
void initWiFiClock();
void printLocalWifiTime();
void codeForTask1(void * parameter);
void checkWakeUpReason();
void initSerialBT();
void createSquareArray(int squaresPerRow);