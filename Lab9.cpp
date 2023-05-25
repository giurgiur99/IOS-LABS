//--------------------------------------------------------------------
// TUCN, Computer Science Department
// Input/Output Systems and Peripheral Devices
//--------------------------------------------------------------------
// http://users.utcluj.ro/~baruch/en/pages/teaching/inputoutput-systems/laboratory.php
//--------------------------------------------------------------------
// File:		AppScroll-e.cpp
// Date:		10.10.2015
// Modified:	21.11.2015
//--------------------------------------------------------------------
// IOS application example with vertical scroll bar
//--------------------------------------------------------------------

#include <windows.h>
#include "Hw.h"
#include "SetupAPI.h"
#include "hidsdi.h"

#define NLIN 500								// number of lines in the display window
#define NCOL 240								// number of columns in the display window

// Global variables
char szBuffer[NLIN][NCOL];						// buffer for the window contents
int  cLine;										// number of current line in the display buffer

// Declarations of external functions
void DisplWindow(HWND hWnd);

//--------------------------------------------------------------------
// Function AppScroll
//--------------------------------------------------------------------
//
// Function:	IOS application example with vertical scroll bar
//
// Parameters:	- Handle to the application window
//
// Returns:		0 - Operation completed successfully
//				1 - Error at initializing the Hw driver
//
//--------------------------------------------------------------------



//5.8.2 Communication to a USB printer
HANDLE handler;
BOOL establishComm(HWND hWnd){

	BOOL ok;
	//Step 1
	static GUID GUID_USB = { 0xA5DCBF10L, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };

	//Step 2
	HANDLE PnPHandle;
	PnPHandle = SetupDiGetClassDevs(&GUID_USB, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (PnPHandle == INVALID_HANDLE_VALUE){
		wsprintf(szBuffer[cLine], "Error getting info about printer devices");
	}

	//Steps 3-9
	for (int i = 0; i < 20; i++){
		ok = true;
		//Step 3
		_SP_DEVICE_INTERFACE_DATA data;
		data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		BOOL setupInterfaces = SetupDiEnumDeviceInterfaces(PnPHandle, NULL, &GUID_USB, i, &data);

		if (GetLastError() == ERROR_NO_MORE_ITEMS){
			break;
		}

		//Step 4
		if (setupInterfaces){

			PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;


			DWORD reqSize;

			//first call
			SetupDiGetDeviceInterfaceDetail(PnPHandle, &data, NULL, 0, &reqSize, NULL);


			detailData = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(reqSize);
			detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			SP_DEVINFO_DATA devInfo;
			devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

			//second call
			bool setupdetailInterfaces = SetupDiGetDeviceInterfaceDetail(PnPHandle, &data, detailData, reqSize, &reqSize, &devInfo);

			if (!setupdetailInterfaces){
				wsprintf(szBuffer[cLine++], "Error setting up detail interfaces");
				free(detailData);
				continue;
			}


			//Step 5
			handler = CreateFile(detailData->DevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (handler == INVALID_HANDLE_VALUE){
				wsprintf(szBuffer[cLine++], "Error creating file");
				free(detailData);
				continue;
			}
			

			//Step 6
			char buff[256];
			DWORD reqBuffSize[256];
			BOOL setupDevRegProp = SetupDiGetDeviceRegistryProperty(PnPHandle, &devInfo, SPDRP_DEVICEDESC, NULL, (PBYTE)buff, sizeof(buff), reqBuffSize);
			if (setupDevRegProp){
				//Step 7
				char s[] = "USB Printing Support";			
				if (!strcmp(buff, s))
					return true;
			}

			//Step 8
			CloseHandle(handler);
			
			
			free(detailData);
		}
	}
	
	return false;
}

//5.8.2 Print
int print(PCHAR fileName){

	handler = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (handler == INVALID_HANDLE_VALUE){
		return 1;
	}


	DWORD written, read;
	BYTE bytes[256];

	while (1){

		BOOL readok = ReadFile(handler, bytes, sizeof(bytes), &read, NULL);
		if (!readok){
			CloseHandle(handler);
			return 2;
		}
	}

	return 0;
}















int AppScroll(HWND hWnd)
{
	int   i;

	char szMes0[] = "Error initializing the Hw driver";
	char szMes1[] = "IOS Application";

	// Initialize the Hw library
	

	// Erase the display buffer and the window contents
	for (i = 0; i < NLIN; i++) {
		memset (szBuffer[i], ' ', NCOL);
	}
	cLine = 1;

	// Copy the start message into the display buffer and display the message
	wsprintf(szBuffer[cLine], szMes1);
	cLine += 2;
	DisplWindow(hWnd);

//--------------------------------------------------------------------
// To be completed with the application's code
//--------------------------------------------------------------------


	BOOL connectedDevice = establishComm(hWnd);
	if (connectedDevice){
		wsprintf(szBuffer[cLine++], "Device connected");
	}
	else
		wsprintf(szBuffer[cLine++], "No device found");


	// Display the messages
	DisplWindow(hWnd);

	
	return 0;
}
