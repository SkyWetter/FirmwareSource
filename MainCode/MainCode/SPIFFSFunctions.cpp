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
	File file = SPIFFS.open("garden.txt", FILE_WRITE); 

	if (file.println(serialBedData))
	{
		Serial.println("File was written");
	}
	else
	{
		Serial.println("File write failed");
	}

	file.close();
}

void spiffsRead()
{
	File file = SPIFFS.open("garden.txt");

	if (!file)
	{
		Serial.println("Failed to open file for reading");
		return;
	}

	Serial.print("File Content: ");

	while (file.available())
	{
		Serial.print(file.read());
		spiffsBedData += file.read();
	}

	Serial.println();

	Serial.print("spiffsBedData: ");
	Serial.println(spiffsBedData);

	file.close();
}



