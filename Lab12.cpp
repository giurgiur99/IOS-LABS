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

//8.8.3 Write a function that sends the Start/Stop Unit command to an ATAPI drive in order to open the drive’s door.
void sendStartStop(WORD baseAddressCommand, WORD baseAddressControl, int driveNo){
	//1. define the command packet as an array of 12 bytes
	// and initialize each byte to 0 using the memset() function
	BYTE commandPacket[12];
	memset(commandPacket, 0, 12);

	//2. initialize byte 0 of the command packet with the code of the SCSI Start/Stop Unit command
	commandPacket[0] = SCSI_START_STOP_UNIT;

	//3. In byte 4 of the command packet, set the LOEJ bit to 1 and the other bits to 0 (Figure 8.22).
	commandPacket[4] = 0x2;

	//4. implement the ATAPI protocol for non - data commands, as described in Section 8.6.1
		//4.1 Software reads the Status register and waits until the BSY and DRQ bits become 0. 
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_BSY) != 0);
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_DRQ) != 0);

		//4.2. Software initializes the Device register. In the Device register, the DEV bit should be
		//reset to 0 for accessing drive 0 or it should be set to 1 for accessing drive 1.
	if (driveNo == 0)
		__outp((baseAddressCommand + ATAPI_DEVICE), 0x0);
	else
		if (driveNo == 1){
			__outp((baseAddressCommand + ATAPI_DEVICE), DEVICE_DEV);
		}

		//4.3 Software writes the code of the Packet command (0xA0) to the Command register.
	__outp((baseAddressCommand + ATAPI_COMMAND), ATA_PACKET);

		//4.4 Software reads the Status register and waits until the BSY bit becomes 0.
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_BSY) != 0);

		//4.5 Software reads the Status register and checks the DRQ bit.
	if (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_DRQ) == 0){
		wsprintf(szBuffer[cLine++], "ERROR - Command not recognised");
	}

	//NEED TO MAKE WORDS OUT OF 2 BYTES (START WITH LSB AND GO LEFT) TO SEND THE COMMAND PACKET!
		//4.6 Software writes the command packet to the Data register, one word at a time. If the
		//command packet is defined as an array of bytes, two consecutive bytes should be
		// packed in a word before writing the word to the Data register.The first byte should
		//be placed in the low part of the word, and the next byte should be placed in the high
		//part of the word.
	for (int i = 0; i  < 6; i++){
		WORD data = (commandPacket[2 * i + 1] << 8) | commandPacket[2 * i];
		 __outpw((baseAddressCommand + ATAPI_DATA), data);
	}

		//4.7 Software waits for a time corresponding to a PIO transfer cycle. For instance, it may
		//read the Alternate Status register, ignoring the read result.

		//4.8 Software reads the Status register and waits until the BSY bit becomes 0.
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_BSY) != 0);

		//4.9 Software reads the Status register and waits until the DRQ bit becomes 0
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_DRQ) == 0);
}

//8.8.5 Write a function that sends the Read TOC / PMA / ATIP command to an ATAPI drive in order to read the table of contents of an audio CD
void readAudioCD(WORD baseAddressCommand, WORD baseAddressControl, int driveNo){
	//1. define the command packet as an array of 12 bytes and the	data returned by the command as an array of 256 bytes
	//  Initialize each byte of the command packet to 0 using the memset() function
	BYTE commandPacket[12];
	memset(commandPacket, 0, 12);

	BYTE dataReturned[256];

	//2. initialize byte 0 of the command packet with the code of the SCSI Read TOC / PMA / ATIP command
	commandPacket[0] = SCSI_READ_TOC_PMA_ATIP;

	//3. In byte 1 of the command packet, set the MSF bit to 1 and the other bits to 0 (Figure 8.18).
	commandPacket[1] = 0x2;

	//4. Initialize the field for the allocation length of the command packet with the size of the buffer allocated for the data returned by the
	//	command(byte 7 should be initialized with the high - order byte of the size, and byte 8 with the low - order byte of the size).
	commandPacket[7] = HIBYTE(sizeof(dataReturned));
	commandPacket[8] = LOBYTE(sizeof(dataReturned));

	//5. Implement the ATAPI protocol for input in PIO mode, as described in Section 8.6.2.
		//5.1 Software reads the Status register and waits until the BSY and DRQ bits become 0
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_BSY) != 0);
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_DRQ) != 0);

		//5.2 Software initializes the Device register, the Byte Count Low register, and the Byte Count High register.
		//In the Device register, the DEV bit should be reset to 0 for accessing drive 0 or it should be set to 1 for accessing drive 1.
	if (driveNo == 0)
		__outp((baseAddressCommand + ATAPI_DEVICE), 0x0);
	else
		if (driveNo == 1){
			__outp((baseAddressCommand + ATAPI_DEVICE), DEVICE_DEV);
		}

		//The Byte Count registers should be loaded with the maximum number of bytes that the drive should transfer	with each assertion of the DRQ bit.
		//For the simple commands used in this laboratory work, these registers should be loaded with the total number of bytes that should be transferred for the particular command that will be issued
	__outp((baseAddressCommand + ATAPI_BYTE_CNT_LOW), LOBYTE(sizeof(dataReturned)));
	__outp((baseAddressCommand + ATAPI_BYTE_CNT_HIGH), HIBYTE(sizeof(dataReturned)));


		//5.3 Software writes the code of the Packet command (0xA0) to the Command register.	__outp((baseAddressCommand + ATAPI_COMMAND), ATA_PACKET);

		//5.4 Software reads the Status register and waits until the BSY bit becomes 0.	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_BSY) != 0);

		//5.5 Software reads the Status register and checks the DRQ bit.
	if (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_DRQ) == 0){
		wsprintf(szBuffer[cLine++], "ERROR - Command not recognised");
	}

		//5.6 Software writes the command packet to the Data register, one word at a time
	for (int i = 0; i < 6; i++){
		WORD data = (commandPacket[2 * i + 1] << 8) | commandPacket[2 * i];
		__outpw((baseAddressCommand + ATAPI_DATA), data);
	}

		//5.7 Software waits for a time corresponding to a PIO transfer cycle. 

		//5.8 Software reads the Status register and waits until the BSY bit becomes 0.
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_BSY) != 0);

		//5.9 Software reads the Status register and checks the DRQ bit. 
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_DRQ) == 0);

		//5.10 Software reads the Byte Count Low and Byte Count High registers and initializes a variable of type WORD with the contents of these registers.
		// Software transfers a data block by reading in a loop the Data register and storing it in a buffer, using as loop count the variable initialized previously.

	for (int i = 0; i < 128; i++){
		WORD data = __inpw(baseAddressCommand + ATAPI_DATA);
		dataReturned[2*i] = LOBYTE(data);
		dataReturned[2*i+1] = HIBYTE(data);
	}

		//5.11 Software waits for a time corresponding to a PIO transfer cycle.

		//5.12 Software reads the Status register and waits until the BSY bit becomes 0
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_BSY) != 0);

		//5.13 Software reads the Status register and checks the DRQ bit. 
	while (((__inp(baseAddressCommand + ATAPI_STATUS)) & STATUS_DRQ) == 0);

	//6. Display the number of the first track and the number of the last track of the audio CD(Figure 8.19).
	BYTE firstTrack = dataReturned[2];
	BYTE lastTrack = dataReturned[3];
	wsprintf(szBuffer[cLine++], "*First Track: %d", firstTrack);
	wsprintf(szBuffer[cLine++], "*Last Track: %d", lastTrack);

	//7. For each audio track display the beginning minute and second of the track(the M and S fields of the logical block address).
	int noTracks = lastTrack - firstTrack +1;

	/*for (int i = 0; i < noTracks; i++){



		wsprintf(szBuffer[cLine++], "*Beginning minute: %x", firstTrack);
		wsprintf(szBuffer[cLine++], "*Last Track: %x", lastTrack);
	}*/

}



int AppScroll(HWND hWnd)
{
	int i;

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
	//FROM LAST LAB
	DWORD firstSATA = baseAddress(0, 31, 2, 0);
	//wsprintf(szBuffer[cLine++], "First SATA:");
	//wsprintf(szBuffer[cLine++], "*Command Block: %x", HIWORD(firstSATA));
	//wsprintf(szBuffer[cLine++], "*Control Block: %x", LOWORD(firstSATA));

	//wsprintf(szBuffer[cLine++], "-------------------------------------------------------------------");


	DWORD secondSATA = baseAddress(0, 31, 2, 1);
	//wsprintf(szBuffer[cLine++], "Second SATA:");
	//wsprintf(szBuffer[cLine++], "*Command Block: %x", HIWORD(secondSATA));
	//wsprintf(szBuffer[cLine++], "*Control Block: %x", LOWORD(secondSATA));

	//wsprintf(szBuffer[cLine++], "********************************************************************");

	//executeDiagnostic(secondaryCommand);
	//wsprintf(szBuffer[cLine++], "********************************************************************");
	//identifyDevice(primaryCommand, primaryControl, 0);

	//This lab
	//sendStartStop(secondaryCommand, secondaryControl, 1);
	readAudioCD(secondaryCommand, secondaryControl, 1);

	// Display the messages
	DisplWindow(hWnd);

	HwClose();
	return 0;
}
