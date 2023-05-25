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

PPCI_CONFIG0 returnConfigHeader(BYTE busNumber, BYTE deviceNumber, BYTE pci_eFunctionNumber){


	DWORD64 qwFuncAddr = PCI_CONFIG_START ;
	qwFuncAddr = qwFuncAddr | (busNumber << 20) | (deviceNumber << 15) | (pci_eFunctionNumber << 12);

	return (PPCI_CONFIG0)qwFuncAddr;
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


	for (int busNumber = 0; busNumber < 16; busNumber++){
		for (int deviceNumber = 0; deviceNumber < 32; deviceNumber++){
			for (int functionNumber = 0; functionNumber < 8; functionNumber++){
				PPCI_CONFIG0 current = returnConfigHeader(busNumber, deviceNumber, functionNumber);
				WORD wVendorID = _inmw((DWORD_PTR)&current->VendorID);
					if (wVendorID != 0xFFFF){

						BYTE baseClass = _inm((DWORD_PTR)&current->BaseClass);
						BYTE subClass = _inm((DWORD_PTR)&current->SubClass);
						BYTE progInterface = _inm((DWORD_PTR)&current->ProgInterface);
						WORD subsystemVendorId = _inmw((DWORD_PTR)&current->SubSystVendorID);
						WORD vendorId = _inmw((DWORD_PTR)&current->SubSystID);

						wsprintf(szBuffer[cLine++], "Bus Number: %d, Device Number: %d, Function Number: %d", busNumber, deviceNumber, functionNumber);
						wsprintf(szBuffer[cLine++], "ClassCode: %x, Sub Class Code: %x", baseClass, subClass);
						wsprintf(szBuffer[cLine++], "Programming Interface : %x, Subsystem vendorID : %x, SubsystemID : %x", progInterface, subsystemVendorId, vendorId);
						PCI_CLASS_TABLE *tab = PciClassTable;
						for (int i = 0; i < PCI_CLASS_TABLE_LEN; i++){
							
							BYTE classCode = PciClassTable[i].Class;
							BYTE subclassCode = PciClassTable[i].SubClass;
							BYTE progIf = PciClassTable[i].ProgIf;
							if ((classCode == baseClass) && (subclassCode == subClass)){
								wsprintf(szBuffer[cLine++], "Class-Subclass Descriptor : %s, Prog Interface Descriptor : %s", PciClassTable[i].ClassDesc, PciClassTable[i].ProgIfDesc);
							}
							break;
							
							//	wsprintf(szBuffer[cLine++], "Class Descriptor: , Subclass Descriptor: ,Programming Interface Descriptor: ", tab[current->BaseClass]);
						}


						for (int i = 0; i < PCI_VENTABLE_LEN; i++){
							if (wVendorID == PciVenTable[i].VenId){
								wsprintf(szBuffer[cLine++], "VenFull : %s", PciVenTable[i].VenFull);
							}
						}

						wsprintf(szBuffer[cLine++], "----------------------------------------------------------------------------------------------------------------------------------");
					}
			}
		}
	}



	// Display the messages
	DisplWindow(hWnd);

	HwClose();
	return 0;
}
