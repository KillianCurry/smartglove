#include "smartglove.h"

#ifdef SMARTGLOVE_EXPORTS
#define SMARTGLOVE_API __declspec(dllexport)
#else
#define SMARTGLOVE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	SMARTGLOVE_API int hello()
	{
		return 3;
	}
#ifdef __cplusplus
}
#endif