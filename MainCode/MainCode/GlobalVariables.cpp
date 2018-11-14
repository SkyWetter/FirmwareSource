#include "GlobalVariables.h"
#include <BluetoothSerial.h> 


// STEPPER VARIABLES

int dome_maxPos_CW = 400;
int dome_minPos_CCW = 0;
int stepCountDome = 0;
int stepCountValve = 0;
int currentDomePosition = 0;
int currentDomeDirection = 0;
int domeDefaultSpeed = 10;


// SOLAR PANEL VARIABLES

float solarPanelVoltageVal;          

// RTC_DATA_ATTR int bootCount = 0;                 // this will be saved in deep sleep memory (RTC mem apprently == 8k)
RTC_DATA_ATTR time_t last;                 // remember last boot in RTC Memory
struct timeval now;


// SERIAL DATA

BluetoothSerial SerialBT;  // Given name for the ESP's serialBT object

bool firstSingleSquare = true;					//Used to allow any packet # for first square
bool repeatPacketReceived = false;				//Flag if packet received was a duplicate

char squarePacketNumberChar[4] = { '0', '0', '0', 0x00 };       //Holds packet # string for single square serial packet
char lastSquarePacketNumberChar[4] = { '0', '0', '0', 0x00 };   // Holds packet # string for last square serial packet
int squarePacketNumberInt = 0;                                  // Int versions of above
int lastSquarePacketNumber = 0;

char squareChecksumChar[4] = { '0', '0', '0', 0x00 };           

char squareID[4] = { '0','0','0',0x00 };  //Holds the value for the currently selected square during bed creation
int squareIDInt = 998;                    // Arbitrary starting value for square id (can't be 0 or less than 626)

char singleSquare_lastPacket[11] = { '%', '@', '@', '@', '@', '@', '@', '@', '@', '@', 0x00 }; //Previous incoming squarepacket
char singleSquareData[11] = { '%', '@', '@', '@', '@', '@', '@', '@', '@', '@', 0x00 };        //Incoming squaredata packet

bool quickOff = false;  //Used in debug to flag something off to avoid repeat serial prints
bool message = false;

const double metersPerSquare = 0.5;
const int SQUARES_PER_ROW = 25;
const int TOTAL_SQUARES = SQUARES_PER_ROW * SQUARES_PER_ROW;
const int STEPS_PER_FULL_TURN = 400;


int squareArray[TOTAL_SQUARES][6]; // [square id #][ {x,y,distance,angle,lowFlowTolerance,HighFlowTolerance} ]

enum serialStates serialState;
				// Used during serial error handling checks
enum packetState squareChecksumState;
enum packetState squarePacketState;// Ok -- proceed with serial packet handling
																				// Ignore -- skip packet																				// Resend -- request packet aga
enum systemStates systemState;
enum systemStates systemState_previous;
