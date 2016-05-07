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
