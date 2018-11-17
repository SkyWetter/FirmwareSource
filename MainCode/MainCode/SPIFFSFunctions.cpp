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

//create the SPIFFS save file and populate with first array
void spiffsSave(char array[], int arraySize, char packageNum[])
{
	//bool success = false;

	int j;
	char fileName[] = "/xxxx.txt";

	for (int i = 0; i < 4; i++)
	{
		fileName[i + 1] = packageNum[i];
	}

	Serial.println(fileName);


	File file = SPIFFS.open(fileName, FILE_WRITE); 

	if (!file)
	{
		Serial.println("There was an error opening the file for writing");
		return;
	}

	for (int i = 0; i < arraySize; i++)
	{
		file.print(array[i]);
		j++;
	}

	//error check
	if(j == arraySize)
	{
		Serial.println("File content was saved");
		spiffsSize = arraySize;
		//success = true;
	}
	else
	{
		Serial.println("File save failed");
	}

	file.close();

	//return success;  wanted to have function as type bool to check for success, but wouldnt work
}

//add additional arrays to SPIFFS
void spiffsAppend(char array[], int arraySize)
{
	//bool success = false;

	int j;

	File file = SPIFFS.open("/garden.txt", FILE_APPEND);

	if (!file)
	{
		Serial.println("There was an error opening the file for appending");
		return;
	}

	for (int i = 0; i < arraySize; i++)
	{
		file.print(array[i]);
		j++;
	}

	spiffsSize += arraySize;

	if (j == arraySize)
	{
		Serial.println("File content was saved");
		//success = true;
	}
	else
	{
		Serial.println("File save failed");
	}

	file.close();

	//return success;
}

//read SPIFFS data, eventually use to acts as a second "transmission"
void spiffsRead(char fileNum[])
{
	char fileName[] = "/xxxx.txt";

	for (int i = 0; i < 4; i++)
	{
		fileName[i + 1] = fileNum[i];
	}

	Serial.println(fileName);
	
	File file = SPIFFS.open(fileName);

	if (!file)
	{
		Serial.println("There was an error opening the file for reading");
		return;
	}

	while (file.available())
	{
		Serial.print(file.readString());
	}

	Serial.println();

	//printString((char*)file.read(), 20);  //try this

	file.close();
}

void spiffsParse(char fileNum[])
{
	char fileName[] = "/xxxx.txt";
	char header[11];
	char lengthArray[4];
	int length;

	for (int i = 0; i < 4; i++)
	{
		fileName[i + 1] = fileNum[i];
	}

	Serial.print("spiffsParse fileName: ");
	Serial.println(fileName);
	
	File file = SPIFFS.open(fileName);

	if (!file)
	{
		Serial.println("There was an error opening the file for reading");
		return;
	}

	for(int i = 0; i < 11; i++)
	{
		header[i] = file.read();
	}

	for (int i = 0; i < 4; i++)
	{
		lengthArray[i] = header[(i + 6)];
	}

	length = charToInt(lengthArray, 4);

	Serial.print("spiffsParse length: ");
	Serial.println(length);


	for (int i = 0; i < 11; i++)
	{
		bedsToSpray[i] = header[i];
	}

	for (int i = 11; i < length; i++)
	{
		bedsToSpray[i] = file.read();
	}

	bedsToSprayLength = length;

	Serial.print("spiffsParse file: ");
	Serial.println(bedsToSpray);
	
}