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
#include <chrono>
#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")

class Glove
{
public:
	//STRETCH SENSOR DATA
	//the service handle for the stretch characteristic
	USHORT stretchHandle;
	//raw stretch sensors from the BLE
	std::vector<unsigned short> stretchRaw;
	//the minimum and maximum raw stretch values
	std::vector<unsigned short> minValues;
	std::vector<unsigned short> maxValues;
	//the minimum and maximum angles of rotation for each joint
	std::vector<double> minAngles;
	std::vector<double> maxAngles;
	//the processed stretch sensor values
	std::vector<double> stretch;

	//IMU DATA
	//the service handle for the IMU characteristic
	USHORT imuHandle;
	//raw IMU from the BLE (6 values: orientation xyz, acceleration xyz)
	std::vector<unsigned short> imuRaw;

	//The last time a BLE notification was received
	std::chrono::high_resolution_clock::time_point lastNotification;

	//INFORMATION ABOUT THE GLOVE HARDWARE
	//whether the glove is equipped with an IMU
	bool hasIMU;
	//how many sensors the glove is equipped with
	int sensorCount;

	int ID;
	std::string UUIDstr;

	HANDLE pHandle;

	Glove(std::string _UUID, int _ID);
	~Glove();
	void parseUUID(std::string _UUID);
	void setAngles(double* minimum, double* maximum);
	void clearCalibration();
};

#endif