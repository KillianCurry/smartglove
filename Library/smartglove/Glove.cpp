#include "Glove.h"

#define CHANNELS 10

	Glove::Glove(std::string _UUIDstr, int _ID)
	{
		//set UUID and glove ID
		UUIDstr = _UUIDstr;
		ID = _ID;

		minValues = std::vector<int>(10, INT_MAX);
		maxValues = std::vector<int>(10, INT_MIN);

		stretch = std::vector<int>(10, 0);
		imu = std::vector<int>(10, 0);
	}

	Glove::~Glove()
	{
		//close the BLE connection
		CloseHandle(pHandle);
		//clear and free the vectors
		minValues.clear();
		maxValues.clear();
		stretch.clear();
		imu.clear();
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