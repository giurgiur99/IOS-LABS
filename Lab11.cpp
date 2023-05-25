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
#include <time.h>
#include "Hw.h"
#include "ATA-ATAPI-e.h"



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



PSATA_PCI_CFG returnConfigHeader(BYTE busNumber, BYTE deviceNumber, BYTE pci_eFunctionNumber){


	DWORD64 qwFuncAddr = PCI_CONFIG_START;
	qwFuncAddr = qwFuncAddr | (busNumber << 20) | (deviceNumber << 15) | (pci_eFunctionNumber << 12);

	return (PSATA_PCI_CFG)qwFuncAddr;
}

WORD primaryCommand;
WORD primaryControl;
WORD secondaryCommand;
WORD secondaryControl;

//7.10.2 - Function to determine the base address for the I/O registers of the first SATA controler
// the first three parameters are the bus number, device number, and PCIe function number of
// the SATA controller.The last parameter specifies the ATA channel for which the base addresses should be returned
DWORD baseAddress(BYTE busNumber, BYTE deviceNumber, BYTE PCIeFunctionNumber, int ATA_Channel){


	DWORD result;

	PSATA_PCI_CFG configHeader = returnConfigHeader(busNumber, deviceNumber, PCIeFunctionNumber);

	if (ATA_Channel == 0){

		primaryCommand = LOWORD(_inmdw((DWORD_PTR)&configHeader->PCMD_BAR));
		primaryControl = LOWORD(_inmdw((DWORD_PTR)&configHeader->PCNL_BAR));

		primaryCommand = primaryCommand & 0xFFFC;
		primaryControl = primaryControl & 0xFFF8;

		result = (primaryCommand << 16) | primaryControl;
	}
	if (ATA_Channel == 1){

		secondaryCommand = LOWORD(_inmdw((DWORD_PTR)&configHeader->SCMD_BAR));
		secondaryControl = LOWORD(_inmdw((DWORD_PTR)&configHeader->SCNL_BAR));

		secondaryCommand = secondaryCommand & 0xFFFC;
		secondaryControl = secondaryControl & 0xFFF8;

		result = (secondaryCommand << 16) | secondaryControl;
	}


	return result;
}

//7.10.3 Function to send the Execute Device Diagnostic command to a disk drive
void executeDiagnostic(WORD baseAddress){

	//1. wait for bus and data request
	while (((__inp(baseAddress + ATA_STATUS)) & STATUS_BSY) != 0);
	while (((__inp(baseAddress + ATA_STATUS)) & STATUS_DRQ) != 0);

	//2. clear dev bit
	__outp((baseAddress + ATA_DEVICE), 0x0);

	//3. write command code to command register
	__outp((baseAddress + ATA_COMMAND), ATA_EXEC_DEVICE_DIAG);

	//4. Software waits for at least 2 ms
	Sleep(2);

	//5. Read Status Reg and wait for BSY bit to be 0
	while (((__inp(baseAddress + ATA_STATUS)) & STATUS_BSY) != 0);

	//6. Software checks the results of the command execution.

	//Display Diagnostic
	WORD err = __inp(baseAddress + ATA_ERROR);
	wsprintf(szBuffer[cLine++], "Diagnostic Code: %02x", err);
	if (err == 0x01) wsprintf(szBuffer[cLine++], "Description: Drive 0 passed, Drive 1 passed or not present");
	if ((err == 0x00) || ((err >= 0x02) && (err <= 0x7F))) wsprintf(szBuffer[cLine++], "Description: Drive 0 failed, Drive 1 passed or not present");
	if (err == 0x81) wsprintf(szBuffer[cLine++], "Description: Drive 0 passed, Drive 1 failed");
	if ((err == 0x80) || ((err >= 0x82) && (err <= 0xFF))) wsprintf(szBuffer[cLine++], "Description:Drive 0 failed, Drive 1 failed");

	wsprintf(szBuffer[cLine++], "-------------------------------------------------------------------");
	//Check if it is ATA or ATAPI
	if ((__inp(baseAddress + ATA_SECT_CNT) == 0x01) &&
		(__inp(baseAddress + ATA_LBA_LOW) == 0x01) &&
		(__inp(baseAddress + ATA_LBA_MID) == 0x00) &&
		(__inp(baseAddress + ATA_LBA_HIGH) == 0x00))
		wsprintf(szBuffer[cLine++], "It is an ATA register");

	if ((__inp(baseAddress + ATA_SECT_CNT) == 0x01) &&
		(__inp(baseAddress + ATA_LBA_LOW) == 0x01) &&
		(__inp(baseAddress + ATA_LBA_MID) == 0x14) &&
		(__inp(baseAddress + ATA_LBA_HIGH) == 0xEB))
		wsprintf(szBuffer[cLine++], "It is an ATAPI register");
}


//7.10.5 writing a function that sends the Identify Device command to a disk drive.
void identifyDevice(WORD baseAddressCommand, WORD baseAddressControl, int driveNo){
	//1. wait for bus and data request
	while (((__inp(baseAddressCommand + ATA_STATUS)) & STATUS_BSY) != 0);
	while (((__inp(baseAddressCommand + ATA_STATUS)) & STATUS_DRQ) != 0);

	//2. Set dev bit of device register
	if (driveNo == 0)
		__outp((baseAddressCommand + ATA_DEVICE), 0x0);
	else
		if (driveNo == 1){
			__outp((baseAddressCommand + ATA_DEVICE), DEVICE_DEV);
		}

	//3. Software initializes the registers with the command parameters
	//4. Software writes the command code to the Command register. 
	__outp((baseAddressCommand + ATA_COMMAND), ATA_ID_DEVICE);

	//5. Software reads the Status register and waits until the BSY bit becomes 0.
	while (((__inp(baseAddressCommand + ATA_STATUS)) & STATUS_BSY) != 0);

	//6. Software checks the DRQ bit of the Status register. 
	if (((__inp(baseAddressCommand + ATA_STATUS)) & STATUS_DRQ) == 0){
	//do sth with error reg
	}

	//7. Software transfers a data block by reading the Data register word by word. 
	WORD data[256];
	for (int i = 0; i < 256; i++){
		data[i] = __inpw(baseAddressCommand + ATA_DATA);
	}


	//model number, total number of addressable sectors
	//	with 28 - bit LBA addressing, and total number of addressable sectors with 48 - bit LBA addressing
	wsprintf(szBuffer[cLine++], "-------------------------------------------------------------------");
	BYTE model[40];
	int a = 0;
	for (int i = 27; i <= 46; i++){
		model[a++] = HIBYTE(data[i]);
		model[a++] = LOBYTE(data[i]);
	}
	wsprintf(szBuffer[cLine++], "Model Number: %s", model);

	BYTE serial[20];
	a = 0;
	for (int i = 10; i <= 19; i++){
		serial[a++] = HIBYTE(data[i]);
		serial[a++] = LOBYTE(data[i]);
	}
	wsprintf(szBuffer[cLine++], "Serial Number: %s", serial);
	
	BYTE firmware[8];
	a = 0;
	for (int i = 23; i <= 26; i++){
		firmware[a++] = HIBYTE(data[i]);
		firmware[a++] = LOBYTE(data[i]);
	}
	wsprintf(szBuffer[cLine++], "Firmware Revision: %s", firmware);

	BYTE add28[4];
	a = 0;
	for (int i = 60; i <= 61; i++){
		add28[a++] = HIBYTE(data[i]);
		add28[a++] = LOBYTE(data[i]);
	}
	wsprintf(szBuffer[cLine++], "Total number of addresable sectors with 28	-bit LBA adressing: %s", add28);

	BYTE add48[8];
	a = 0;
	for (int i = 100; i <= 103; i++){
		add48[a++] = HIBYTE(data[i]);
		add48[a++] = LOBYTE(data[i]);
	}
	wsprintf(szBuffer[cLine++], "Total number of addresable sectors with 48	-bit LBA adressing: %s", add48);


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
	DWORD firstSATA = baseAddress(0, 31, 2, 0);
	wsprintf(szBuffer[cLine++], "First SATA:");
	wsprintf(szBuffer[cLine++], "*Command Block: %x", HIWORD(firstSATA));
	wsprintf(szBuffer[cLine++], "*Control Block: %x", LOWORD(firstSATA));

	wsprintf(szBuffer[cLine++], "-------------------------------------------------------------------");


	DWORD secondSATA = baseAddress(0, 31, 2, 1);
	wsprintf(szBuffer[cLine++], "Second SATA:");
	wsprintf(szBuffer[cLine++], "*Command Block: %x", HIWORD(secondSATA));
	wsprintf(szBuffer[cLine++], "*Control Block: %x", LOWORD(secondSATA));

	wsprintf(szBuffer[cLine++], "********************************************************************");

	executeDiagnostic(secondaryCommand);
	wsprintf(szBuffer[cLine++], "********************************************************************");
	identifyDevice(primaryCommand, primaryControl, 0);

	// Display the messages
	DisplWindow(hWnd);

	HwClose();
	return 0;
}
