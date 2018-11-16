#pragma once

void getSerialData();
int getSquareID(char singleSquaredata[]);
void checkPacketNumber(char singleSquareData[]);
void checkChecksum(char singleSquareData[]);
char getDebugChar();
void debugInputParse(char debugCommand);
void parseInput();