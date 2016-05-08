// Serial Communication
#include <windows.h>
#include "stdafx.h"
#include "SerialPort.h"

// Myo Connect
#include "DataCollector.h"

#define PORT_NAME L"\\\\.\\COM14"

int main(int argc, char** argv)
{
	int timer;

	CSerialPort port;
	if (!port.Open(PORT_NAME, CBR_9600, 8, ONESTOPBIT, NOPARITY))
		return 1;
	port.SetTimeout(10, 10, 1);

	int n;
	char buff[1024] = { 0 };
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
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
			hub.run(1000 / 20);
			// After processing events, we call the print() member function we defined above to print out the values we've
			// obtained from any events that have occurred.
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

			if (timer == 100) {
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
