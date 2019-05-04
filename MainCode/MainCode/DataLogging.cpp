/* Data Logging Test File*/
//Takes a float input, associated with a field specified in the enum located in DataLogging.h
// and sends to through an HTTP Post function. Returns the html response code as an int

#include "DataLogging.h"
#include <HTTPClient.h>



//||||||||||||||||||||||||||||
//		Variables 


const char* urlStart = "https://docs.google.com/forms/d/e/1FAIpQLScInTNH6d752Bc1mI0PB-YwCMFBcAZEuQjzhQUogPvv82PBQg/formResponse?";
const char* dataFields[2] = {
	"entry.455005866=",
	"entry.1135029920="
};
const char* end = "submit=Submit";

char toSend[1000];


// Log Data
// Takes a value and field number and returns an html response code 



int logData(float val, int field){

	createURL(val, field);
	int response = postData();
	return response;
	
}

int logData(float* val, int* field,int numVals)
{
	createURL(val, field, numVals);
	int response = postData();
	return response;
}


// CREATE URL  -- takes value and field #, returns a string assembled for http POST operation
void createURL(float val, int field){

	sprintf(toSend, "%s%s%.5f%s", urlStart, dataFields[field], val, end);
}

void createURL(float* val, int* field,int numVals)
{
	strcpy(toSend,urlStart);

	for (int i = 0; i < numVals; i++)
	{
		char buf[100];
		sprintf(buf, "%.5f", val[i]);
		strcat(toSend,dataFields[field[i]]);
		strcat(toSend, buf);
		strcat(toSend, "&");

	}
}

//Posts the data to google sheets via HTTP Post command


int postData()
{
	HTTPClient http;		//Create http client to 
	http.begin(toSend);		// Prepare http with url to send

	int response = http.POST("Posting");  //Post data
	printf("DataLogging->postData: HTTP Post Response %d\n",response); 
	
	return response;

}

