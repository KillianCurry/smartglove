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
#define CHANNELS 10

#ifdef __cplusplus
extern "C" {
#endif
	//TODO redesign architecture to use shorts? (values from BLE are 2-byte)
	//the minimum and maximum raw values
	std::vector<int> minValues(CHANNELS, INT_MAX);
	std::vector<int> maxValues(CHANNELS, INT_MIN);

	std::vector<int> stretch(CHANNELS, 0);
	std::vector<int> imu(CHANNELS, 0);

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
	
	//TODO replace mallocs with vectors
	SMARTGLOVE_API bool establishConnection()
	{
		//set values that are guaranteed to be overwritten
		for (int i = 0; i < CHANNELS; i++)
		{
			minValues[i] = INT_MAX;
			maxValues[i] = INT_MIN;
		}

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

		//point to the vector we want to fill
		std::vector<int>* splitInts;
		//use characteristic handle to understand whether this is stretch or IMU data
		if (ValueChangedEventParameters->ChangedAttributeHandle == 13) splitInts = &stretch;
		else splitInts = &imu;

		//the values are stored as 2-bit short integers
		//convert them with bitwise math
		for (int i = 0; i < 10; i++)
		{
			int val = ValueChangedEventParameters->CharacteristicValue->Data[i * 2] * 256;
			val += ValueChangedEventParameters->CharacteristicValue->Data[(i * 2) + 1];
			(*splitInts)[i] = val;
		}
	}

	SMARTGLOVE_API void closeConnection()
	{
		//TODO is this all that's required to close the BLE connection?
		CloseHandle(pHandle);
	}

	//TODO consider just calling a main update loop that calls getData()

	SMARTGLOVE_API double* getData()
	{
		//allocate enough memory for all the stretch channels + xyz orientation
		std::vector<double> values(CHANNELS + 3, 0);
		values[0] = ((double)imu[0]/ (double)100);
		values[1] = ((double)imu[1]/ (double)100);
		values[2] = ((double)imu[2]/ (double)100);
		for (int i = 0; i < CHANNELS; i++)
		{
			//if this sensor has been calibrated, constrain the sensor value
			//into a range of zero to one. otherwise send zero.
			//this is a NaN check, will only return true if the value is NaN
			if (stretch[i] != stretch[i])
			{
				values[i + 3] = 0;
				continue;
			}
			if (stretch[i] < minValues[i]) minValues[i] = stretch[i];
			if (stretch[i] > maxValues[i]) maxValues[i] = stretch[i];
			values[i + 3] = (double)(stretch[i] - minValues[i]) / (double)(maxValues[i] - minValues[i]);
		}

		//TODO split returns of IMU and stretch sensors into two functions?

		//return a pointer to the vector
		double* pntr = &values[0];
		return pntr;
	}

	SMARTGLOVE_API void clearCalibration()
	{
		//set values that are guaranteed to be overwritten
		for (int i = 0; i < CHANNELS; i++)
		{
			minValues[i] = INT_MAX;
			maxValues[i] = INT_MIN;
		}
	}

#ifdef __cplusplus
}
#endif