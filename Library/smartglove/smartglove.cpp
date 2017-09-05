//This code snippet will help you to read data from arduino
#include "smartglove.h"
#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")

using std::cout;
using std::endl;

#ifdef SMARTGLOVE_EXPORTS
#define SMARTGLOVE_API __declspec(dllexport)
#else
#define SMARTGLOVE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	SMARTGLOVE_API HANDLE getHandle(__in GUID pGUID)
	{
		HDEVINFO hDI;
		SP_DEVICE_INTERFACE_DATA did;
		SP_DEVINFO_DATA dd;
		GUID BluetoothInterfaceGUID = pGUID;
		HANDLE hComm = NULL;

		hDI = SetupDiGetClassDevs(&BluetoothInterfaceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

		if (hDI == INVALID_HANDLE_VALUE) return NULL;

		did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		dd.cbSize = sizeof(SP_DEVINFO_DATA);

		for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDI, NULL, &BluetoothInterfaceGUID, i, &did); i++)
		{
			SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

			DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			DWORD size = 0;

			if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, NULL, 0, &size, 0))
			{
				int err = GetLastError();

				if (err == ERROR_NO_MORE_ITEMS) break;

				PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);

				pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

				if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, pInterfaceDetailData, size, &size, &dd))
					break;

				hComm = CreateFile(
					pInterfaceDetailData->DevicePath,
					GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					0,
					NULL);

				GlobalFree(pInterfaceDetailData);
			}
		}

		SetupDiDestroyDeviceInfoList(hDI);
		return hComm;
	}

	SMARTGLOVE_API bool establishConnection(int gloveID)
	{
		GUID pGUID;
		std::wstring wideUUIDstr = std::wstring(gloves[gloveID].UUIDstr.begin(), gloves[gloveID].UUIDstr.end());
		CLSIDFromString(wideUUIDstr.c_str(), &pGUID);
		gloves[gloveID].pHandle = getHandle(pGUID);

		HRESULT hr;

#pragma region Find Service

		//check how many services there are by sending blank arguments to the GetServices() function
		USHORT serviceCount;
		hr = BluetoothGATTGetServices(
			gloves[gloveID].pHandle,
			0,
			NULL,
			&serviceCount,
			BLUETOOTH_GATT_FLAG_NONE);

		//something went wrong and the function isn't returning service count
		if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA))
		{
			SetLastError(hr);
			return false;
		}

		//allocate memory for the services buffer
		PBTH_LE_GATT_SERVICE serviceBuffer = (PBTH_LE_GATT_SERVICE)malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceCount);
		//check if out of memory
		if (serviceBuffer == NULL)
		{
			SetLastError(E_OUTOFMEMORY);
			return false;
		}
		//otherwise, clear the buffer memory
		else RtlZeroMemory(serviceBuffer, sizeof(BTH_LE_GATT_SERVICE) * serviceCount);

		//actually retrieve the services
		USHORT numServices;
		hr = BluetoothGATTGetServices(
			gloves[gloveID].pHandle,
			serviceCount,
			serviceBuffer,
			&numServices,
			BLUETOOTH_GATT_FLAG_NONE);

		//report if there's an issue with reading the services
		if (hr != S_OK)
		{
			SetLastError(hr);
			return false;
		}

#pragma endregion

#pragma region Find Characteristics
		USHORT charCount;
		hr = BluetoothGATTGetCharacteristics(
			gloves[gloveID].pHandle,
			&serviceBuffer[0],
			0,
			NULL,
			&charCount,
			BLUETOOTH_GATT_FLAG_NONE);
		//something went wrong and the function isn't returning characteristic count
		if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA))
		{
			SetLastError(hr);
			return false;
		}

		PBTH_LE_GATT_CHARACTERISTIC charBuffer = NULL;

		if (charCount > 0)
		{
			charBuffer = (PBTH_LE_GATT_CHARACTERISTIC)malloc(sizeof(BTH_LE_GATT_CHARACTERISTIC) * charCount);

			//check if out of memory
			if (charBuffer == NULL)
			{
				SetLastError(E_OUTOFMEMORY);
				return false;
			}
			//otherwise, empty out the buffer (fill with zeros)
			else RtlZeroMemory(charBuffer, sizeof(BTH_LE_GATT_CHARACTERISTIC) * charCount);
		}

		//actually retrieve the characteristics
		USHORT numChars;
		hr = BluetoothGATTGetCharacteristics(
			gloves[gloveID].pHandle,
			&serviceBuffer[0],
			charCount,
			charBuffer,
			&numChars,
			BLUETOOTH_GATT_FLAG_NONE);

		//report if there's an issue with reading the characteristics
		if (hr != S_OK)
		{
			SetLastError(hr);
			return false;
		}
#pragma endregion

#pragma region Set Notifications
		//there will only be a second characteristic if the glove has an IMU
		int chars = 1;
		if (gloves[gloveID].hasIMU) chars = 2;
		for (int ch = 0; ch < chars; ch++)
		{
			//retrieve descriptor buffer size
			USHORT descCount;
			hr = BluetoothGATTGetDescriptors(
				gloves[gloveID].pHandle,
				&charBuffer[0],
				0,
				NULL,
				&descCount,
				BLUETOOTH_GATT_FLAG_NONE);
			//something went wrong and the function isn't returning descriptor count
			if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA))
			{
				SetLastError(hr);
				return false;
			}

			PBTH_LE_GATT_DESCRIPTOR descBuffer = NULL;
			if (descCount > 0)
			{
				descBuffer = (PBTH_LE_GATT_DESCRIPTOR)malloc(sizeof(BTH_LE_GATT_DESCRIPTOR) * descCount);

				if (descBuffer == NULL)
				{
					SetLastError(E_OUTOFMEMORY);
					return false;
				}
				else RtlZeroMemory(descBuffer, sizeof(BTH_LE_GATT_DESCRIPTOR) * descCount);
			}

			//retrieve the descriptors
			USHORT numDescs;
			hr = BluetoothGATTGetDescriptors(
				gloves[gloveID].pHandle,
				&charBuffer[ch],
				descCount,
				descBuffer,
				&numDescs,
				BLUETOOTH_GATT_FLAG_NONE);

			if (hr != S_OK)
			{
				SetLastError(hr);
				return false;
			}

			printf("%d descriptors found.\n", descCount);
			PBTH_LE_GATT_DESCRIPTOR currDesc = &descBuffer[0];
			BTH_LE_GATT_DESCRIPTOR_VALUE newVal;
			RtlZeroMemory(&newVal, sizeof(newVal));
			newVal.DescriptorType = ClientCharacteristicConfiguration;
			newVal.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

			hr = BluetoothGATTSetDescriptorValue(
				gloves[gloveID].pHandle,
				currDesc,
				&newVal,
				BLUETOOTH_GATT_FLAG_NONE);

			if (hr != S_OK)
			{
				SetLastError(hr);
				return false;
			}
			else printf("Setting notification for service handle %d\n", charBuffer[ch].AttributeHandle);

			if (ch == 0) gloves[gloveID].stretchHandle = charBuffer[ch].AttributeHandle;
			else if (ch == 1) gloves[gloveID].imuHandle = charBuffer[ch].AttributeHandle;

			BTH_LE_GATT_EVENT_TYPE eType = CharacteristicValueChangedEvent;
			BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION eParam;
			eParam.NumCharacteristics = 1;
			eParam.Characteristics[0] = charBuffer[ch];

			BLUETOOTH_GATT_EVENT_HANDLE eHandle;

			hr = BluetoothGATTRegisterEvent(
				gloves[gloveID].pHandle,
				eType,
				&eParam,
				(PFNBLUETOOTH_GATT_EVENT_CALLBACK)notificationResponse,
				&gloves[gloveID].ID,
				&eHandle,
				BLUETOOTH_GATT_FLAG_NONE);

			if (hr != S_OK)
			{
				SetLastError(hr);
				return false;
			}
		}
#pragma endregion
		return true;
	}

	SMARTGLOVE_API void CALLBACK notificationResponse(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
	{
		PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;
		int gloveID = *(int*)Context;

		//the values are stored as 2-byte short integers
		//convert them with bitwise math
		if (ValueChangedEventParameters->ChangedAttributeHandle == gloves[gloveID].stretchHandle)
		{
			for (int i = 0; i < gloves[gloveID].sensorCount; i++)
			{
				gloves[gloveID].stretchRaw[i] =
					ValueChangedEventParameters->CharacteristicValue->Data[i * 2] * 256
					+ ValueChangedEventParameters->CharacteristicValue->Data[(i * 2) + 1];
				//update autocalibration limits
				if (gloves[gloveID].stretchRaw[i] < gloves[gloveID].minValues[i]) gloves[gloveID].minValues[i] = gloves[gloveID].stretchRaw[i];
				if (gloves[gloveID].stretchRaw[i] > gloves[gloveID].maxValues[i]) gloves[gloveID].maxValues[i] = gloves[gloveID].stretchRaw[i];
				//perform calibration
				//set to 0 if raw data is NaN or calibration isn't satisfactory
				if (gloves[gloveID].stretchRaw[i] != gloves[gloveID].stretchRaw[i] ||
					gloves[gloveID].minValues[i] > gloves[gloveID].maxValues[i])
					gloves[gloveID].stretch[i] = 0;
				//otherwise, use autocalibration limits to set glove
				else
					gloves[gloveID].stretch[i] = (double)(gloves[gloveID].stretchRaw[i] - gloves[gloveID].minValues[i]) / (double)(gloves[gloveID].maxValues[i] - gloves[gloveID].minValues[i]);

			}
		}
		else if (gloves[gloveID].hasIMU && ValueChangedEventParameters->ChangedAttributeHandle == gloves[gloveID].imuHandle)
		{
			for (int i = 0; i < 6; i++)
			{
				gloves[gloveID].imuRaw[i] =
					ValueChangedEventParameters->CharacteristicValue->Data[i * 2] * 256
					+ ValueChangedEventParameters->CharacteristicValue->Data[(i * 2) + 1];
			}
		}
		//record the time at which this notification occurred
		gloves[gloveID].lastNotification = std::chrono::high_resolution_clock::now();
	}

	SMARTGLOVE_API void closeConnection(int gloveID)
	{
		//TODO is this all that's required to close the BLE connection?
		CloseHandle(gloves[gloveID].pHandle);
	}

	SMARTGLOVE_API void addUUID(char* buffer, int* bufferSize)
	{
		std::string UUID = "";
		for (int i = 0; i < *bufferSize; i++)
		{
			UUID += *(buffer + i);
		}
		Glove newGlove(UUID, (int)gloves.size());
		gloves.push_back(newGlove);
	}

	SMARTGLOVE_API char* findGloves()
	{
		//TODO actually find paired smartgloves and their UUIDs
		std::string str = "";
		for each (Glove g in gloves)
		{
			str += g.UUIDstr + " ";
		}
		return &str[0];
	}

	//TODO consider just calling a main update loop that calls getData()

	SMARTGLOVE_API double* getData(int gloveID)
	{
		//allocate enough memory for xyz orientation + 15 finger joints
		std::vector<double> values(18, 0);
		//write orientation data
		values[0] = ((double)gloves[gloveID].imuRaw[0])/100.0;
		values[1] = ((double)gloves[gloveID].imuRaw[1])/100.0;
		values[2] = ((double)gloves[gloveID].imuRaw[2])/100.0;
		for (int i = 0; i < gloves[gloveID].sensorCount; i++)
		{
			// 5 SENSOR MODEL
			//   distributes curl evenly across three joints
			if (gloves[gloveID].sensorCount == 5)
			{
				values[3 + i * 3] = 
				values[3 + i * 3 + 1] = 
				values[3 + i * 3 + 2] = gloves[gloveID].stretch[i];
			}
			//10 SENSOR MODEL
			//   even i = base joint, odd i = next two joints
			else if (gloves[gloveID].sensorCount == 10)
			{
				if ((i%2)==0) values[3 + (i/2) * 3] = gloves[gloveID].stretch[i];
				else		  values[3 + (i/2) * 3 + 1] =
							  values[3 + (i/2) * 3 + 2] = gloves[gloveID].stretch[i];
			}
			//15 SENSOR MODEL
			//   sensors map directly to joints
			else
			{
				//output calibrated stretch value to the glove
				values[i + 3] = gloves[gloveID].stretch[i];
			}
		}

		//TODO split returns of IMU and stretch sensors into two functions?

		//return a pointer to the vector
		double* pntr = &values[0];
		return pntr;
	}

	SMARTGLOVE_API double getLastNotification(int gloveID)
	{
		return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - gloves[gloveID].lastNotification).count();
	}

	SMARTGLOVE_API void freePointer(char* ptr)
	{
		delete ptr;
	}

	SMARTGLOVE_API void clearCalibration(int gloveID)
	{
		gloves[gloveID].clearCalibration();
	}
	
	SMARTGLOVE_API void closeLibrary()
	{
		gloves.clear();
	}
	
	SMARTGLOVE_API void capturePose(int gloveID, char* buffer, int* bufferSize)
	{
		std::string poseName("");

		poseName = convertChar(buffer, bufferSize);

		Pose capture(poseName, gloves[gloveID].stretch/*, gloves[gloveID].imuRaw*/);
		poses.push_back(capture); //Add a check for poses with the same name?
	}

	SMARTGLOVE_API void writeOutPoses(char* buffer, int* bufferSize)
	{
		std::ofstream file;
		std::string fileName("");

		fileName = convertChar(buffer, bufferSize);
		file.open(fileName);
		
		for (std::vector<Pose>::iterator it = poses.begin(); it != poses.end(); ++it)
		{
			file << it->writeOut();
		}

		file.close();
	}

	SMARTGLOVE_API void readInPoses(std::string fileName)
	{

	}

	SMARTGLOVE_API bool checkPoseName(int gloveID, char* buffer, int* bufferSize)
	{
		Pose *pose = NULL;
		Pose *curPose = NULL;
		std::string poseName("");

		poseName = convertChar(buffer, bufferSize);
		
		// Find the first pose in the list with a matching name.
		for (std::vector<Pose>::iterator it = poses.begin(); it != poses.end(); ++it)
		{
			if (it->name == poseName)
			{
				pose = &(*it);
				break;
			}
		}

		if (pose == NULL) return false;

		// Construct a pose from the current hand data.
		curPose = new Pose(poseName, gloves[gloveID].stretch/*, gloves[gloveID].imuRaw*/);

		// Check if the two poses are within tolerances.
		if (*pose == *curPose)
		{
			return true;
		}

		return false;
	}

	std::string convertChar(char *buffer, int *bufferSize)
	{
		std::string out("");

		for (int i = 0; i < *bufferSize; i++)
		{
			out += *(buffer + i);
		}

		return out;
	}

#ifdef __cplusplus
}
#endif