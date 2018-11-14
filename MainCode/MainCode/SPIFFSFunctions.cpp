#include "SPIFFSFunctions.h"
#include <SPIFFS.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "GlobalVariables.h"

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
	File file = SPIFFS.open("/garden.txt", FILE_WRITE); 

	if (!file)
	{
		Serial.println("There was an error opening the file for writing");
		return;
	}

	if (file.println(serialBedData))
	{
		Serial.print("File was written: ");
		Serial.println(serialBedData.length());

	}
	else
	{
		Serial.println("File write failed");
	}

	file.close();
}

void spiffsRead()
{
	File file = SPIFFS.open("/garden.txt");

	if (!file)
	{
		Serial.println("Failed to open file for reading");
		return;
	}

	while (file.available())
	{
		spiffsBedData += file.readString();
	}

	Serial.print("File content: ");
	Serial.println(spiffsBedData.length());

	file.close();
}



