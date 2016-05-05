#ifndef ABC


// Serial Communication
#include <windows.h>
#include "stdafx.h"
#include "SerialPort.h"

// Myo Connect
#include "DataCollector.h"

#define SERIAL_COMM			// define if serial communication mode
//#define MYO_COMMAND				// define if myo command mode
#define PORT_NAME L"\\\\.\\COM14"

int main(int argc, char** argv)
{
#ifdef SERIAL_COMM
	//	setlocale(LC_ALL, "");      //지역화 설정을 전역적으로 적용

	CSerialPort port;
	if (!port.Open(PORT_NAME, CBR_9600, 8, ONESTOPBIT, NOPARITY))
		return 1;
	port.SetTimeout(10, 10, 1);

	int n;
	char buff[1024] = {0};
	char **context = (char **)malloc(sizeof(char) * 1024);;
	int bthbuff[25];

	port.Flush();

	std::cout << "\nWRITE: ";
	strcpy_s(buff, "sp0.001f");
	std::cout << buff << "\n";
	n = strlen(buff);
	// write from port
	port.Write(buff, n);
	Sleep(1000);

	// read from port
	n = port.Read(buff, 1024);
	char* pch = strtok_s(buff, "\0", context);

	//		n = port.Read((char *)bthbuff, 20);

	std::wcout << "\nREAD: " << pch << " (" << n << ')' << '\n';

	while (1) {
		std::cout << "\nWRITE: ";
		strcpy_s(buff, "sl00f");
//		strcpy_s(buff, "sp0.001f\n");
		std::cout << buff << "\n";
		//		std::cin >> buff;

		n = strlen(buff);

		// write from port
		port.Write(buff, n);

		Sleep(500);

		// read from port
		n = port.Read(buff, 1024);
		std::string buff1 = buff;

		char* pch = strtok_s(buff, "\n", context);
		char* pch1 = strtok_s(*context, "\n", context);
		std::string pch2 = buff1.substr(0, n);

//		n = port.Read((char *)bthbuff, 20);

		std::cout << pch2 << " (" << n << ')' << "\n";
//		std::wcout << "READ: " << pch << "\n" <<  pch1 << " (" << n << ')' << '\n';
	};
	/*
	CSerialPort com1;
	com1.Open("\\\\.\\COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	com1.SetTimeout(10, 10, 1);

	int n;
	char buff[1024];

	while (1) {
	printf("\nWRITE: ");
	scanf("%s", buff);
	n = strlen(buff);

	com1.Write(buff, n);

	Sleep(100);

	n = com1.Read(buff, 1024);

	printf("READ: %s (%d)", buff, n);
	};
	*/

	return 0;
#endif

#ifdef MYO_COMMAND

	// We catch any exceptions that might occur below -- see the catch statement for more details.
	try {

		// First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
		// publishing your application. The Hub provides access to one or more Myos.
		myo::Hub hub("com.example.hello-myo");
		//		myo::Hub hub("myodesignproject");
		hub.setLockingPolicy(myo::Hub::lockingPolicyNone);

		std::cout << "Attempting to find a Myo..." << std::endl;

		// Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
		// immediately.
		// waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
		// if that fails, the function will return a null pointer.
		myo::Myo* myo = hub.waitForMyo(10000);

		// If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
		if (!myo) {
			throw std::runtime_error("Unable to find a Myo!");
		}

		// We've found a Myo.
		std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

		// Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
		DataCollector collector;

		// Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
		// Hub::run() to send events to all registered device listeners.
		hub.addListener(&collector);

		// Finally we enter our main loop.
		while (1) {
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
			hub.run(1000 / 20);
			// After processing events, we call the print() member function we defined above to print out the values we've
			// obtained from any events that have occurred.
			collector.print();
		}

		// If a standard exception occurred, we print out its message and exit.
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}

#endif

}
#endif


#ifdef DEF

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <ws2bth.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define BthName L"CTRL"

int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;

	SOCKET LocalSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	SOCKADDR_BTH    SockAddrBthLocal = { 0 };

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}	

	//
	// Open a bluetooth socket using RFCOMM protocol
	//

//	LocalSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (LocalSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
//		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}


	//
	// Setting address family to AF_BTH indicates winsock2 to use Bluetooth port
	//
	SockAddrBthLocal.addressFamily = AF_BTH;
	SockAddrBthLocal.port = BT_PORT_ANY;

	//
	// bind() associates a local address and port combination
	// with the socket just created. This is most useful when
	// the application is a server that has a well-known port
	// that clients know about in advance.
	//
	iResult = bind(LocalSocket, (struct sockaddr *) &SockAddrBthLocal, sizeof(SOCKADDR_BTH));
	if (SOCKET_ERROR == iResult) 
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(LocalSocket);
		WSACleanup();
		return 1;
	}
		
	iResult = listen(LocalSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) 
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(LocalSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(LocalSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) 
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(LocalSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(LocalSocket);

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}
#endif
