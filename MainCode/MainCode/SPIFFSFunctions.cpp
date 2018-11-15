#include "SPIFFSFunctions.h"
#include <SPIFFS.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "GlobalVariables.h"
#include "GeneralFunctions.h"


//initialize the SPIFFS system
void spiffsBegin()
{
	if (!SPIFFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}
}

//create the SPIFFS save file and populate with first 
bool spiffsSave(char array[], int arraySize)
{
	bool success = false;

	File file = SPIFFS.open("/garden.txt", FILE_WRITE); 

	if (!file)
	{
		Serial.println("There was an error opening the file for writing");
		return;
	}

	for (int i = 0; i < arraySize; i++)
	{
		file.print(array[i]);
	}

	if(file.size() == arraySize)
	{
		Serial.println("File content was saved");
		spiffsSize = arraySize;
		success = true;
	}
	else
	{
		Serial.println("File save failed");
	}

	file.close();

	return success;
}

bool spiffsAppend(char array[], int arraySize)
{
	bool success = false;

	File file = SPIFFS.open("/garden.txt", FILE_APPEND);

	if (!file)
	{
		Serial.println("There was an error opening the file for appending");
		return;
	}

	for (int i = 0; i < arraySize; i++)
	{
		file.print(array[i]);
	}

	spiffsSize += arraySize;

	if (file.size() == arraySize)
	{
		Serial.println("File content was saved");
		success = true;
	}
	else
	{
		Serial.println("File save failed");
	}

	file.close();

	return success;
}

void spiffsRead()
{
	File file = SPIFFS.open("/garden.txt");

	if (!file)
	{
		Serial.println("There was an error opening the file for reading");
		return;
	}

	while (file.available())
	{
		Serial.print(file.readString());
	}

	//printString((char*)file.read(), 20);  //try this

	file.close();
}