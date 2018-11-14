#include "GlobalVariables.h"
#include <BluetoothSerial.h> 


BluetoothSerial SerialBT;  // Given name for the ESP's serialBT object
byte stepperCase;

// steppers
int stepCountDome = 0;
int stepCountValve = 0;

byte hallSensorDomeVal;
byte hallSensorValveVal;

// pulse counter
double duration;
int freq;

// power    
float solarPanelVoltageVal;                     // VALUE READ FROM GPIO 3   OR ADC7

// power management
// RTC_DATA_ATTR int bootCount = 0;                 // this will be saved in deep sleep memory (RTC mem apprently == 8k)
RTC_DATA_ATTR time_t last;                 // remember last boot in RTC Memory
struct timeval now;


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

int currentDomePosition = 0;
int currentDomeDirection = 0;

bool quickOff = false;  //Used in debug to flag something off to avoid repeat serial prints
bool message = false;

const double metersPerSquare = 0.5;
const int SQUARES_PER_ROW = 25;
const int TOTAL_SQUARES = SQUARES_PER_ROW * SQUARES_PER_ROW;
const int STEPS_PER_FULL_TURN = 400;

int domeDefaultSpeed = 10;

int squareArray[TOTAL_SQUARES][6]; // [square id #][ {x,y,distance,angle,lowFlowTolerance,HighFlowTolerance} ]

enum serialStates serialState;
				// Used during serial error handling checks
enum packetState squareChecksumState;
enum packetState squarePacketState;// Ok -- proceed with serial packet handling
																				// Ignore -- skip packet																				// Resend -- request packet aga
enum systemStates systemState;
enum systemStates systemState_previous;
