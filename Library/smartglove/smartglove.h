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
		Reads everything currently in the port buffer.
		Sensor values returned are the ten stretch sensors, in order.

		@return An array of doubles for the sensor values.
	*/
	SMARTGLOVE_API double* getData();
	/*
		Set the lower bound of sensor values.
		Corresponds to 0.00 in the final range.
	*/
	SMARTGLOVE_API void calibrateMinimum();
	/*
		Set the upper bound of sensor values.
		Corresponds to 1.00 in the final range.
		If the lower bound matches the upper bound,
		the sensor is not considered calibrated.
	*/
	SMARTGLOVE_API void calibrateMaximum(int sensor);

#ifdef __cplusplus
}
#endif