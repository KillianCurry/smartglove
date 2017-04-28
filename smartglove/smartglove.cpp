//This code snippet will help you to read data from arduino
#include "smartglove.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SerialPort.h"

using std::cout;
using std::endl;

//String for incoming data
char incomingData[MAX_DATA_LENGTH];

#ifdef SMARTGLOVE_EXPORTS
#define SMARTGLOVE_API __declspec(dllexport)
#else
#define SMARTGLOVE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	SerialPort *serial;

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

	SMARTGLOVE_API char* getData()
	{
		//Check if data has been read or not
		int read_result = serial->readSerialPort(incomingData, MAX_DATA_LENGTH);
		//prints out data
		return(incomingData);
	}

#ifdef __cplusplus
}
#endif