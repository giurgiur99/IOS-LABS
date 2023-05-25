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
#include <stdio.h>
#include "Hw.h"
#include "PCI-e.h"
#include "PCI-vendor-dev.h"

#define NLIN 500								// number of lines in the display window
#define NCOL 240								// number of columns in the display window

// Global variables
char szBuffer[NLIN][NCOL];						// buffer for the window contents
int  cLine;										// number of current line in the display buffer

// Declarations of external functions
void DisplWindow(HWND hWnd);


// Consider an 8-bit read/write port with the address defined by the PORT constant
// and an 8-bit mask defined by the BIT4 constant as (1 << 4). Use the __inp() and __outp()
// functions of the Marvin HW driver to write the sequences of instructions in the C language that
// perform the following operations:
// • Wait until the bit of the port defined by the BIT4 mask becomes set;
// • Set the bit of the port defined by the BIT4 mask;
// • Clear the bit of the port defined by the BIT4 mask;
// • Complement (toggle) the bit of the port defined by the BIT4 mask.
// Other bits of the port should not be altered by the sequences of instructions.
void initialize_HW()
{
	if (!HwOpen()) {
		dwLastError = GetLastError();
		printf("\nError initializing the HW library: %d", dwLastError);
		return;
	}

	printf("\nHW initialization: OK\n");
	return;
}


// In the AppScroll-e.cpp source file, write a function to initialize the COM1 serial port
// with the following parameters: binary rate of 115,200 bits/s; character length of 8 bits; no
// parity bit; 1 stop bit. Use the ComDef-e.h definition file for the serial port. Perform the following steps to initialize the serial port:
void initialize_COM1()
{
	// Open the COM1 port
	hFile = CreateFile(TEXT("COM1"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		dwLastError = GetLastError();
		printf("\nError opening the COM1 port: %d", dwLastError);
		return;
	}


	//set bit 7 of LCR to 1
	__outp(COM1 + LCR, LCR_DLAB);
	int com1_input = __inp(COM1 + LCR);

	//Write the least significant byte of the divisor to the DLR_LSB register and the most significant byte of the divisor to the DLR_MSB register
	//set the binary rate to 19,200 
	__outp(COM1 + DLR_LSB, 0x06);
	__outp(COM1 + DLR_MSB, 0x00);

	//write to LCR register a byte
	__outp(COM1 + LCR, 0x03); // 0000 0011

	//set DTR, RTS, OUT2 for MCR register
	__outp(COM1 + MCR, MCR_DTR | MCR_RTS | MCR_OUT2);
	return;

}


void send(char a){
	__outp(COM1 + THR, a);
	while ((__inp(COM1 + LSR) & LSR_THR_EMPTY) == 0);
}

char receive(){
	while ((__inp(COM1 + LSR) & LSR_DATA_READY) == 0);
	return __inp(COM1 + RBR);

}

PPCI_CONFIG0 returnConfigHeader(BYTE busNumber, BYTE deviceNumber, BYTE pci_eFunctionNumber){


	DWORD64 qwFuncAddr = PCI_CONFIG_START ;
	qwFuncAddr = qwFuncAddr | (busNumber << 20) | (deviceNumber << 15) | (pci_eFunctionNumber << 12);

	return (PPCI_CONFIG0)qwFuncAddr;
}


DWORD readWord(BYTE busNumber, BYTE deviceNumber, BYTE pci_eFunctionNumber, DWORD number){

	DWORD dwFuncAddr = dwFuncAddr | (busNumber << 16) | (deviceNumber << 11) | (pci_eFunctionNumber << 8) | (number << 2) | (1 << 31);
	__outpdw(PCI_CONFIG_ADR, dwFuncAddr);
	return  __inpdw(PCI_CONFIG_DATA);

}

int AppScroll(HWND hWnd)
{
	



	// Display the messages
	DisplWindow(hWnd);

	HwClose();
	return 0;
}
