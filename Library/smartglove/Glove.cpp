#include "Glove.h"

#define CHANNELS 10

	Glove::Glove(std::string _UUIDstr, int _ID)
	{
		//set UUID and glove ID
		UUIDstr = _UUIDstr;
		ID = _ID;

		minValues = std::vector<int>(10, INT_MAX);
		maxValues = std::vector<int>(10, INT_MIN);

		stretchRaw = std::vector<int>(10, 0);
		imuRaw = std::vector<int>(10, 0);
		stretch = std::vector<double>(10, 0);
	}

	Glove::~Glove()
	{
		//close the BLE connection
		CloseHandle(pHandle);
		//clear and free the vectors
		minValues.clear();
		maxValues.clear();
		stretchRaw.clear();
		imuRaw.clear();
	}
	
	void Glove::clearCalibration()
	{
		//set values that are guaranteed to be overwritten
		for (int i = 0; i < CHANNELS; i++)
		{
			minValues[i] = INT_MAX;
			maxValues[i] = INT_MIN;
		}
	}