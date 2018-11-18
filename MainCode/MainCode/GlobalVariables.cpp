/* <<<<< G L O B A L  V A R I A B L E S . C P P / .H

[ Instantiates and holds all global variables for the program ]
 

-For a given variable, add in a protoype in the header file with type extern in front :

extern int myInt;

-Then define the variable in the.cpp file :

int myInt = 12; < --note, you still need to declare the variable type, but no longer need the extern signifier

	- Arrays can be externed as well, but must be given a real number initializer as an array size within the header file(no variables)

	- enums also may be externed, but the steps are a little unusual :

In the header file, first create the enum prototype :

	enum week { mon, tues, weds, thur, fri, sat }; < -- - We haven't instantiated a weekDays object, just created the definition for the enum

	Then create an extern for an instance of the enum (also in the header)

	extern enum week weekFive; < --weekFive is the actual instance of a 'week' type enum

	Finally, in the cpp file, repeated the instantiation of the weekFive enum, without the extern

	enum week weekFive;

*/
#include "GlobalVariables.h"
#include <BluetoothSerial.h> 


// ************* U S E R   D E F I N E D   V A R I A B L E S
// bluetooth
BluetoothSerial SerialBT;
byte stepperCase;

// steppers
int currentDomePosition = 0;
int currentDomeDirection = 0;

int currentValvePosition = 0;
int currentValveDirection = 0;
/* stepper default values[5]
{
speed-[steps per second],
acceleration-[steps per sec],
current-[mA],step limit-[# of steps],
direction of home
}
*/

int valveStepperDefaults[5] = {450,300,1500,100,LOW };	//low on dir pin is close
int domeStepperDefaults[5] = {250,400,450,395,HIGH}; //high on dome dir pin is ccw and home
 
byte hallSensorDomeVal;
byte hallSensorValveVal;

// pulse counter
double duration;
float freq;
float oldfreq;

// power    
float solarPanelVoltageVal;                     // VALUE READ FROM GPIO 3   OR ADC7

// sleep, realtimeclock, power management 
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR struct timeval tv;
RTC_DATA_ATTR time_t time1;									 // delcare time1 as a typedef time type
RTC_DATA_ATTR struct tm tm1;
RTC_DATA_ATTR  int usrHour, usrMin, usrSec, usrDay, usrMon, usrYear, secsLastBootOffset;
RTC_DATA_ATTR int waterHour, waterMin;


//******* V A R I A B L E S  A N D  A R R A Y S -- D A V E 

bool firstSingleSquare = true;  //Used to allow any packet # for first square
bool repeatPacketReceived = false;  //Flag if packet received was a duplicate

char squarePacketNumberChar[4] = { '0', '0', '0', 0x00 };
char lastSquarePacketNumberChar[4] = { '0', '0', '0', 0x00 };
int squarePacketNumberInt = 0;
int lastSquarePacketNumber = 0;

char squareChecksumChar[4] = { '0', '0', '0', 0x00 };

char squareID[4] = { '0','0','0',0x00 };  //Holds the value for the currently selected square during bed creation
int squareIDInt = 998;

char singleSquare_lastPacket[11] = { '%', '@', '@', '@', '@', '@', '@', '@', '@', '@', 0x00 };
char singleSquareData[11] = { '%', '@', '@', '@', '@', '@', '@', '@', '@', '@', 0x00 };

bool quickOff = false;  //Used in debug to flag something off to avoid repeat serial prints
bool message = false;

const int SQUARES_PER_ROW = 25;
const int TOTAL_SQUARES = SQUARES_PER_ROW * SQUARES_PER_ROW;
const int STEPS_PER_FULL_TURN = 400;

int squareArray[625][4]; // [square id #][ {x,y,distance,angle} ]

enum serialStates serialState;
				// Used during serial error handling checks
enum packetState squareChecksumState;
enum packetState squarePacketState;// Ok -- proceed with serial packet handling
																				// Ignore -- skip packet																				// Resend -- request packet aga
enum systemStates systemState;
enum systemStates systemState_previous;


//J A M E S '  S U P E R  C O O L  S P I F F S  V A R I A B L E S

//string to write to SPIFFS, received from bluetooth

int spiffsSize;		//size of total spiffs contents
char *input2DArray[13];	//container for each of 14 input strings
int input2DArrayPosition = 0;	//position in 2D array