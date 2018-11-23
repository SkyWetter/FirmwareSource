// *********   P R E P R O C E S S O R S
// standard library includes
#include "rgbLed.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <soc\rtc.h>
// esp32 periph includes
#include <Stepper.h>
#include <BluetoothSerial.h> 
#include <pthread.h>
#include <SPIFFS.h>
// local includes
#include "sys/time.h"
#include "sdkconfig.h"
#include "driver\adc.h"
#include "driver/gpio.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
//#include <freertos/ringbuf.h>
// custom includes
#include  "deepSleep.h"
#include "GeneralFunctions.h"
#include "GlobalVariables.h"
#include "InitESP.h"
#include "pulseIn.h"
#include "realTimeFunctions.h"
#include "SerialData.h"
#include "SolarPowerTracker.h"
#include "SPIFFSFunctions.h"
#include "stateMachine.h"
#include "StepperFunctions.h"

// rgb led
#define rgbLedGreen 25
#define rgbLedBlue 26
#define rgbLedRed 27

void ledBlue(bool onOff)
{
	digitalWrite(rgbLedBlue, onOff);
}

void ledRed(bool onOff)
{
	digitalWrite(rgbLedRed, onOff);
}

void ledsOut()
{
	digitalWrite(rgbLedGreen, LOW);
	digitalWrite(rgbLedBlue, LOW);
	digitalWrite(rgbLedRed, LOW);
}