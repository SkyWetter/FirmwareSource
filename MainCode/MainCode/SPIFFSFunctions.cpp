#include "SPIFFSFunctions.h"
#include "SPIFFS.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void spiffsBegin()
{
	if (!SPIFFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}
}

void spiffsSave()
{
	File file = SPIFFS.open(fileName, FILE_WRITE); // all underlined
}

