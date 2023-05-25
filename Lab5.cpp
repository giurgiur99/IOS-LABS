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
#include "SMBus-e.h"

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

PSMBUS_PCI_CFG returnConfigHeader(BYTE busNumber, BYTE deviceNumber, BYTE pci_eFunctionNumber){


	DWORD64 qwFuncAddr = PCI_CONFIG_START;
	qwFuncAddr = qwFuncAddr | (busNumber << 20) | (deviceNumber << 15) | (pci_eFunctionNumber << 12);

	return (PSMBUS_PCI_CFG)qwFuncAddr;
}

//3.8.2
WORD getBaseAddr(){

	PSMBUS_PCI_CFG reg = returnConfigHeader(0, 31, 3);
	WORD baseAddr = LOWORD( _inmdw((DWORD_PTR)&reg->SMB_BASE)) & 0xFFFE;
	
	return baseAddr;
}

//3.8.4
int sendReceiveByte(HWND hWnd, WORD baseAddr, BYTE deviceAddr){
	
	BYTE port = __inp(baseAddr + SMB_HST_STS);

	while ((port & HST_STS_HOST_BUSY) != 0x0){
		port = __inp(baseAddr + SMB_HST_STS);
	}

	deviceAddr = (deviceAddr << 1) | 0x1;
	__outp((baseAddr + SMB_XMIT_SLVA) , deviceAddr);



	//Write in hostCntrl the command with bit start set
	__outp((baseAddr + SMB_HST_CNT), SMB_SEND_REC_BYTE | HST_CNT_START);

	BYTE regContents;
	int result;
	while (true){
		BYTE check = __inp(baseAddr + SMB_HST_STS);

		//INTR is set
		if (check & HST_STS_INTR){
			regContents = __inp(baseAddr + SMB_HST_STS);
			__outp((baseAddr + SMB_HST_STS), regContents);
			result = 0;
			break;
		}
		//Failed
		if ((check & HST_STS_FAILED) | (check & HST_STS_BUS_ERR) | (check & HST_STS_DEV_ERR)){
			regContents = __inp(baseAddr + SMB_HST_STS);
			__outp((baseAddr + SMB_HST_STS), regContents);
			result = 2;
			break;
		}

	}
	return result;
}



//3.8.6
int readByte(HWND hWnd, WORD baseAddr, BYTE deviceAddr, BYTE commandCode){

	BYTE port = __inp(baseAddr + SMB_HST_STS);

	while ((port & HST_STS_HOST_BUSY) != 0x0){
		port = __inp(baseAddr + SMB_HST_STS);
	}

	deviceAddr = (deviceAddr << 1) | 0x1;
	__outp((baseAddr + SMB_XMIT_SLVA), deviceAddr);
	__outp((baseAddr + SMB_HST_CMD), commandCode);



	//Write in hostCntrl the command with bit start set
	__outp((baseAddr + SMB_HST_CNT), SMB_WR_RD_BYTE | HST_CNT_START);

	BYTE regContents;
	int result;
	while (true){
		BYTE check = __inp(baseAddr + SMB_HST_STS);

		//INTR is set
		if (check & HST_STS_INTR){
			regContents = __inp(baseAddr + SMB_HST_STS);
			__outp((baseAddr + SMB_HST_STS), regContents);
			result = 0;
			break;
		}
		//Failed
		if ((check & HST_STS_FAILED) | (check & HST_STS_BUS_ERR) | (check & HST_STS_DEV_ERR)){
			regContents = __inp(baseAddr + SMB_HST_STS);
			__outp((baseAddr + SMB_HST_STS), regContents);
			result = 2;
			break;
		}

	}
	return result;
}

//3.8.7
void readSPDMemory(HWND hWnd, WORD baseAddr, BYTE deviceAddr){

	int res = readByte(hWnd, baseAddr, deviceAddr, 0x0);
	BYTE buf[256];
	int nbbytes = 0;
	if (res == 0){
		buf[0] = __inp(baseAddr + SMB_HST_D0);
		if ((buf[0] & 0xF) == 0x01)
			nbbytes = 128;
		else if ((buf[0] & 0xF) == 0x10)
			nbbytes = 176;
		else
			nbbytes = 256;
	}

	for (int i = 1; i < nbbytes; i++){
		if (sendReceiveByte(hWnd, baseAddr, deviceAddr) == 0){
			buf[i] = __inp(baseAddr + SMB_HST_D0);
			for (int j = 0; j < 8; j++){
				wsprintf(szBuffer[cLine++], "Offset:  %x; 8 data bytes in hexa:", buf[i]);
			}
		}
	}
}


int AppScroll(HWND hWnd)
{
	int   i;

	char szMes0[] = "Error initializing the Hw driver";
	char szMes1[] = "IOS Application";

	// Initialize the Hw library
	if (!HwOpen()) {
		wsprintf(szBuffer[0], szMes0);
		MessageBox(NULL, szBuffer[0], "HwOpen", MB_ICONSTOP);
		return 1;
	}

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

	WORD baseAddr = getBaseAddr();
	wsprintf(szBuffer[cLine++], "Base Addr : %x", baseAddr);
	
	//3.8.5
	for (WORD deviceAddr = 0x10; deviceAddr < 0x7F; deviceAddr = deviceAddr + 0x01){
		int res = sendReceiveByte(hWnd, baseAddr, deviceAddr);
		if (res == 0){
			if (deviceAddr >= 0x18 && deviceAddr <= 0x1F){
				wsprintf(szBuffer[cLine++], "SMBus Device Found ->  %x:Thermal Sensors of an SPD memory", deviceAddr);
			}
			else if (deviceAddr >= 0x30 && deviceAddr <= 0x37){
				wsprintf(szBuffer[cLine++], "SMBus Device Found ->  %x:Write protection for an SPD memory", deviceAddr);
			}
			else if (deviceAddr >= 0x40 && deviceAddr <= 0x47){
				wsprintf(szBuffer[cLine++], "SMBus Device Found ->  %x:Real-time clock", deviceAddr);
			}
			else if (deviceAddr >= 0x50 && deviceAddr <= 0x57){
				wsprintf(szBuffer[cLine++], "SMBus Device Found ->  %x:SPD memory", deviceAddr);
				readSPDMemory(hWnd, baseAddr, deviceAddr);
			}
			else
				wsprintf(szBuffer[cLine++], "SMBus Device Found ->  %x", deviceAddr);
		}	
	}



	// Display the messages
	DisplWindow(hWnd);

	HwClose();
	return 0;
}
