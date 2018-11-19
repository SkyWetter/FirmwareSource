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

	//pull out packet number for save file name
	for (int i = 0; i < 4; i++)
	{
		fileName[i + 1] = packageNum[i];
	}

	Serial.println(fileName);

	//create new file with packet number name
	File file = SPIFFS.open(fileName, FILE_WRITE); 

	if (!file)
	{
		Serial.println("There was an error opening the file for writing");
		return;
	}

	//save packet data under file name
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

//add additional arrays to SPIFFS - not used currently
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

//read SPIFFS data, good for testing
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

//parse spiffs data -> pull out data in format approipriate for spraying.
//input is the packet number desired
//Mon AM is 0001, Mon PM is 0002, Tues AM is 0003, etc <- enumerate this
void spiffsParse(char fileNum[])
{
	char fileName[] = "/xxxx.txt";
	char header[11];
	char lengthArray[4];
	int length;

	//desired packet number is file name
	for (int i = 0; i < 4; i++)
	{
		fileName[i + 1] = fileNum[i];
	}

	Serial.print("spiffsParse fileName: ");
	Serial.println(fileName);
	
	//open file for reading only
	File file = SPIFFS.open(fileName);

	if (!file)
	{
		Serial.println("There was an error opening the file for reading");
		return;
	}

	//rebuild header file
	for(int i = 0; i < 11; i++)
	{
		header[i] = file.read();
	}

	//determine length for for loop
	for (int i = 0; i < 4; i++)
	{
		lengthArray[i] = header[(i + 6)];
	}

	length = charToInt(lengthArray, 4);

	Serial.print("spiffsParse length: ");
	Serial.println(length);

	//pull out full file in to global variable for spraying
	for (int i = 0; i < 11; i++)
	{
		bedsToSprayFile[i] = header[i];
	}

	for (int i = 11; i < length; i++)
	{
		bedsToSprayFile[i] = file.read();
	}

	bedsToSprayLength = length;

	Serial.print("spiffsParse file: ");
	Serial.println(bedsToSprayFile);	
}

//pass in bedsToSprayFile[]
//format: !3,123,124,124,!2,12,13
void parseBedData(char array[])
{
	int j = 0;	//bedsToSprayFile incrementer
	int k = 0;	//bedsToSprayInstructions incrementer
	
	while (j <= bedsToSprayLength)
	{
		if (array[j] == '!')
		{
			int timesToWrite;
			int bedNum;
			char bedNumArray[3];

			j++;	//step to number of repeats

			timesToWrite = array[j];	

			j++;	//step to comma

			while (array[j] == ',')
			{
				for (int i = 0; i < 3; i++)
				{
					j++;
					bedNumArray[i] = array[j];
				}

				bedNum = charToInt(bedNumArray, 3);

				for (int i = 0; i < timesToWrite; i++)
				{
					bedsToSprayInstructions[k] = bedNum;
					k++;
				}

				if (array[(j + 1)] == ',')
				{
					j++;
				}
			}
		}

		j++;

	}
}