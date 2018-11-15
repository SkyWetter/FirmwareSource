#pragma once
#include <BluetoothSerial.h> 
#include <Stepper.h>
#include <soc\rtc.h>
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "sys/time.h"
#include "sdkconfig.h"

// ************* U S E R   D E F I N E D   V A R I A B L E S
// bluetooth
extern BluetoothSerial SerialBT;
extern byte stepperCase;

// steppers
extern int stepCountDome;
extern int stepCountValve;

extern byte hallSensorDomeVal;
extern byte hallSensorValveVal;

// pulse counter
extern double duration;
extern int freq;

// power    
extern float solarPanelVoltageVal;                     // VALUE READ FROM GPIO 3   OR ADC7

// power management
// RTC_DATA_ATTR int bootCount = 0;                 // this will be saved in deep sleep memory (RTC mem apprently == 8k)
extern RTC_DATA_ATTR time_t last;                 // remember last boot in RTC Memory
extern struct timeval now;


//******* V A R I A B L E S  A N D  A R R A Y S -- D A V E 

extern bool firstSingleSquare;  //Used to allow any packet # for first square
extern bool repeatPacketReceived;  //Flag if packet received was a duplicate

extern char squarePacketNumberChar[4];
extern char lastSquarePacketNumberChar[4];
extern int squarePacketNumberInt;
extern int lastSquarePacketNumber;

extern char squareChecksumChar[4];

extern char squareID[4];  //Holds the value for the currently selected square during bed creation
extern int squareIDInt;

extern char singleSquare_lastPacket[11];
extern char singleSquareData[11];

extern int currentDomePosition;
extern int currentDomeDirection;

/* Program State enums */



enum serialStates { doNothing, singleSquare, fullBed, sendData, debugCommand, parseGarden };   // State during getSerial fxn
extern enum serialStates serialState;

enum packetState { ok, ignore, resend };						// Used during serial error handling checks
extern enum packetState squareChecksumState;			
extern enum packetState squarePacketState;// Ok -- proceed with serial packet handling
																				// Ignore -- skip packet
																				// Resend -- request packet again
enum systemStates { sleeping, solar, program, water, low_power };
extern enum systemStates systemState;
extern enum systemStates systemState_previous;

extern bool quickOff;  //Used in debug to flag something off to avoid repeat serial prints
extern bool message;

extern const int SQUARES_PER_ROW;
extern const int TOTAL_SQUARES; 
extern const int STEPS_PER_FULL_TURN;

extern int squareArray[625][4]; // [square id #][ {x,y,distance,angle} ]


//J A M E S '  S U P E R  C O O L  S P I F F S  V A R I A B L E S

extern int spiffsSize;
extern char* input2DArray[];
extern int input2DArrayPosition;