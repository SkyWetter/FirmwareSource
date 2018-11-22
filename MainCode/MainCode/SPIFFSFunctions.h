#pragma once

void spiffsBegin();
void spiffsSave(char array[], int arraySize, char packageNum[]);
void spiffsAppend(char array[], int arraySize);
void spiffsRead(char fileNum[]);
void spiffsParse(char header[]);
void extractBedData(char array[]);