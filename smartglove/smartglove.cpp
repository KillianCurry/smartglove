//This code snippet will help you to read data from arduino
#include "smartglove.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		//String for incoming data
		char incomingData[MAX_DATA_LENGTH];
		//read data
		serial->readSerialPort(incomingData, MAX_DATA_LENGTH);
		//make char buffer for marshaler to use
		char* buffer = NULL;
		size_t bufferSize = strlen(incomingData) + sizeof(char);
		buffer = (char*)::CoTaskMemAlloc(bufferSize);
		//copy string to managed buffer
		strcpy_s(buffer, bufferSize, incomingData);
		//return managed buffer
		return(buffer);
	}

#ifdef __cplusplus
}
#endif