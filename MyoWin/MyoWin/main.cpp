// Serial Communication
#include <windows.h>
#include "stdafx.h"
#include "SerialPort.h"

// Myo Connect
#include "DataCollector.h"

#define SERIAL_TESTING
#define PORT_NAME L"COM4"

int main(int argc, char** argv)
{
//	setlocale(LC_ALL, "");      //지역화 설정을 전역적으로 적용

	CSerialPort port;
	port.Open(PORT_NAME, CBR_9600, 8, ONESTOPBIT, NOPARITY);
	port.SetTimeout(10, 10, 1);

	int n;
	char buff[1024];

	while (1) {
		std::cout << "\nWRITE: ";
		std::cin >> buff;
//		printf("\nWRITE: ");
//		scanf_s("%s", buff, sizeof(buff));
		n = strlen(buff);

		port.Write(buff, n);

		Sleep(100);

		n = port.Read(buff, 1024);

		std::wcout << "READ: " << buff << " (" << n << ')';
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


#ifndef SERIAL_TESTING

	// We catch any exceptions that might occur below -- see the catch statement for more details.
	try {

		// First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
		// publishing your application. The Hub provides access to one or more Myos.
		myo::Hub hub("com.example.hello-myo");
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
