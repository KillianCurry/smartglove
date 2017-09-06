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
	//Objects keeping track of individual glove data.
	std::vector<Glove> gloves;
	//Recorded poses.
	std::vector<Pose> poses;
	///<summary>Converts a UUID string to a handle.</summary>
	///<param name="pGUID">The GUID to convert.</param>
	///<returns>A HANDLE pointing to the peripheral device.</returns>
	SMARTGLOVE_API HANDLE getHandle(__in GUID pGUID);
	///<summary>Connects to the BLE device.</summary>
	///<returns>Whether the connection was established successfully.</returns>
	SMARTGLOVE_API bool establishConnection(int gloveID);
	///<summary>Processes values when a characteristic changes.</summary>
	SMARTGLOVE_API void CALLBACK notificationResponse(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context);
	///<summary>Closes the BLE connection.</summary>
	///<param name="gloveID">ID of the glove to close the connection for.</param>
	SMARTGLOVE_API void closeConnection(int gloveID);
	///<summary>Adds a new glove to the library based on a given UUID.</summary>
	///<param name="buffer">A pointer to a stringbuilder in managed memory, with the UUID enclosed in {}.</param>
	///<param name="bufferSize">The size of the stringbuilder.</param>
	SMARTGLOVE_API void addUUID(char* buffer, int* bufferSize);
	///<summary>Finds all paired StretchSense gloves and generates corresponding Glove objects for the gloves vector.</summary>
	///<returns>An ordered string of glove UUIDs, separated by spaces.</returns>
	SMARTGLOVE_API char* findGloves();
	///<summary>Reads values from the BLE peripheral. Sensor values returned are the three IMU orientation components, then the ten stretch sensors, in order.</summary>
	///<param name="gloveID">ID of the glove to read.</param>
	///<returns>An array of doubles for the sensor values.</returns>
	SMARTGLOVE_API double* getData(int gloveID);
	///<summary>Read time since last BLE notification.</summary>
	///<param name="gloveID">ID of the glove to read last notification time from.</param>
	///<returns>The time in seconds since the last update.</returns>
	SMARTGLOVE_API double getLastNotification(int gloveID);
	///<summary>Clears the autocalibrated values.</summary>
	///<param name="gloveID">ID of the glove to clear calibration from.</param>
	SMARTGLOVE_API void clearCalibration(int gloveID);
	///<summary>Cleans all the memory used by the library.</summary>
	SMARTGLOVE_API void closeLibrary();

	SMARTGLOVE_API void capturePose(int gloveID, char* buffer, int* bufferSize);

	SMARTGLOVE_API void writeOutPoses(char* buffer, int* bufferSize);

	SMARTGLOVE_API void readInPoses(std::string fileName);

	SMARTGLOVE_API bool checkPoseName(int gloveID, char* buffer, int* bufferSize);

	std::string convertChar(char *buffer, int *bufferSize);
#ifdef __cplusplus
}
#endif