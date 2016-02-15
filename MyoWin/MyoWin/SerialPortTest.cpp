// SerialPort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SerialPort.h"
#include <windows.h>

#pragma warning (disable:4996)

int _tmain(int argc, _TCHAR* argv[])
{
	CSerialPort com1;
	 
//	com1.Open ("\\\\.\\COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	com1.SetTimeout (10, 10, 1);

	int n;
	char buff[1024];
	 
	while (1) {
		printf ("\nWRITE: ");
		scanf ("%s", buff);
		n = strlen(buff);
	 
//		com1.Write (buff, n);

		Sleep (100);
		
		n = com1.Read (buff, 1024);
	 
		printf ("READ: %s (%d)", buff, n);
	};
	 
	return 0;
}

