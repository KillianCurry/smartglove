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
	//the raw values on the current line of data
	std::vector<double> lineValues;
	//the raw values on the last line of data
	std::vector<double> lastValues;
	//the processed values from the last line of data
	std::vector<double> calibratedValues;
	//the minimum and maximum raw values
	std::vector<double> minValues;
	std::vector<double> maxValues;
	//whether there's a calibration available for the given sensor value
	std::vector<bool> isCalibrated;
	//a string to store the current number
	char curString[MAX_DATA_LENGTH];
	//a string to store the raw serial data
	char incomingData[MAX_DATA_LENGTH];
	//the current character index, to build the string
	int curChar = 0;

	
	SMARTGLOVE_API bool openPort(int portNum)
	{
		//TODO idiot-proof this function
		//backslashes make port name safe for double-digit numbers
		char *portPrefix = "\\\\.\\COM";
		
		//combine the port prefix with the given port number
		char portName[20] = "";
		snprintf(portName, 20, "%s%i", portPrefix, portNum);

		//create a new SerialPort to open the port
		serial = new SerialPort(portName);

		//return true if the serial port opened properly
		return serial->isConnected();
	}

	SMARTGLOVE_API void closePort()
	{
		delete serial;
	}

	SMARTGLOVE_API double* getData()
	{
		//read the contents of the serial port
		serial->readSerialPort(incomingData, MAX_DATA_LENGTH);
		//iterate through the data
		int c = 0;
		while (incomingData[c] != '\0')
		{
			//on a linebreak, process the line of values
			if (incomingData[c] == '\n')
			{
				lastValues = lineValues;
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
			//delete the string behind as we go
			incomingData[c] = '\0';
			c++;
		}

		calibratedValues.clear();
		calibratedValues.push_back(lastValues[0]);
		calibratedValues.push_back(lastValues[1]);
		calibratedValues.push_back(lastValues[2]);
		for (int i = 0; i < lastValues.size(); i++)
		{
			//if this sensor has been calibrated, constrain the sensor value
			//into a range of zero to one. otherwise send zero.
			if (isCalibrated.size() >= i + 1 && isCalibrated[i]) calibratedValues.push_back((lastValues[i + 3] - minValues[i]) / (maxValues[i] - minValues[i]));
			else calibratedValues.push_back(0);
		}

		//return a pointer to the last line that was read
		return calibratedValues.data();
	}

	//release the pointer for the latest line, preventing memory leaks
	SMARTGLOVE_API int releaseLine(double* line)
	{
		delete[] line;
		return 0;
	}

	SMARTGLOVE_API void calibrateMinimum()
	{
		//clear previous values, if any
		minValues.clear();
		//loop through current raw sensor values
		for (int i = 0; i < 10; i++)
		{
			//set minimum to current value
			minValues.push_back(lastValues[i+3]);
		}
	}

	SMARTGLOVE_API void calibrateMaximum()
	{
		//clear previous values, if any
		maxValues.clear();
		//loop through current raw sensor values
		for (int i = 0; i < 10; i++)
		{
			//populate vector as we go
			maxValues.push_back(0);
			isCalibrated.push_back(false);
			//if the maximum is different from the minimum,
			//update max and complete calibration for that sensor
			if (lastValues[i+3] != minValues[i])
			{
				maxValues[i] = lastValues[i+3];
				isCalibrated[i] = true;
			}
		}
	}

#ifdef __cplusplus
}
#endif