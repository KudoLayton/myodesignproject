//#define MyoWin
//#define NUCServer
#define MAIN

//test
#include <time.h>


// include
	// MyoWin
	// Serial Communication
//	#include <windows.h>
	#include "stdafx.h"
	#include "SerialPort.h"

	// Myo Connect
	#include "DataCollector.h"

	// NUCServer
	#include <iostream>
	#include <stdlib.h>
	#include <thread>
	#include <winsock2.h>
	#include <Windows.h>
	#include <string>
	#include <sstream>
	#include <cstring>
	#include "data.pb.h"
//	#include "SerialPort.h"

// define
#define PORT_NAME L"COM5"

// pragma comment
#pragma comment(lib, "ws2_32.lib")

#ifdef MAIN
void ErrorHandling(char* message);

int main() {		// Myo, Serial, Socket
	try {
		// myo connect
		myo::Hub hub("com.example.hello-myo");
		hub.setLockingPolicy(myo::Hub::lockingPolicyNone);

		std::cout << "Attempting to find a Myo..." << std::endl;
		myo::Myo* myo = hub.waitForMyo(10000);

		if (!myo) {
			throw std::runtime_error("Unable to find a Myo!");
		}
		std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

		DataCollector collector;
		hub.addListener(&collector);


		// serial open
		CSerialPort port;
		if (!port.Open(PORT_NAME, CBR_9600, 8, ONESTOPBIT, NOPARITY))
			return 1;
		port.SetTimeout(10, 10, 1);

		// serial init
		port.Flush();
		char buff[BUFSIZ];
		int n;

		std::cout << "\nWRITE: ";
		strcpy_s(buff, "sp0.001f");
		std::cout << buff << "\n";
		n = strlen(buff);
		// write from port
		port.Write(buff, n);

		port.Read(buff, BUFSIZ);
		std::string buff1 = buff;
//		std::string pch3 = buff1.substr(0, n);
		std::cout << buff1.substr(0, n) << " (" << n << ')' << "\n";

		// socket open
		GOOGLE_PROTOBUF_VERIFY_VERSION;
		WSADATA wsaData;
		SOCKET hServerSock, hClientSock;
		SOCKADDR_IN serverAddr, clientAddr;
		int sizeClientAddr;
		int result;

		// socket open - 1. WSAStart
		std::cout << "WSAStartup" << std::endl;
		result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0)
		{
			ErrorHandling("WSAStartup() error");
		}
		std::cout << "WSAStartup finished" << std::endl;

		// socket open - 2. socket open
		std::cout << "socket open" << std::endl;
		hServerSock = socket(PF_INET, SOCK_STREAM, 0);
		if (hServerSock == INVALID_SOCKET)
		{
			ErrorHandling("socket() error");
		}
		std::cout << "socket open finished" << std::endl;

		// socket open - 3. bind socket with port
		std::cout << "bind socket" << std::endl;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(atoi("9000"));

		result = bind(hServerSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (result == SOCKET_ERROR)
		{
			ErrorHandling("bind() error");
		}
		std::cout << "bind socket finished" << std::endl;


		char tcpBuffer[BUFSIZ];
		Sensor sensor;
		std::stringbuf buffer;
		std::ostream os(&buffer);
		sensor.SerializeToOstream(&os);

		//listen
		std::cout << "listen client..." << std::endl;
		result = listen(hServerSock, 5);
		if (result == SOCKET_ERROR)
		{
			ErrorHandling("listen() error");
		}

		//accept client
		std::cout << "accept client..." << std::endl;
		sizeClientAddr = sizeof(clientAddr);
		hClientSock = accept(hServerSock, (SOCKADDR*)&clientAddr, &sizeClientAddr);
		if (hClientSock == INVALID_SOCKET)
		{
			ErrorHandling("accept() error");
		}
		std::cout << "connected" << std::endl;

		while (1) {
			// myo comm
			hub.run(1000 / 20);
			collector.print();

			char speed[7];
			_itoa_s((int)(collector.speed * 50), speed, 10);

			// serial comm
			strcpy_s(buff, "sl");
			strcat_s(buff, speed);
			strcat_s(buff, "f\0");
			n = strlen(buff);

			// write from port
			port.Write(buff, n);
			std::cout << "\nWRITE: " << buff << "\n";

			// read from port
			n = port.Read(buff, 1024);
			std::string buff1 = buff;

			buff1.erase(std::unique(buff1.begin(), buff1.end(),
				[](char a, char b) { return a == '\n' && b == '\n'; }), buff1.end());

			std::cout << "\nRead: " << buff1.substr(0, buff1.find_first_of("\n")) << "\n";
			std::string buff2 = buff1.substr(buff1.find_first_of("\n") + 1);

			if ( buff2.find("\n") != std::string::npos){
				std::cout << "\nbuff2: " << buff2.substr(0, buff2.find_first_of("\n")) << "\n";

				std::string s = buff1.find("set") != std::string::npos ?	// buff1: Set ~~
								buff2.substr(0, buff2.find_first_of("\n")) : buff1.substr(0, buff1.find_first_of("\n"));

				std::cout << "Parsed: " << s << std::endl;

				try {
					float f;

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg0(f);
						std::cout << "arg0: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg0(f);
						std::cout << "arg0: " << f << std::endl;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg1(f);
						std::cout << "arg1: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg1(f);
						std::cout << "arg1: " << f << std::endl;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg2(f);
						std::cout << "arg2: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg2(f);
						std::cout << "arg2: " << f << std::endl;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg3(f);
						std::cout << "arg3: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg3(f);
						std::cout << "arg3: " << f << std::endl;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg4(f);
						std::cout << "arg4: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg4(f);
						std::cout << "arg4: " << f << std::endl;
					}

					sensor.SerializeToArray(tcpBuffer, BUFSIZ);
					//message send
				}
				catch (std::exception e) {
					std::cout << "parsing error!" << std::endl;
				}

				try {
					send(hClientSock, tcpBuffer, sensor.ByteSize(), 0);
				}
				catch (std::exception e) {
					std::cout << "cannot send data!" << std::endl;
				}
				std::cout << "send: " << sensor.ByteSize() << std::endl;

			}
			std::cout << " (" << n << ')' << "\n";
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}
}

void ErrorHandling(char* message)
{
	fputs(message, stdout);
	fputc('\n', stdout);
	system("pause");
	exit(1);
}
#endif



#ifdef MyoWin
/*
// Serial Communication
#include <windows.h>
#include "stdafx.h"
#include "SerialPort.h"

// Myo Connect
#include "DataCollector.h"

#define PORT_NAME L"\\\\.\\COM14"

*/
int main(int argc, char** argv)
{
	int timer;

	CSerialPort port;
	if (!port.Open(PORT_NAME, CBR_9600, 8, ONESTOPBIT, NOPARITY))
		return 1;
	port.SetTimeout(10, 10, 1);

	int n;
	char buff[1024];
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

		timer = 0;
		// Finally we enter our main loop.
		while (1) {
			std::cout << time(NULL) << std::endl;
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
			hub.run(1000 / 20);
			// After processing events, we call the print() member function we defined above to print out the values we've
			// obtained from any events that have occurred.
			std::cout << time(NULL) << std::endl;
			collector.print();
			std::cout << timer;

			std::cout << "\nWRITE: ";
			char speed[7];
			_itoa_s((int)(collector.speed * 50), speed, 10);

			strcpy_s(buff, "sl");
			strcat_s(buff, speed);
			strcat_s(buff, "f\0");
			//				strcpy_s(buff, "sl00f");
			//		strcpy_s(buff, "sp0.001f\n");
			//				std::cout << buff << "\n";
			//		std::cin >> buff;

			n = strlen(buff);

			// write from port
			port.Write(buff, n);

			if (timer == 10) {
				timer = 0;

				//				Sleep(500);

				// read from port
				n = port.Read(buff, 1024);
				std::string buff1 = buff;
				std::string pch2 = buff1.substr(0, n);

				char* pch = strtok_s(buff, "\n", context);
				char* pch1 = strtok_s(*context, "\n", context);
				std::string pch3 = buff1.substr(0, n);

				//		n = port.Read((char *)bthbuff, 20);

				std::cout << pch3 << " (" << n << ')' << "\n";
				//		std::wcout << "READ: " << pch << "\n" <<  pch1 << " (" << n << ')' << '\n';
			}


			timer++;
		}

		// If a standard exception occurred, we print out its message and exit.
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}


}

#endif

#ifdef NUCServer
//#define  -D_SCL_SECURE_NO_WARNINGS 
//#pragma warning(disable:4996)
/*
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <sstream>
#include <cstring>
#include "data.pb.h"
#include "SerialPort.h"
#pragma comment(lib, "ws2_32.lib")
#define PORT_NAME L"COM3"
*/

void ErrorHandling(char* message);

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	//define variable
	WSADATA wsaData;
	SOCKET hServerSock, hClientSock;
	SOCKADDR_IN serverAddr, clientAddr;
	Sensor sensor;
	CSerialPort port;
	int sizeClientAddr;
	int n;
	int result;
	bool GoOn = true;

	//Serial port setup
	port.Open(PORT_NAME, CBR_9600, 8, ONESTOPBIT, NOPARITY);
	port.SetTimeout(10, 10, 1);



	char message[] = "Hello World!";

	//WSAStart
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		ErrorHandling("WSAStartup() error");
	}

	//socket open
	hServerSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSock == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}

	//bind socket with port
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi("9000"));

	result = bind(hServerSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}

	//listen
	result = listen(hServerSock, 5);
	if (result == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}

	//accept client
	sizeClientAddr = sizeof(clientAddr);
	hClientSock = accept(hServerSock, (SOCKADDR*)&clientAddr, &sizeClientAddr);
	if (hClientSock == INVALID_SOCKET)
	{
		ErrorHandling("accept() error");
	}
	std::cout << "connected" << std::endl;


	char serialBuffer[BUFSIZ];
	char tcpBuffer[BUFSIZ];
	int realSize = 0;
	float f;
	bool isRight = false;
	bool sec = false;
	std::string buf1;
	std::string buf2;
	std::stringbuf buffer;
	std::ostream os(&buffer);
	sensor.SerializeToOstream(&os);
	port.Flush();
	port.Read(serialBuffer, BUFSIZ);
	while (GoOn) {
		/*while (!sec) {
		if (!isRight) {
		do {
		realSize = port.Read(serialBuffer, BUFSIZ);
		strcount = 0;
		stringBuffer[0] = serialBuffer[strcount];
		strcount++;
		} while (*stringBuffer != '\n');
		isRight = true;
		}
		i = 0;
		sec = true;
		while (true) {
		do {
		if (realSize - strcount < 1) {
		realSize = port.Read(serialBuffer, BUFSIZ);
		strcount = 0;
		}
		stringBuffer[i] = serialBuffer[strcount];
		strcount++;
		} while (stringBuffer[i] < 0);
		if ((stringBuffer[i] == '\n') && i > 0) {
		stringBuffer[i] = '\0';
		break;
		}
		++i;
		}
		}
		sec = false;*/
		realSize = port.Read(serialBuffer, BUFSIZ);
		std::string buff1 = serialBuffer;
		std::string pch2 = buff1.substr(0, realSize);
		std::string s = pch2.substr(pch2.find_first_of('\n') + 1);
		s = s.substr(0, s.find_first_of('\n'));
		std::cout << "Parsed: " << s << std::endl;

		try {
			f = std::stof(s.substr(0, s.find_first_of(',')));
			sensor.set_arg0(f);
			s = s.substr(s.find_first_of(',') + 1);
			f = std::stof(s.substr(0, s.find_first_of(',')));
			sensor.set_arg1(f);
			s = s.substr(s.find_first_of(',') + 1);
			f = std::stof(s.substr(0, s.find_first_of(',')));
			sensor.set_arg2(f);
			s = s.substr(s.find_first_of(',') + 1);
			f = std::stof(s.substr(0, s.find_first_of(',')));
			sensor.set_arg3(f);
			s = s.substr(s.find_first_of(',') + 1);
			f = std::stof(s.substr(0, s.find_first_of(',')));
			sensor.set_arg4(f);
			s = s.substr(s.find_first_of(',') + 1);
			sensor.SerializeToArray(tcpBuffer, BUFSIZ);
			//message send
		}
		catch (std::exception e) {
			isRight = false;
			continue;
		}
		try {
			send(hClientSock, tcpBuffer, sensor.ByteSize(), 0);
		}
		catch (std::exception e) {
			std::cout << "cannot send data!" << std::endl;
			continue;
		}
		std::cout << "send: " << sensor.ByteSize() << std::endl;
		int j = 0;
		while (j != 100000000) {
			j++;
		}
	}
	//close socket
	closesocket(hClientSock);
	port.Close();
	closesocket(hServerSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stdout);
	fputc('\n', stdout);
	system("pause");
	exit(1);
}

#endif
