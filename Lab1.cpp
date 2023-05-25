// Lab1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Hw.h"
#include "ComDef-e.h"

DWORD dwLastError;							// code of last error
HANDLE hFile;

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

int _tmain(int argc, _TCHAR* argv[])
{

	initialize_HW();
	initialize_COM1();



	CloseHandle(hFile);
	HwClose();
	getchar();
	
	return 0;
}

