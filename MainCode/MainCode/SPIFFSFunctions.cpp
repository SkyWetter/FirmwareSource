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
	bool addMore = true;

	File file = SPIFFS.open("/garden.txt", FILE_WRITE); 

	if (!file)
	{
		Serial.println("There was an error opening the file for writing");
		return;
	}

	while (addMore == true)
	{
		Serial.println("Enter string");

		while (!Serial.available()) {}

		while (Serial.available())
		{
			file.print(Serial.readString());
		}

		Serial.println("Press '0' to stop adding text, '1' to continue");

		while (!Serial.available()) {}

		if (Serial.read() == '0')
		{
			addMore = false;
		}

	}

	Serial.println("File saved");

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
		Serial.print(file.readString());
	}

	file.close();
}



