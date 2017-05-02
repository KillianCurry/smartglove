#pragma once

#ifdef SMARTGLOVE_EXPORTS
#define SMARTGLOVE_API __declspec(dllexport)
#else
#define SMARTGLOVE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	SMARTGLOVE_API bool openPort(int portNum);
	SMARTGLOVE_API void closePort();
	SMARTGLOVE_API double* getData();
	SMARTGLOVE_API int releaseLine(double* line);

#ifdef __cplusplus
}
#endif