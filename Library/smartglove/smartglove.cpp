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

#define UUID_TO_SEARCH "{00601001-7374-7265-7563-6873656e7365}"

#ifdef __cplusplus
extern "C" {
#endif
	//the raw values on the current line of data
	std::vector<double> lineValues;
	//the raw values on the last line of data
	std::vector<double> lastValues;
	//the processed values from the last line of data
	std::vector<double> calibratedValues;
	//the minimum and maximum raw values
	std::vector<double> minValues;
	std::vector<double> maxValues;
	//whether there's a calibration available for the given sensor value
	std::vector<bool> isCalibrated;
	//the current character index, to build the string
	int curChar = 0;

	int* stretch;
	int* imu;

	HANDLE pHandle;

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
	
	SMARTGLOVE_API bool establishConnection()
	{
		//TODO redesign architecture to use shorts? (values from BLE are 2-bit)
		stretch = (int*)malloc(sizeof(int) * 10);
		RtlZeroMemory(stretch, sizeof(int) * 10);
		imu = (int*)malloc(sizeof(int) * 10);
		RtlZeroMemory(imu, sizeof(int) * 10);

		//TODO get GUID from function argument string
		GUID pGUID;
		CLSIDFromString(TEXT(UUID_TO_SEARCH), &pGUID);
		pHandle = getHandle(pGUID);

		HRESULT hr;

		#pragma region Find Service

		//check how many services there are by sending blank arguments to the GetServices() function
		USHORT serviceCount;
		hr = BluetoothGATTGetServices(
			pHandle,
			0,
			NULL,
			&serviceCount,
			BLUETOOTH_GATT_FLAG_NONE);

		//something went wrong and the function isn't returning service count
		if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) printf("Issue with reading service buffer.\n");

		printf("%d services found.\n", serviceCount);

		//allocate memory for the services buffer
		PBTH_LE_GATT_SERVICE serviceBuffer = (PBTH_LE_GATT_SERVICE)malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceCount);
		//check if out of memory
		if (serviceBuffer == NULL) printf("Not enough memory for service buffer.\n");
		//otherwise, empty out the buffer (fill with zeros)
		else RtlZeroMemory(serviceBuffer, sizeof(BTH_LE_GATT_SERVICE) * serviceCount);

		//actually retrieve the services
		USHORT numServices;
		hr = BluetoothGATTGetServices(
			pHandle,
			serviceCount,
			serviceBuffer,
			&numServices,
			BLUETOOTH_GATT_FLAG_NONE);

		printf("%d services found?\n", numServices);

		//report if there's an issue with reading the services
		if (hr != S_OK) printf("Issue with reading services.");

	#pragma endregion

		#pragma region Find Characteristics
		USHORT charCount;
		hr = BluetoothGATTGetCharacteristics(
			pHandle,
			&serviceBuffer[0],
			0,
			NULL,
			&charCount,
			BLUETOOTH_GATT_FLAG_NONE);
		//something went wrong and the function isn't returning characteristic count
		if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) printf("Issue with reading characteristic buffer.\n");

		printf("%d characteristics found.\n", charCount);

		PBTH_LE_GATT_CHARACTERISTIC charBuffer = NULL;

		if (charCount > 0)
		{
			charBuffer = (PBTH_LE_GATT_CHARACTERISTIC)malloc(sizeof(BTH_LE_GATT_CHARACTERISTIC) * charCount);

			//check if out of memory
			if (charBuffer == NULL) printf("Not enough memory for characteristic buffer.\n");
			//otherwise, empty out the buffer (fill with zeros)
			else RtlZeroMemory(charBuffer, sizeof(BTH_LE_GATT_CHARACTERISTIC) * charCount);
		}

		//actually retrieve the characteristics
		USHORT numChars;
		hr = BluetoothGATTGetCharacteristics(
			pHandle,
			&serviceBuffer[0],
			charCount,
			charBuffer,
			&numChars,
			BLUETOOTH_GATT_FLAG_NONE);

		//report if there's an issue with reading the characteristics
		if (hr != S_OK) printf("Issue with reading characteristics.");

		printf("%d characteristics found?\n", numChars);
	#pragma endregion

		#pragma region Set Notifications
		for (int ch = 0; ch < 2; ch++)
		{
			//retrieve descriptor buffer size
			USHORT descCount;
			hr = BluetoothGATTGetDescriptors(
				pHandle,
				&charBuffer[0],
				0,
				NULL,
				&descCount,
				BLUETOOTH_GATT_FLAG_NONE);
			//something went wrong and the function isn't returning descriptor count
			if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) printf("Issue with reading descriptor buffer.\n");

			PBTH_LE_GATT_DESCRIPTOR descBuffer = NULL;
			if (descCount > 0)
			{
				descBuffer = (PBTH_LE_GATT_DESCRIPTOR)malloc(sizeof(BTH_LE_GATT_DESCRIPTOR) * descCount);

				if (descBuffer == NULL) printf("Not enough memory for descriptor buffer.\n");
				else RtlZeroMemory(descBuffer, sizeof(BTH_LE_GATT_DESCRIPTOR) * descCount);
			}

			//retrieve the descriptors
			USHORT numDescs;
			hr = BluetoothGATTGetDescriptors(
				pHandle,
				&charBuffer[ch],
				descCount,
				descBuffer,
				&numDescs,
				BLUETOOTH_GATT_FLAG_NONE);

			if (hr != S_OK) printf("Issue with reading descriptors.");

			printf("%d descriptors found.\n", descCount);
			PBTH_LE_GATT_DESCRIPTOR currDesc = &descBuffer[0];
			BTH_LE_GATT_DESCRIPTOR_VALUE newVal;
			RtlZeroMemory(&newVal, sizeof(newVal));
			newVal.DescriptorType = ClientCharacteristicConfiguration;
			newVal.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

			hr = BluetoothGATTSetDescriptorValue(
				pHandle,
				currDesc,
				&newVal,
				BLUETOOTH_GATT_FLAG_NONE);

			if (hr != S_OK) printf("\nERROR: Could not set descriptor value.\n");
			else printf("Setting notification for service handle %d\n", currDesc->AttributeHandle);
			printf("handle: %d\n", charBuffer[ch].AttributeHandle);

			BTH_LE_GATT_EVENT_TYPE eType = CharacteristicValueChangedEvent;
			BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION eParam;
			eParam.Characteristics[0] = charBuffer[ch];
			eParam.NumCharacteristics = 1;

			BLUETOOTH_GATT_EVENT_HANDLE eHandle;

			hr = BluetoothGATTRegisterEvent(
				pHandle,
				eType,
				&eParam,
				notificationResponse,
				NULL,
				&eHandle,
				BLUETOOTH_GATT_FLAG_NONE);

			if (hr != S_OK) printf("\nERROR: could not register notification response.\n");
		}
		#pragma endregion
		//TODO return false if an error is encountered
		return true;
	}

	SMARTGLOVE_API void notificationResponse(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
	{
		PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;

		//TODO fix to register handles on notification registration

		//point to the array we want to fill
		int* splitInts;
		//use characteristic handle to understand whether this is stretch or IMU data
		if (ValueChangedEventParameters->ChangedAttributeHandle == 13) splitInts = stretch;
		else splitInts = imu;

		//the values are stored as 2-bit short integers
		//convert them with bitwise math
		for (int i = 0; i < 10; i++)
		{
			splitInts[i] = ValueChangedEventParameters->CharacteristicValue->Data[i * 2] * 256;
			splitInts[i] += ValueChangedEventParameters->CharacteristicValue->Data[(i * 2) + 1];
		}
	}

	SMARTGLOVE_API void closeConnection()
	{
		//TODO is this all that's required to close the BLE connection?
		CloseHandle(pHandle);
	}

	SMARTGLOVE_API double* getData()
	{
		//TODO replace calibratedValues with an array, not a vector
		calibratedValues.clear();
		calibratedValues.push_back((double)imu[0]/ (double)100);
		calibratedValues.push_back((double)imu[1]/ (double)100);
		calibratedValues.push_back((double)imu[2]/ (double)100);
		for (int i = 0; i < 10; i++)
		{
			//if this sensor has been calibrated, constrain the sensor value
			//into a range of zero to one. otherwise send zero.
			if (isCalibrated.size() >= i + 1 && isCalibrated[i]) calibratedValues.push_back((double)(stretch[i] - minValues[i]) / (double)(maxValues[i] - minValues[i]));
			else calibratedValues.push_back(0);
		}

		//TODO split returns of IMU and stretch sensors into two functions?

		//return a pointer to the last line that was read
		return calibratedValues.data();
	}

	SMARTGLOVE_API void calibrateMinimum()
	{
		//TODO advanced, gesture-based calibration
		//clear previous values, if any
		minValues.clear();
		//loop through current raw sensor values
		for (int i = 0; i < 10; i++)
		{
			//set minimum to current value
			minValues.push_back(stretch[i]);
		}
		//set up maximum calibration vector
		maxValues.clear();

		for (int a = 0; a < 10; a++)
		{
			//populate vector as we go
			maxValues.push_back(0);
			isCalibrated.push_back(false);
		}
	}

	SMARTGLOVE_API void calibrateMaximum(int sensor)
	{
		//if the maximum is different from the minimum,
		//update max and complete calibration for that sensor
		if (stretch[sensor] != minValues[sensor])
		{
			maxValues[sensor] = stretch[sensor];
			isCalibrated[sensor] = true;
		}

		/*code to calibrate all at once
		//clear previous values, if any
		maxValues.clear();
		//loop through current raw sensor values
		for (int i = 0; i < 10; i++)
		{
			//populate vector as we go
			maxValues.push_back(0);
			isCalibrated.push_back(false);
			//if the maximum is different from the minimum,
			//update max and complete calibration for that sensor
			if (lastValues[i+3] != minValues[i])
			{
				maxValues[i] = lastValues[i+3];
				isCalibrated[i] = true;
			}
		}*/
	}

#ifdef __cplusplus
}
#endif