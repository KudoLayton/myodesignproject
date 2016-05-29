// Serial Communication
#include <windows.h>
#include "stdafx.h"
#include "SerialPort.h"
#include <iostream> //�߰���
#include <string>  //�߰���
#include <fstream> //�߰��� 

#define PORT_BTHCTRL L"\\\\.\\COM19"
#define PORT_CAMMOTOR L"\\\\.\\COM18"

int main(int argc, char** argv)
{
	//	setlocale(LC_ALL, "");      //����ȭ ������ ���������� ����

	CSerialPort port_bthctrl;
	if (!port_bthctrl.Open(PORT_BTHCTRL, CBR_9600, 8, ONESTOPBIT, NOPARITY))
		return 1;
	port_bthctrl.SetTimeout(10, 10, 1);

	CSerialPort port_cammotor;
	if (!port_cammotor.Open(PORT_CAMMOTOR, CBR_9600, 8, ONESTOPBIT, NOPARITY))
		return 1;
	port_cammotor.SetTimeout(10, 10, 1);

	int n;
	char buff[1024];
	char **context = (char **)malloc(sizeof(char) * 1024);

	int i = 0,j=0;
	while (1) {
		/*
//		std::cout << "\nWRITE: ";
//		std::cin >> buff;

//		n = strlen(buff);

		// write from port
//		port.Write(buff, n);
	*/

		Sleep(100); //100ms�� ���°� while ��ٷ��� ���α׷��� ���߰� �޴°Ŷ� ������ ���� ��ũ - �д°� �޴°Ÿ� ���°� 
			//�����ٽ����� flush����

		// read from port
		n = port_bthctrl.Read(buff, 1024);
		char* pch = strtok_s(buff, "\n", context);
		char *ptr;

		float f[4] = {2,2,2,2}; //���� �ȳ��ö� �ʱⰪ�� 2����Ƽ� ����ó��

		f[1] = strtof(pch, &ptr);
		f[2] = strtof(ptr+1, &ptr);
		f[3] = strtof(ptr+1, &ptr);
		f[4] = strtof(ptr+1, &ptr);

//		std::wcout << "READ: " << pch << " (" << n << ')' << '\n';
		std::cout << '\r' << f[1] << "\t" << f[2] << "\t" << f[3] << "\t" << f[4];
		char camtheta[2];

		if (f[3] <= -0.3) {
			camtheta[0] += 1;
			camtheta[0] = camtheta[1] > 15 ? 15 : camtheta[0];
		}
		else if (f[3] >= 0.3) {
			camtheta[0] -= 1;
			camtheta[0] = camtheta[0] < 0 ? 0 : camtheta[0];
		}
		else
			camtheta[0] = 8;


		if (f[4] <= -0.3) {
			camtheta[1] += 1;
			camtheta[1] = camtheta[1] > 15 ? 15 : camtheta[1];
		}
		else if (f[4] >= 0.3) {
			camtheta[1] -= 1;
			camtheta[1] = camtheta[1] < 0 ? 0 : camtheta[1];
		}
		else
			camtheta[1] = 8;
		//		std::cin >> (char) hor >> "\t" >> (char) ver;
//			(int)(f[4] * 8) + 8;
		port_cammotor.Write(camtheta, sizeof(char) * 2);

		std::cout << '\t' << (int)camtheta[0] << '\t' << (int)camtheta[1] << "\t";
		i++;

	};

	return 0;

}
