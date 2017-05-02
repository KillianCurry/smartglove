//This code snippet will help you to read data from arduino
#include "smartglove.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "SerialPort.h"

using std::cout;
using std::endl;


#ifdef SMARTGLOVE_EXPORTS
#define SMARTGLOVE_API __declspec(dllexport)
#else
#define SMARTGLOVE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	SerialPort *serial;
	//the values on the current line of data
	std::vector<double> lineValues;
	//a string to store the current number
	char curString[MAX_DATA_LENGTH];
	//the current character index, to build the string
	int curChar = 0;

	SMARTGLOVE_API bool openPort(int portNum)
	{
		/*Portname must contain these backslashes, and remember to
		replace the following com port*/
		char *portPrefix = "\\\\.\\COM";
		
		char portName[20] = "";
		snprintf(portName, 20, "%s%i", portPrefix, portNum);
		serial = new SerialPort(portName);

		return serial->isConnected();
	}

	SMARTGLOVE_API void closePort()
	{
		delete serial;
	}

	SMARTGLOVE_API double* getData()
	{
		//read the contents of the serial port
		char incomingData[MAX_DATA_LENGTH];
		serial->readSerialPort(incomingData, MAX_DATA_LENGTH);
		//vector of latest values from the serial port
		std::vector<double> latestLine;
		//iterate through the data
		int c = 0;
		while (incomingData[c] != '\0')
		{
			//on a linebreak, process the line of values
			if (incomingData[c] == '\n')
			{
				latestLine = lineValues;
				//clear the vector
				lineValues.clear();
			}
			//on a comma, convert the full number string into a float
			else if (incomingData[c] == ',')
			{
				//terminate the current number string
				curString[curChar] = '\0';
				//add the converted string to the current line's values
				lineValues.push_back(::atof(curString));
				//reset the current number string
				curString[0] = '\0';
				curChar = 0;
			}
			//otherwise, read the character into the current number string
			else
			{
				curString[curChar] = incomingData[c];
				curChar++;
			}
			c++;
		}
		//return a pointer to the last line that was read
		return latestLine.data();
	}

	//release the pointer for the latest line, preventing memory leaks
	SMARTGLOVE_API int releaseLine(double* line)
	{
		delete[] line;
		return 0;
	}

#ifdef __cplusplus
}
#endif