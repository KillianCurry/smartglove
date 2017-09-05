#pragma once

#ifdef SMARTGLOVE_EXPORTS
#define SMARTGLOVE_API __declspec(dllexport)
#else
#define SMARTGLOVE_API __declspec(dllimport)
#endif

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <Bluetoothleapis.h>
#include <string>
#include <sstream>
#include <limits.h>
#include <iostream>
#include <fstream>
#include "Glove.h"
#include "Pose.h"

#ifdef __cplusplus
extern "C" {
#endif

	std::vector<Glove> gloves;
	std::vector<Pose> poses;
	/*
		Converts a UUID string to a handle.

		@param pGUID The GUID to convert.
		@return A HANDLE pointing to the peripheral device.
	*/
	SMARTGLOVE_API HANDLE getHandle(__in GUID pGUID);
	/*
		Connects to the BLE device.

		@return Whether the connection was established successfully.
	*/
	SMARTGLOVE_API bool establishConnection(int gloveID);
	/*
		Processes values when a characteristic changes.

		@param EventType
		@param EventOutParameter
		@param Context
	*/
	SMARTGLOVE_API void CALLBACK notificationResponse(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context);
	/*
		Closes the BLE connection.
	*/
	SMARTGLOVE_API void closeConnection(int gloveID);
	SMARTGLOVE_API void addUUID(char* buffer, int* bufferSize);
	/*
		Finds all paired StretchSense gloves and generates
		corresponding Glove objects for the gloves vector.

		@return An ordered string of glove UUIDs, separated by spaces
	*/
	SMARTGLOVE_API char* findGloves();
	/*
		Reads values from the BLE peripheral.
		Sensor values returned are the three IMU orientation
		compents, then the ten stretch sensors, in order.

		@return An array of doubles for the sensor values.
	*/
	SMARTGLOVE_API double* getData(int gloveID);
	/*
		Read time since last BLE notification.

		@return The time in seconds since the last update.
	*/
	SMARTGLOVE_API double getLastNotification(int gloveID);
	/*
		Release the pointer passed to the DLL.

		@param ptr The pointer to release.
	*/
	SMARTGLOVE_API void freePointer(char* ptr);
	/*
		Clears the autocalibrated values.
	*/
	SMARTGLOVE_API void clearCalibration(int gloveID);
	/*
		Cleans all the memory used by the library.
	*/
	SMARTGLOVE_API void closeLibrary();

	SMARTGLOVE_API void capturePose(int gloveID, char* buffer, int* bufferSize);

	SMARTGLOVE_API void writeOutPoses(char* buffer, int* bufferSize);

	SMARTGLOVE_API void readInPoses(std::string fileName);

	SMARTGLOVE_API bool checkPoseName(int gloveID, char* buffer, int* bufferSize);

	std::string convertChar(char *buffer, int *bufferSize);
#ifdef __cplusplus
}
#endif