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


//4.12.2 - Display identifier strings in HID-class devices attached to the system
void getIdentifierStrings(HWND hWnd){

	//Step 1
	_GUID GUID;
	HidD_GetHidGuid(&GUID);


	//Step 2
	HANDLE PnPHandle;
	PnPHandle = SetupDiGetClassDevs(&GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (PnPHandle == INVALID_HANDLE_VALUE){
		wsprintf(szBuffer[cLine], "Error getting info about all HID-class devices");
	}

	//Steps 3-9
	for (int i = 0; i < 20; i++){

		//Step 3
		_SP_DEVICE_INTERFACE_DATA data;
		data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		BOOL setupInterfaces = SetupDiEnumDeviceInterfaces(PnPHandle, NULL, &GUID, i, &data);

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

			//second call
			bool setupdetailInterfaces = SetupDiGetDeviceInterfaceDetail(PnPHandle, &data, detailData, reqSize, &reqSize, NULL);

			if (!setupdetailInterfaces){
				wsprintf(szBuffer[cLine++], "Error setting up detail interfaces");
				free(detailData);
				continue;
			}
		

			//Step 5
			HANDLE handler = CreateFile(detailData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (handler == INVALID_HANDLE_VALUE){
				wsprintf(szBuffer[cLine++], "Error creating file");
				free(detailData);
					continue;
			}
		
			//Step 6
			char cBuffer[256];
			HidD_GetProductString(handler, cBuffer, sizeof(cBuffer));
			wsprintf(szBuffer[cLine++], "%ls", &cBuffer);
			

			//-------4.12.6 Display aditional information about devices--------------------------------------------------------------------------------------------------------------------------------------

			//***1***
			HIDD_ATTRIBUTES attributes;
			attributes.Size = sizeof(attributes);
			if (HidD_GetAttributes(handler, &attributes)){
				wsprintf(szBuffer[cLine++], "VendorID: %x; ProductID: %x; VersionNumber: %x", attributes.VendorID, attributes.ProductID, attributes.VersionNumber);
			}

			//***2***
			PHIDP_PREPARSED_DATA phid;
			if (HidD_GetPreparsedData(handler, &phid)){
				//***3***
				HIDP_CAPS caps;
				if (HidP_GetCaps(phid, &caps) == HIDP_STATUS_SUCCESS){
					wsprintf(szBuffer[cLine++], "Usage Page: %x; Usage ID: %x; Length of input reports: %x;  Length of output reports: %x;  Length of feature reports: %x;", caps.UsagePage, caps.Usage, caps.InputReportByteLength, caps.OutputReportByteLength, caps.FeatureReportByteLength);
				}
			}
			//***4***
			HidD_FreePreparsedData(phid);
			wsprintf(szBuffer[cLine++], "---------------------------------------------------------------------------------------------------------------------------------------------------------------------");
		
			//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

			//Step 7
			free(detailData);

			//Step 8
			//Unicode
			//Need to specifie the long string "%ls", "%S" for printing

			//Step 9
			CloseHandle(handler);
				
		}
	}
	//Step 10
	//SetupDiDestroyDeviceInfoList();
	
}


//4.12.3 - Establish communication with the CP-JR ARM7 USB-LPC2148
HANDLE handler;
BOOL establishComm(HWND hWnd){

	BOOL ok;
	//Step 1
	_GUID GUID;
	HidD_GetHidGuid(&GUID);


	//Step 2
	HANDLE PnPHandle;
	PnPHandle = SetupDiGetClassDevs(&GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (PnPHandle == INVALID_HANDLE_VALUE){
		wsprintf(szBuffer[cLine], "Error getting info about all HID-class devices");
	}

	//Steps 3-9
	for (int i = 0; i < 20; i++){
		ok = true;
		//Step 3
		_SP_DEVICE_INTERFACE_DATA data;
		data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		BOOL setupInterfaces = SetupDiEnumDeviceInterfaces(PnPHandle, NULL, &GUID, i, &data);

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

			//second call
			bool setupdetailInterfaces = SetupDiGetDeviceInterfaceDetail(PnPHandle, &data, detailData, reqSize, &reqSize, NULL);

			if (!setupdetailInterfaces){
				wsprintf(szBuffer[cLine++], "Error setting up detail interfaces");
				free(detailData);
				continue;
			}


			//Step 5
			handler = CreateFile(detailData->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (handler == INVALID_HANDLE_VALUE){
				wsprintf(szBuffer[cLine++], "Error creating file");
				free(detailData);
				continue;
			}

			//Step 6
			char cBuffer[256];
			HidD_GetProductString(handler, cBuffer, sizeof(cBuffer));
			//wsprintf(szBuffer[cLine++], "%ls", &cBuffer);
			//Compare the string with the identifier string of the development board
	
			char s[] = "Keil MCB2140 HID";
			for (int cont = 0; cont < sizeof(s); cont++){
				if (s[cont] != cBuffer[cont * 2]){
					ok = false;
					break;
				}
			}
			if (ok){
				return true;
			}			


			//Step 7
			free(detailData);

			//Step 8


			//Step 9
			CloseHandle(handler);

		}
	}
	//Step 10
	//SetupDiDestroyDeviceInfoList();
	return false;
}


//4.12.3 - Read and display the status of the buttons
int readButtonStatus(){
	BYTE buf[2];
	buf[0] = 0;
	if (HidD_GetInputReport(handler, buf, sizeof(buf))){
		wsprintf(szBuffer[cLine++], "%02X", buf[1]);
		return 0;
	}
	return 1;
}




	
















int AppScroll(HWND hWnd)
{
	int   i;

	char szMes0[] = "Error initializing the Hw driver";
	char szMes1[] = "IOS Application";

	// Initialize the Hw library
/*	if (!HwOpen()) {
		wsprintf(szBuffer[0], szMes0);
		MessageBox(NULL, szBuffer[0], "HwOpen", MB_ICONSTOP);
		return 1;
	}*/

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
	getIdentifierStrings(hWnd);
	BOOL connectedDevice = establishComm(hWnd);
	if (connectedDevice){
		wsprintf(szBuffer[cLine++], "Device connected");
		readButtonStatus();
	}
	else
		wsprintf(szBuffer[cLine++], "No device found");


	// Display the messages
	DisplWindow(hWnd);

//	HwClose();
	return 0;
}
