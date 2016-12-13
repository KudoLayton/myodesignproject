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
// send to query
#include "curl/curl.h"

#define PORT_NAME L"COM3"
//#define PORT_NAME L"\\\\.\\COM14"
#define PORT_MOTOR_COM L"COM5"
#define PORT_BTHCTRL_COM L"\\\\.\\COM19"
#define PORT_CAMMOTOR_COM L"\\\\.\\COM18"
#define PORT_MOTOR_NUC L"COM3"
#define PORT_BTHCTRL_NUC L"COM6"
#define PORT_CAMMOTOR_NUC L"COM5"

#define Car_L 15
#define Car_d 20
#define Whell_2r 3
#define rpm2cm_s (Whell_2r*3.141592/60)
#define motor_delay 20

std::string read_from_port(char *buff, float *Lmove, float *Rmove) {
	std::string buffs = buff;
	while (buffs.find_first_of('\n') == 0)
		buffs.erase(0, 1);
	//buffs.erase(std::unique(buffs.begin(), buffs.end(),
	//	[](char a, char b) { return a == '\n' && b == '\n'; }), buffs.end());
	while (buffs.find_first_of('s') == 0) {
		//std::cout << buffs.substr(0, buffs.find_first_of('\n')) << std::endl;
		buffs.erase(0, buffs.find_first_of('\n'));
		while (buffs.find_first_of('\n') == 0)
			buffs.erase(0, 1);
	}

	//std::cout << "\n buffs: " << buffs << std::endl;
	std::string buffs1 = buffs.substr(0, buffs.find_first_of('\n')+1);
	//std::cout << "\n buffs1: " << buffs1 << std::endl;
	//while(std::string buffs1 = buffs.substr(0, buffs.find_first_of('\n')) && buffs1.size() != 0)
	//do {
	//	;
	//} while (std::string buffs1 = buffs.sub.size() != 0);
	if (buffs1.length() < 2)
		return std::string(" ");

	std::string vals[6];
	for (int i = 0; i < 6; i++) {
		if (buffs1.find_first_of(',') < buffs1.length()) {
			vals[i] = buffs1.substr(0, buffs1.find_first_of(','));
			buffs1.erase(0, buffs1.find_first_of(',') + 2);
		}
		else if (buffs1.find_first_of('\n') < buffs1.length()) {
			vals[i] = buffs1.substr(0, buffs1.find_first_of('\n'));
			buffs1.erase(0, buffs1.find_first_of('\n') + 1);
		}
		else break;
	}

	std::string Lspeed = vals[0];
	std::string Rspeed = vals[1];
	int curvel_i = ((atoi(Lspeed.c_str()) + atoi(Rspeed.c_str())) / 2) / (rpm2cm_s*100);
	//std::cout << curvel_i << std::endl;
	std::string curvel = std::to_string(curvel_i);
	std::string Ldistance = vals[2];
	*Lmove = atof(Ldistance.c_str());
	std::string Rdistance = vals[3];
	*Rmove = atof(Rdistance.c_str());
	//std::string avrvel = std::to_string((atoi(Lspeed.c_str()) + atoi(Rspeed.c_str())) / 2);
	std::string temp = vals[4];
	std::string pressure = vals[5];

	//if (vals[5].string)
	//std::cout << temp << std::endl;
	temp = std::string("temp=").append(temp);
	pressure = std::string("&press=").append(pressure);
	curvel = std::string("&curvel=").append(curvel);
	std::string total = temp.append(pressure.append(curvel));

	//std::cout << total << std::endl;
	//std::cout << total.c_str() << std::endl;
	return total;
	////			std::cout << "\n buff: " << buff << std::endl;

	//std::string buff_val = buffs1.substr(0, buffs.find_first_of(','));
	//buffs1.erase(0, buffs1.find_first_of(',') + 2);
	//std::cout << "\n buff_val: " << buff_val << std::endl;
	//std::cout << "\n buffs1: " << buffs1 << std::endl;
	//std::string val = buffs1.substr(0, buffs.find_first_of(','));

	//std::string temp = "temp=" + buff_val;
	//std::cout << "\n temp: " << temp << std::endl;

	//std::string press = "&press=";
	////std::string buff_val = buffs1.substr(0, buffs.find_first_of(','));

	//std::string total;


	////			buff1.erase(std::unique(buff1.begin(), buff1.end(),
	////				[](char a, char b) { return a == '\n' && b == '\n'; }), buff1.end());
	////			if (buff1.find_first_of("\n") == 0)
	////				buff1.erase(0);
	////
	////			std::string buff2 = buff1.substr(buff1.find_first_of('\n') + 1);
	////			std::string buff3 = buff2.substr(buff2.find_first_of('\n') + 1);
	////
	////			std::cout << "Total Read: " << buff1.substr(0, n) << std::endl;
	////
	////			std::cout << "buff1: " << buff1.substr(0, buff1.find_first_of("\n")) << std::endl;
	////			std::cout << "buff2: " << buff2.substr(0, buff2.find_first_of("\n")) << std::endl;
	////
	////			if ( buff3.find("\n") != std::string::npos){
	//////				std::cout << "buff3: " << buff3.substr(0, buff3.find_first_of("\n")) << std::endl;
	////
	////				std::string s = buff1.find("set", buff1.find_first_of("\n")) != std::string::npos ?	// buff1 : set~
	////					buff2.find("set", buff2.find_first_of("\n")) != std::string::npos ? // buff2 : set~
	////						buff1.substr(0, buff1.find_first_of("\n")) :
	////						buff3.substr(0, buff3.find_first_of("\n")) :
	////						buff2.substr(0, buff2.find_first_of("\n"))	;
	////
	//////				std::string s = buff1.substr(0, buff1zz.find_first_of("\n"));
	//////				std::cout << "Parsed: " << s << std::endl;
	////			}
	//////			std::cout << " (" << n << ')' << "\n";
	////
	////std::cout << buff2 << std::endl;
	////std::string val = buff2.substr(buff2.find_first_of('=')+1, buff2.find_first_of("\n")-buff2.find_first_of('=') - 2);
	////std::cout << "val: " + val << std::endl;
	////			if (!buffn.empty()) buffn.erase('\n');
	//std::string curvel = "&curvel=";
	//std::string avrvel = "&avrvel=";
	////std::string total = temp + val + press + val + curvel + val + avrvel + val;
	//return total;
}

int main() {		// Myo, Serial, curl
	try {
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
		//myo::Hub hub("com.example.hello-myo");
		//hub.setLockingPolicy(myo::Hub::lockingPolicyNone);
		//MyoData myodata(hub);

		// serial open
		CSerialPort port;
		//if (!port.Open(PORT_NAME, CBR_115200, 8, ONESTOPBIT, NOPARITY))
		if (!port.Open(L"COM5", CBR_115200, 8, ONESTOPBIT, NOPARITY))
		//	if (!port.Open(L"\\\\.\\COM17", CBR_115200, 8, ONESTOPBIT, NOPARITY))
				return 1;
		//if (!port.Open(L"COM3", CBR_9600, 8, ONESTOPBIT, NOPARITY))
		//		return 1;
		port.SetTimeout(10, 10, 1);


		// bthctrl
		CSerialPort port_bthctrl;
		if (!port_bthctrl.Open(PORT_BTHCTRL_NUC, CBR_9600, 8, ONESTOPBIT, NOPARITY))
			if (!port_bthctrl.Open(PORT_BTHCTRL_COM, CBR_9600, 8, ONESTOPBIT, NOPARITY))
				return 1;
		port_bthctrl.SetTimeout(10, 10, 1);

		// cammotor
		CSerialPort port_cammotor;
		if (!port_cammotor.Open(PORT_CAMMOTOR_NUC, CBR_9600, 8, ONESTOPBIT, NOPARITY))
			if (!port_bthctrl.Open(PORT_BTHCTRL_COM, CBR_9600, 8, ONESTOPBIT, NOPARITY))
				return 1;
		port_cammotor.SetTimeout(10, 10, 1);




		// serial init
		port.Flush();
		char buff[BUFSIZ];
		int n;

		std::cout << "WRITE: ";
		strcpy_s(buff, "sp0.0005f");
		std::cout << buff << std::endl;
		n = strlen(buff);
		// write from port
		port.Write(buff, n);

		n = port.Read(buff, BUFSIZ);
		std::string buff1 = buff;
//		std::string pch3 = buff1.substr(0, n);
		std::cout << buff1.substr(0, n) << " (" << n << ')' << std::endl;


		float _speed = 0;
		char Lspeed[7];
		char Rspeed[7];

		float Lmove = 0;
		float Rmove = 0;

		char camtheta = 8 << 4 + 8;

		// curry
		CURL *curl;
		CURLcode res;
		curl_global_init(CURL_GLOBAL_ALL);

		while (1) {
			// myo comm
			hub.run(1000 / 20);
			collector.print();

			_speed = std::sin(collector.theta) * Car_d / Car_L;
			_itoa_s((int)((1 - _speed) * collector.speed * 20), Lspeed, 10);
			_itoa_s((int)((1 + _speed) * collector.speed * 20), Rspeed, 10);
//			std::cout << "theta: " << collector.theta << '\t' << _speed << std::endl;
//			std::cout << "\r";
			std::cout << "set speed: " << Lspeed << '\t' << Rspeed << "\t";
			//<< Lmove << "\t" << Rmove;// << std::endl;

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
			Sleep(100);

			// read from port
			n = port.Read(buff, 1024);
			std::string total = read_from_port(buff, &Lmove, &Rmove);

			if (total.length() > 1) {
				curl = curl_easy_init();
				if (curl) {
					curl_easy_setopt(curl, CURLOPT_URL, "http://lhslhg.iptime.org/input/");
					//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "temp=28.2&press=33.0");
					//if (val.size() != 0) {
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, total.c_str());
					//}
					res = curl_easy_perform(curl);
					if (res != CURLE_OK)
						fprintf(stderr, "curl_Easy_perform() failed: %s\n", curl_easy_strerror(res));
					curl_easy_cleanup(curl);
				}
			}
		}

		char **context = (char **)malloc(sizeof(char) * 1024);

		// bthctrl - cammotor
		n = port_bthctrl.Read(buff, 1024);
		char* pch = strtok_s(buff, "\n", context);
		char *ptr;

		float f[4] = { 2,2,2,2 }; //값이 안나올때 초기값을 2로잡아서 에러처리

		f[0] = strtof(pch, &ptr);
		f[1] = strtof(ptr + 1, &ptr);
		f[2] = strtof(ptr + 1, &ptr);
		f[3] = strtof(ptr + 1, &ptr);

		//		std::wcout << "READ: " << pch << " (" << n << ')' << '\n';
		std::cout << f[0] << "\t" << f[1] << "\t" << f[2] << "\t" << f[3];

		if (f[4] <= -0.3) {
			camtheta += 1;
			camtheta = (camtheta % (1 << 4)) > 15 ? 15 : camtheta;
		}
		else if (f[4] >= 0.3) {
			camtheta -= 1;
			camtheta = camtheta < 0 ? 0 : camtheta;
		}

		if (f[3] <= -0.3) {
			camtheta += 1 << 4;
			camtheta = camtheta > 15 << 4 ? 15 << 4 : camtheta;
		}
		else if (f[3] >= 0.3) {
			camtheta -= 1 << 4;
			camtheta = camtheta < 0 ? 0 : camtheta;
		}

		//		std::cin >> (char) hor >> "\t" >> (char) ver;
		//			(int)(f[4] * 8) + 8;
		port_cammotor.Write(&camtheta, sizeof(char));

		std::cout << "\t\t\t" << (int)(camtheta >> 4) << '\t' << (int)(camtheta % (1 << 4)) << "\t";

			}

	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}

	curl_global_cleanup();
}