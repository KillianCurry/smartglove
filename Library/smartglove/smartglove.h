#pragma once

#ifdef SMARTGLOVE_EXPORTS
#define SMARTGLOVE_API __declspec(dllexport)
#else
#define SMARTGLOVE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/*
		Opens the specified serial port.
		
		@param portNum The number corresponding to the desired port.
		@return Whether the port was opened successfully.
	*/
	SMARTGLOVE_API bool openPort(int portNum);
	/*
		Closes the open port.
	*/
	SMARTGLOVE_API void closePort();
	/*
		Reads everything currently in the port buffer.
		Sensor values returned are the ten stretch sensors, in order.

		@return An array of doubles for the sensor values.
	*/
	SMARTGLOVE_API double* getData();
	/*
		Releases the pointer from memory to prevent memory leaks.

		@param line The pointer to release.
		@return Whether the memory was correctly freed.
	*/
	SMARTGLOVE_API int releaseLine(double* line);
	/*
		Set the lower bound of sensor values.
		Corresponds to 0 in the final range.
	*/
	SMARTGLOVE_API void calibrateMinimum();
	/*
		Set the upper bound of sensor values.
		Corresponds to 1 in the final range.
		If the lower bound matches the upper bound,
		the sensor is not considered calibrated.
	*/
	SMARTGLOVE_API void calibrateMaximum();

#ifdef __cplusplus
}
#endif