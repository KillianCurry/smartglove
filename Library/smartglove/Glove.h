#ifndef GLOVE_H
#define GLOVE_H

#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <Bluetoothleapis.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")
#define CHANNELS 10

class Glove
{
public:
	//the minimum and maximum raw values
	std::vector<int> minValues;
	std::vector<int> maxValues;

	//the processed sensor values
	std::vector<int> stretchRaw;
	USHORT stretchHandle;
	std::vector<int> imuRaw;
	USHORT imuHandle;
	std::vector<double> stretch;

	int ID;
	std::string UUIDstr;

	HANDLE pHandle;

	Glove(std::string _UUID, int _ID);
	~Glove();
	void clearCalibration();
};

#endif