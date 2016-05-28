// pragma comment
#pragma comment(lib, "ws2_32.lib")

// Serial Communication
#include "stdafx.h"
#include "SerialPort.h"
// Myo Connect
#include "DataCollector.h"
// NUCServer
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <sstream>
#include <cstring>
#include <stack>
#include <mutex>
#include "data.pb.h"

//#define PORT_NAME L"COM5"
//#define PORT_NAME L"\\\\.\\COM14"

#define Car_L 15
#define Car_d 20
#define Whell_r 3
#define rpm2cm_s (3*3.141592/60)
#define motor_delay 20

void ErrorHandling(char* message);
void client_connect(SOCKET, char *, Sensor *);
void client_send(SOCKET hClientSock, char *tcpBuffer, Sensor *sensor);
std::stack<std::thread*> clientThreads;
std::mutex tcpBuffer_mut;
int tcpBuffer_check = 0;
std::string sensor_parsing(Sensor *sensor, std::string s, int i, float *Lmove, float *Rmove) {
	float f = 0;

	if (i > 1)
	if (s.find(",") != std::string::npos) {
		f = std::stof(s.substr(0, s.find_first_of(',')));
		s = s.substr(s.find_first_of(',') + 1);
//		std::cout << i << ':' << s << f << std::endl;
	}
	else if (s.length() != 0) {
//		std::cout << i << ':' << s << std::endl;
		f = std::stof(s);
		s = "";
	}
	else
		f = 0;

//	std::cout << i << ':' << s << std::endl;
	f = f == 0 ? -1 : f;

	switch (i) {
	case 0: sensor->set_arg0(*Lmove * rpm2cm_s); break;		// Lmove
	case 1: sensor->set_arg1(*Rmove * rpm2cm_s); break;		// Rmove
	case 2: sensor->set_arg2(f * rpm2cm_s); break;		// set vel
	case 3: sensor->set_arg3(f * rpm2cm_s); break;		// set vel
	case 4: *Lmove += f;  sensor->set_arg4(f * rpm2cm_s); break;		// d Lmove (d = 1s)
	case 5: *Rmove += f;  sensor->set_arg5(f * rpm2cm_s); break;		// d Rmove (d = 1s)
	case 6: sensor->set_arg6(f * rpm2cm_s); break;		// tempurater
	case 7: sensor->set_arg7(f * rpm2cm_s); break;		// dummy
	default:;
	}

	return s;
}

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
//		if (!port.Open(PORT_NAME, CBR_115200, 8, ONESTOPBIT, NOPARITY))
		if (!port.Open(L"COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY))
			if (!port.Open(L"\\\\.\\COM17", CBR_115200, 8, ONESTOPBIT, NOPARITY))
				return 1;
		port.SetTimeout(10, 10, 1);

		// serial init
		port.Flush();
		char buff[BUFSIZ];
		int n;

		std::cout << "WRITE: ";
		strcpy_s(buff, "sp0.001f");
		std::cout << buff << std::endl;
		n = strlen(buff);
		// write from port
		port.Write(buff, n);

		n = port.Read(buff, BUFSIZ);
		std::string buff1 = buff;
//		std::string pch3 = buff1.substr(0, n);
		std::cout << buff1.substr(0, n) << " (" << n << ')' << std::endl;

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

		//listen
		std::cout << "listen client..." << std::endl;
		result = listen(hServerSock, 5);
		if (result == SOCKET_ERROR)
		{
			ErrorHandling("listen() error");
		}

		char tcpBuffer[BUFSIZ];
		Sensor sensor;
		std::stringbuf buffer;
		std::ostream os(&buffer);
		sensor.SerializeToOstream(&os);

		std::thread t (client_connect, hServerSock, tcpBuffer, &sensor);

		float _speed = 0;
		char Lspeed[7];
		char Rspeed[7];

		float Lmove = 0;
		float Rmove = 0;
		while (1) {
			// myo comm
			hub.run(1000 / 20);
			collector.print();

			_speed = std::sin(collector.theta) * Car_d / Car_L;
			_itoa_s((int)((1 - _speed) * collector.speed * 20), Lspeed, 10);
			_itoa_s((int)((1 + _speed) * collector.speed * 20), Rspeed, 10);
//			std::cout << "theta: " << collector.theta << '\t' << _speed << std::endl;
//			std::cout << "\r";
			std::cout << "speed: " << Lspeed << '\t' << Rspeed << "\t" << Lmove << "\t" << Rmove;// << std::endl;

			// serial comm
			strcpy_s(buff, "sl");
			strcat_s(buff, Lspeed);
			strcat_s(buff, "f\0");
			n = strlen(buff);

			// write from port
			port.Write(buff, n);
//			std::cout << "WRITE: " << buff << std::endl;


			// serial comm
			strcpy_s(buff, "sr");
			strcat_s(buff, Rspeed);
			strcat_s(buff, "f\0");
			n = strlen(buff);

			// write from port
			port.Write(buff, n);
//			std::cout << "WRITE: " << buff << std::endl;
			Sleep(20);

			// read from port
			n = port.Read(buff, 1024);
			std::string buff1 = buff;

			buff1.erase(std::unique(buff1.begin(), buff1.end(),
				[](char a, char b) { return a == '\n' && b == '\n'; }), buff1.end());
			if (buff1.find_first_of("\n") == 0)
				buff1.erase(0);

			std::string buff2 = buff1.substr(buff1.find_first_of('\n') + 1);
			std::string buff3 = buff2.substr(buff2.find_first_of('\n') + 1);
/*
			std::cout << "Total Read: " << buff1.substr(0, n) << std::endl;

			std::cout << "buff1: " << buff1.substr(0, buff1.find_first_of("\n")) << std::endl;
			std::cout << "buff2: " << buff2.substr(0, buff2.find_first_of("\n")) << std::endl;
			*/

			if ( buff3.find("\n") != std::string::npos){
//				std::cout << "buff3: " << buff3.substr(0, buff3.find_first_of("\n")) << std::endl;

				std::string s = buff1.find("set", buff1.find_first_of("\n")) != std::string::npos ?	// buff1 : set~
					buff2.find("set", buff2.find_first_of("\n")) != std::string::npos ? // buff2 : set~
						buff1.substr(0, buff1.find_first_of("\n")) :
						buff3.substr(0, buff3.find_first_of("\n")) :
						buff2.substr(0, buff2.find_first_of("\n"))	;

//				std::string s = buff1.substr(0, buff1.find_first_of("\n"));
				std::cout << "Parsed: " << s << std::endl;

				try {
/*					if( Lmove == 0 )
						sensor.set_arg0((float) (-1));
					else
						sensor.set_arg0((float) Lmove);
					if (Rmove == 0)
						sensor.set_arg1((float)(-1));
					else
						sensor.set_arg1((float) Rmove);

					std::cout << Lmove << '\t' << Rmove << std::endl;
*/
					for (int i = 0; i < 8; i++) {
//						std::cout << i << ':' << s << std::endl;
						s = sensor_parsing(&sensor, s, i, &Lmove, &Rmove);
					}
/*
					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg2(f);
//						std::cout << "arg0: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg2(f);
//						std::cout << "arg0: " << f << std::endl;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg3(f);
//						std::cout << "arg1: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg3(f);
//						std::cout << "arg1: " << f << std::endl;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						Lmove += f;
						f = f == 0 ? -1 : f;
						sensor.set_arg4(f);
//						std::cout << "arg2: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						Lmove += f;
						f = f == 0 ? -1 : f;
						sensor.set_arg4(f);
//						std::cout << "arg2: " << f << std::endl;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						Rmove += f;
						f = f == 0 ? -1 : f;
						sensor.set_arg5(f);
//						std::cout << "arg3: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						Rmove += f;
						f = f == 0 ? -1 : f;
						sensor.set_arg5(f);
//						std::cout << "arg3: " << f << std::endl;
						f = 0;
					}

					if (s.find(",") != std::string::npos) {
						f = std::stof(s.substr(0, s.find_first_of(',')));
						f = f == 0 ? -1 : f;
						sensor.set_arg6(f);
//						std::cout << "arg4: " << f << std::endl;
						s = s.substr(s.find_first_of(',') + 1);
					}
					else {
						f = std::stof(s.substr(0, s.find_first_of('\n')));
						f = f == 0 ? -1 : f;
						sensor.set_arg6(f);
//						std::cout << "arg4: " << f << std::endl;
					}

					sensor.set_arg7((float) (-1));
					*/

//					std::lock_guard<std::mutex> guard(tcpBuffer_mut);
//					tcpBuffer_mut.lock();
					while (1)
						if (tcpBuffer_mut.try_lock())
							break;
					sensor.SerializeToArray(tcpBuffer, BUFSIZ);
					tcpBuffer_mut.unlock();

					//message send
				}
				catch (std::exception e) {
					std::cout << "parsing error!" << std::endl;
					std::cout << "buff1: " << buff1.substr(0, buff1.find_first_of("\n")) << std::endl;
					std::cout << "buff2: " << buff2.substr(0, buff2.find_first_of("\n")) << std::endl;
					std::cout << "buff3: " << buff3.substr(0, buff3.find_first_of("\n")) << std::endl;
//					std::cout << " (" << n << ')' << "\n";
//					std::cout << " (" << buff1.find_first_of("\n") << ')' << "\n";
					std::cout << "Parsed: " << s << std::endl;
				}
			}
//			std::cout << " (" << n << ')' << "\n";
		}
		std::thread *p;
		while (!clientThreads.empty()) {
			p = clientThreads.top();
			p->join();
			clientThreads.pop();
		}
		t.join();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}

}

void client_connect(SOCKET hServerSock, char *tcpBuffer, Sensor *sensor) {
	SOCKET hClientSock;
	SOCKADDR_IN clientAddr;
	int sizeClientAddr;
	//accept client
	std::cout << "accept client..." << std::endl;
	sizeClientAddr = sizeof(clientAddr);
/*
	while (1) {
		std::thread t(client_send, hClientSock, tcpBuffer, sensor);
		t.detach();
	}
	*/

	while(1) {
		hClientSock = accept(hServerSock, (SOCKADDR*)&clientAddr, &sizeClientAddr);
		//	hClientSock = INVALID_SOCKET;// = accept(hServerSock, (SOCKADDR*)&clientAddr, &sizeClientAddr);
		if (hClientSock == INVALID_SOCKET)
		{
			ErrorHandling("accept() error");
		}
		std::cout << "connected" << std::endl;

		std::thread t(client_send, hClientSock, tcpBuffer, sensor);
		clientThreads.push(&t);
		t.detach();
//		t.join();
	}
}

void client_send(SOCKET hClientSock, char *tcpBuffer, Sensor *sensor) {

	while (1) {
//		for (int i=0; i < 1000; i++);
		Sleep(motor_delay);	// 20ms
//		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		try {
//			std::lock_guard<std::mutex> guard(tcpBuffer_mut);
			while (1)
				if (tcpBuffer_mut.try_lock()) 
					break;
			send(hClientSock, tcpBuffer, (*sensor).ByteSize(), 0);
			tcpBuffer_mut.unlock();
		}
		catch (std::exception e) {
			std::cout << "cannot send data!" << std::endl;
		}
		std::cout << "send: " << (*sensor).ByteSize() << std::endl;
	}

}


void ErrorHandling(char* message)
{
	fputs(message, stdout);
	fputc('\n', stdout);
	system("pause");
	exit(1);
}
