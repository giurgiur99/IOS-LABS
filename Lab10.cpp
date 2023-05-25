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
#include "wnaspi32.h"
#include "scsidefs.h"

#define NLIN 500								// number of lines in the display window
#define NCOL 240								// number of columns in the display window

// Global variables
char szBuffer[NLIN][NCOL];						// buffer for the window contents
int  cLine;										// number of current line in the display buffer

// Declarations of external functions
void DisplWindow(HWND hWnd);



//6.10.3
void findAdapters(){
	HINSTANCE hinstWNASPI32;
	hinstWNASPI32 = LoadLibrary("WNASPI32");
	if (hinstWNASPI32 != NULL)
	{
		DWORD(*pfnGetASPI32SupportInfo)(void);
		DWORD(*pfnSendASPI32Command)(LPSRB);


		pfnGetASPI32SupportInfo = (DWORD(_cdecl*)(void))GetProcAddress(hinstWNASPI32, "GetASPI32SupportInfo");
		pfnSendASPI32Command = (DWORD(_cdecl*)(LPSRB))GetProcAddress(hinstWNASPI32, "SendASPI32Command");

		if (pfnGetASPI32SupportInfo != NULL && pfnSendASPI32Command != NULL){
			BYTE byHaCount;
			BYTE byASPIStatus;
			DWORD dwSupportInfo;
			dwSupportInfo = pfnGetASPI32SupportInfo();
			byASPIStatus = HIBYTE(LOWORD(dwSupportInfo));
			byHaCount = LOBYTE(LOWORD(dwSupportInfo));

			if (byASPIStatus != SS_COMP && byASPIStatus != SS_NO_ADAPTERS)
			{
				// Handle ASPI error here. Usually this involves the display
				// of a dialog box with an informative message.
				wsprintf(szBuffer[cLine++], "ERROR finding adapters!");
				return;
			}

			wsprintf(szBuffer[cLine++], "ASPI Adapters in the system: %d", byHaCount);
			

		}
		else{
			FreeLibrary(hinstWNASPI32);
			wsprintf(szBuffer[cLine++], "ERROR getting addresses!");
			return;
		}

		
	
	}	else{		wsprintf(szBuffer[cLine++], "ERROR loading library!");		return;	}
}

//6.10.4
void e2(){

	HINSTANCE hinstWNASPI32;
	hinstWNASPI32 = LoadLibrary("WNASPI32");
	if (hinstWNASPI32 != NULL)
	{
		DWORD(*pfnGetASPI32SupportInfo)(void);
		DWORD(*pfnSendASPI32Command)(LPSRB);


		pfnGetASPI32SupportInfo = (DWORD(_cdecl*)(void))GetProcAddress(hinstWNASPI32, "GetASPI32SupportInfo");
		pfnSendASPI32Command = (DWORD(_cdecl*)(LPSRB))GetProcAddress(hinstWNASPI32, "SendASPI32Command");

		if (pfnGetASPI32SupportInfo != NULL && pfnSendASPI32Command != NULL){
			BYTE byHaCount;
			BYTE byASPIStatus;
			DWORD dwSupportInfo;
			dwSupportInfo = pfnGetASPI32SupportInfo();
			byASPIStatus = HIBYTE(LOWORD(dwSupportInfo));
			byHaCount = LOBYTE(LOWORD(dwSupportInfo));

			if (byASPIStatus != SS_COMP && byASPIStatus != SS_NO_ADAPTERS)
			{

				wsprintf(szBuffer[cLine++], "ERROR finding adapters!");
				return;
			}

			wsprintf(szBuffer[cLine++], "ASPI Adapters in the system: %d", byHaCount);

			BYTE byHaId;
			BYTE byTarget;
			SRB_GDEVBlock srbGDEVBlock;
			
			for (byHaId = 0; byHaId < byHaCount; byHaId++)
			{
				wsprintf(szBuffer[cLine++], "-------------------------------------------------------------------------");
				SRB_HAInquiry srbHAInquiry;
				memset(&srbHAInquiry, 0, sizeof(SRB_HAInquiry));
				srbHAInquiry.SRB_Cmd = SC_HA_INQUIRY;
				srbHAInquiry.SRB_HaId = byHaId;
				pfnSendASPI32Command((LPSRB)&srbHAInquiry);
				if (srbHAInquiry.SRB_Status != SS_COMP)
				{

					wsprintf(szBuffer[cLine++], "ERROR getting info about adapter!");
					return;
				}

				//Display info
				wsprintf(szBuffer[cLine++], "SPI Manager: %s", srbHAInquiry.HA_ManagerId);
				wsprintf(szBuffer[cLine++], "SCSI identifier of host adapter: %x", srbHAInquiry.HA_SCSI_ID);
				wsprintf(szBuffer[cLine++], "Host adapter: %s", srbHAInquiry.HA_Identifier);
				wsprintf(szBuffer[cLine++], "Maximum number of SCSI devices : %x", srbHAInquiry.HA_Unique[3]);
				wsprintf(szBuffer[cLine++], "****************************************************************");
			
					for (byTarget = 0; byTarget < srbHAInquiry.HA_Unique[3]; byTarget++)
					{
						memset(&srbGDEVBlock, 0, sizeof(SRB_GDEVBlock));
						srbGDEVBlock.SRB_Cmd = SC_GET_DEV_TYPE;
						srbGDEVBlock.SRB_HaId = byHaId;
						srbGDEVBlock.SRB_Target = byTarget;
						pfnSendASPI32Command((LPSRB)&srbGDEVBlock);
						if (srbGDEVBlock.SRB_Status != SS_COMP) continue;
						if (srbGDEVBlock.SRB_DeviceType == DTYPE_CDROM)
						{
							// A CD-ROM exists at HA/ID/LUN = byHaId/byTarget/0.
							// Do whatever you want with it from here!
							wsprintf(szBuffer[cLine++], "SCSI device number: %d", byHaId);
							wsprintf(szBuffer[cLine++], "Logical unit number: %x", srbGDEVBlock.SRB_Lun);
							wsprintf(szBuffer[cLine++], "Device Type: %x", srbGDEVBlock.SRB_DeviceType);
							wsprintf(szBuffer[cLine++], "-------------------------------------------------------------------------");
						}
					}
				}


		}
		else{
			FreeLibrary(hinstWNASPI32);
			wsprintf(szBuffer[cLine++], "ERROR getting addresses!");
			return;
		}



	}	else{		wsprintf(szBuffer[cLine++], "ERROR loading library!");		return;	}


}

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

int AppScroll(HWND hWnd)
{
	int   i;

	
	char szMes1[] = "IOS Application";


	

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

	//findAdapters();
	e2();

	// Display the messages
	DisplWindow(hWnd);

	return 0;
}
