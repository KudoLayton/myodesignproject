#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string>
#include"data.pb.h"
#pragma comment(lib, "ws2_32.lib")

void ErrorHandling(char* message);

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	WSADATA wsaData;
	SOCKET hServerSock, hClientSock;
	SOCKADDR_IN serverAddr, clientAddr;

	Sensor sensor;
	sensor.set_temperature(23.4);
	std::string ss;
	int sizeClientAddr;
	char message[] = "Hello World!";
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		ErrorHandling("WSAStartup() error");
	}

	hServerSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSock == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi("9000"));

	result = bind(hServerSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}

	result = listen(hServerSock, 5);
	if (result == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}

	sizeClientAddr = sizeof(clientAddr);
	hClientSock = accept(hServerSock, (SOCKADDR*)&clientAddr, &sizeClientAddr);
	if (hClientSock == INVALID_SOCKET)
	{
		ErrorHandling("accept() error");
	}

	send(hClientSock, message, sizeof(message), 0);

	closesocket(hClientSock);
	closesocket(hServerSock);
	WSACleanup();

	//system("pause");
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stdout);
	fputc('\n', stdout);
	system("pause");
	exit(1);
}