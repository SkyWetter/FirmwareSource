// standard and ESP-IDF includes
#include <SPIFFS.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// local includes
#include "GlobalVariables.h"
#include "GeneralFunctions.h"
#include "StepperFunctions.h"
#include "SPIFFSFunctions.h"


//initialize the SPIFFS system
void spiffsBegin()
{
	if (!SPIFFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}
	Serial.println("SPIFFS are goin!");
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


//parse spiffs data -> pull out data in format approipriate for spraying.
//input is the packet number desired
//Mon AM is 0001, Mon PM is 0002, Tues AM is 0003, etc <- enumerate this
void spiffsParse(char fileNum[])
{
	char fileName[] = "/xxxx.txt";
	char header[10];
	char lengthArray[4];
	int length;

	clearArray(bedsToSprayFile, 5000);		//clear temp array for new file

	//desired packet number is file name
	for (int i = 0; i < 4; i++)
	{
		fileName[i + 1] = fileNum[i];
	}
	Serial.print("spiffsParse fileName: ");
	Serial.println(fileName);
	
	//open file for reading only
	File file = SPIFFS.open(fileName);

	// error handling
	if (!file)
	{
		Serial.println("There was an error opening the file for reading");
		return;
	}

	//rebuild header file
	for(int i = 0; i < 10; i++)
	{
		header[i] = file.read();
	}

	//determine length for for loop
	for (int i = 0; i < 4; i++)
	{
		lengthArray[i] = header[(i + 6)]; 
		//bedsToSprayFile[i] = header[i+1];
	}
	length = (charToInt(lengthArray, 4) - 10);		//-10 from length because we are not including the header	// JAMES CONFIRM!!! ---> ANDY Added this.. not sure if it is redundant or not but maybe makes sense?!?!?!?

	for (int i = 0; i < length; i++)
	{
		bedsToSprayFile[i] = file.read();
	}

	bedsToSprayLength = length;

	Serial.print("spiffsParse file: ");
	for (int i = 0; i < length; i++)
	{
		Serial.print(bedsToSprayFile[i]);
	}
	Serial.println();
		
}

//pass in bedsToSprayFile[]
//format: : !3,000!2,765!3,604!2,111!1,667
void extractBedData(char array[])
{
	int j = 0;	//bedsToSprayFile incrementer
	int k = 0;	//bedsToSprayInstructions incrementer

	clearIntArray(bedsToSprayInstructions, 5000);					//clear temp array for new file

	Serial.println("bedsToSprayInstructions: ");
	
	while (j <= bedsToSprayLength)	
	{
		if (array[j] == '!')
		{
			int timesToWrite;
			int bedNum;
			char bedNumArray[3];

			j++;												//step to number of repeats

			timesToWrite = array[j] - 48;						//convert char to int 
			Serial.printf("timesToWrite is %i \n", timesToWrite);

			j++;												//step to comma

			while (array[j] == ',')
			{
				for (int i = 0; i < 3; i++)						// parse bed number into temp char array ?? ALSO WHAT IF BEDNUMBER S S 12 and not 012???
				{
					j++;
					bedNumArray[i] = array[j];		
				}

				bedNum = charToInt(bedNumArray, 3);				//convert bedNameChar array to bedNum in interger form

				for (int i = 0; i < timesToWrite; i++)			// load amount of times to spray into main bedToSpray task table
				{
					bedsToSprayInstructions[k] = bedNum;
					Serial.println(bedNum);
					executeSquare(bedNum);
					delay(1000);
					k++;
				}
				valveGoHome();

				if (array[(j + 1)] == ',')
				{
					j++;
				}
			}
		}

		j++;

	}
	for (int i = 0; i < k; i++)
	{
		Serial.print(bedsToSprayInstructions[i]);
	}
	Serial.println();

	Serial.println("parseBedData done");
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

/*
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
}*/