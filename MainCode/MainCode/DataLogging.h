#ifndef _DATA_LOGGING_h
#define _DATA_LOGGING_h


// Function Prototypes

int logData(float val, int field);
int logData(float* val, int* field,int numVals);
void createURL(float* val, int* field, int numVals);
void createURL(float val, int field);
int postData();

#endif // !_DATA_LOGGING_h
