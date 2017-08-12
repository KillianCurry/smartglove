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

#ifdef __cplusplus
extern "C" {
#endif
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
	SMARTGLOVE_API bool establishConnection();
	/*
		Processes values when a characteristic changes.

		@param EventType
		@param EventOutParameter
		@param Context
	*/
	SMARTGLOVE_API void notificationResponse(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context);
	/*
		Closes the BLE connection.
	*/
	SMARTGLOVE_API void closeConnection();
	/*
		Reads values from the BLE peripheral.
		Sensor values returned are the three IMU orientation
		compents, then the ten stretch sensors, in order.

		@return An array of doubles for the sensor values.
	*/
	SMARTGLOVE_API double* getData();
	/*
		Clears the autocalibrated values.
	*/
	SMARTGLOVE_API void clearCalibration();

#ifdef __cplusplus
}
#endif