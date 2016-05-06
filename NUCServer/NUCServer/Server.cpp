#include <iostream>
#include <stdlib.h>
#include <thread>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <sstream>
#include "data.pb.h"
#include "SerialPort.h"
#pragma comment(lib, "ws2_32.lib")
#define PORT_NAME L"COM6"

void ErrorHandling(char* message);
void SendInfo(CSerialPort& port, SOCKET& hClientSock, bool& GoOn);

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
	port.Open(PORT_NAME, CBR_115200, 8, ONESTOPBIT, NOPARITY);
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



	char serialBuffer[BUFSIZ];
	char tcpBuffer[BUFSIZ];
	int i;
	float f;
	bool isRight = false;
	std::stringbuf buffer;
	std::ostream os(&buffer);
	sensor.SerializeToOstream(&os);
	port.Flush();
	try {
		while (GoOn) {
			do{
				if (!isRight) {
					do {
						port.Read(serialBuffer, 1);
					} while (*serialBuffer != '\n');
					isRight = true;
				}
				port.Read(serialBuffer, 25);
				serialBuffer[24] = '\0';
				isRight = (strncmp(serialBuffer, "temperature is ", 15) == 0);
			} while (!isRight);
			
			std::cout << serialBuffer << std::endl;
			std::string s = serialBuffer + 15;
			f = std::stof(s);
			sensor.set_temperature(f);
			sensor.SerializeToArray(tcpBuffer, BUFSIZ);
			//message send
			send(hClientSock, tcpBuffer, sensor.ByteSize(), 0);
			std::cout << "send" << std::endl;
		}
		//close socket
		closesocket(hClientSock);
		port.Close();
	}
	catch (std::exception e) {
		std::cout << "I'm catch something" << std::endl;
		closesocket(hClientSock);
		port.Close();
	}




	
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

void SendInfo(CSerialPort& port, SOCKET& hClientSock, bool& GoOn) {
	char serialBuffer[BUFSIZ];
	char tcpBuffer[BUFSIZ];
	Sensor sensor;
	int i;
	float f;
	std::stringbuf buffer;
	std::ostream os(&buffer);
	sensor.SerializeToOstream(&os);
	try {
		while (GoOn) {
			// read from port
			bool isRight = true;
			do {
				int isPoint = 0;
				for (i = 0; i < BUFSIZ; i++) {
					port.Read(serialBuffer + i, 1);
					if (serialBuffer[i] == '.') {
						isPoint++;
						if (isPoint > 1)
							break;
					}
					else if (serialBuffer[i] == '\r' || serialBuffer[i] == '\n') {
						if (isPoint) {
							serialBuffer[i] = '\0';
							break;
						}
						else {
							i = -1;
						}
					}
				}
				std::string s = serialBuffer;
				try {
					f = std::stof(s);
				}
				catch (std::invalid_argument e) {
					isRight = false;
				}
			} while (!isRight);
			sensor.set_temperature(f);
			sensor.SerializeToArray(tcpBuffer, BUFSIZ);
			//message send
			send(hClientSock, tcpBuffer, sensor.ByteSize(), 0);
			std::cout << "send" << std::endl;
			Sleep(100);
		}
		//close socket
		closesocket(hClientSock);
		port.Close();
	}
	catch (std::exception e) {
		closesocket(hClientSock);
		port.Close();
	}
}
