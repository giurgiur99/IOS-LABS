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

// In the AppScroll-e.cpp source file, first call the PciBaseAddress() function to determine the base address of the PCIe extended configuration space and store the base address
// in a global variable. If the function returns 0 or 1, the base address cannot be successfully
// determined, and in this case the application should return with an error code. Otherwise, display the base address as two double-words. Next, write a function that returns a pointer to a
// PCIe function’s configuration header using the PCIe enhanced configuration mechanism. The
// function has as input parameters the bus number, device number, and PCIe function number,
// and it returns a pointer to a PCI_CONFIG0 structure containing the PCIe function’s configuration header. The enhanced configuration mechanism is described in Section 2.6.3. In this
// function, use the global variable containing the base address of the PCIe extended configuration space. Finally, use this function to search for PCIe devices on each bus between 0 and 63,
// for each device (0..31), and for each function (0..7) of a device. For each existing PCIe device, the following information should be displayed (on separate lines):

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

// Extend Application 2.7.2 to display additional information about the existing
// PCIe devices in the computer. The additional information that should be displayed is the following:
// • Vendor ID, vendor descriptor;
// • Device ID, chip descriptors.
// Use the PCI-vendor-dev.h header file that has been added to the project. To display
// the vendor descriptor, search in the PciVenTable array using the vendor ID as search key
// and display the CONST CHAR *VenFull member of the PCI_VENTABLE structure. To display
// the chip descriptors, search in the PciDevTable array using the vendor ID and device ID as
// search keys and display the CONST CHAR *Chip and CONST CHAR *ChipDesc members of
// the PCI_DEVTABLE structure.
// Notes
// • The number of entries in the PciVenTable array is defined as PCI_V


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
						wsprintf(szBuffer[cLine++], "VendorID: %d",wVendorID);
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
	
	
	
		WORD VendorID = LOWORD(readWord(0, 31, 3, 0));
		if (VendorID != 0xFFFF){
			BYTE base = HIWORD(readWord(0, 31, 3, 2)) | HIBYTE(LOWORD(readWord(0, 31, 3, 2)));
			wsprintf(szBuffer[cLine++], "BaseClass : %d", base);
			wsprintf(szBuffer[cLine++], "VendorID : %d", VendorID);
		}






	// Display the messages
	DisplWindow(hWnd);

	HwClose();
	return 0;
}
