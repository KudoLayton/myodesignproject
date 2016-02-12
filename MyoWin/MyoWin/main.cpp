// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
	DataCollector()
		: roll_w(0), pitch_w(0), yaw_w(0), currentPose()
		//		: onArm(false), isUnlocked(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose()
	{
	}

	// onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
	/*	void onUnpair(myo::Myo* myo, uint64_t timestamp)
	{
	// We've lost a Myo.
	// Let's clean up some leftover state.
	roll_w = 0;
	pitch_w = 0;
	yaw_w = 0;
	onArm = false;
	isUnlocked = false;
	}*/

	// onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
	// as a unit quaternion.
	void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
	{
		using std::atan2;
		using std::asin;
		using std::sqrt;
		using std::max;
		using std::min;



		forward = myo::rotate(quat, myo::Vector3<float>(-1, 0, 0));
		if (isupdateReference(forward.z()))
		{
			myo::Quaternion<float> _antiYaw = myo::rotate(myo::Vector3<float>(1, 0, 0),
				myo::Vector3<float>(forward.x(), forward.y(), 0));
			//			referenceQuat = referenceQuat.conjugate();
			//			referenceQuat = quat.conjugate();
			referenceQuat = _antiYaw;
			referenceQuat = referenceQuat.conjugate();
		}


		Quat = quat;
		Quat = referenceQuat * Quat;
		Quat = Quat.normalized();

		myo::Vector3<float> pos = myo::rotate(Quat, myo::Vector3<float>(-1, 0, 0));
		pos_w = myo::Vector3<int>(10 + 10 * pos.x(), 10 + 10 * pos.y(), 10 + 10 * pos.z());


		// Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
		float roll = atan2(2.0f * (Quat.w() * Quat.x() + Quat.y() * Quat.z()),
			1.0f - 2.0f * (Quat.x() * Quat.x() + Quat.y() * Quat.y()));
		float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (Quat.w() * Quat.y() - Quat.z() * Quat.x()))));
		float yaw = atan2(2.0f * (Quat.w() * Quat.z() + Quat.x() * Quat.y()),
			1.0f - 2.0f * (Quat.y() * Quat.y() + Quat.z() * Quat.z()));

		// Convert the floating point angles in radians to a scale from 0 to 18.
		roll_w = static_cast<int>((roll + (float)M_PI) / (M_PI * 2.0f) * 18);
		pitch_w = static_cast<int>((pitch + (float)M_PI / 2.0f) / M_PI * 18);
		yaw_w = static_cast<int>((yaw + (float)M_PI) / (M_PI * 2.0f) * 18);


		/*
		using std::sin;
		myo::Vector3<float> pos = myo::rotate(quat, myo::Vector3<float>(0.0, 0.0, 1.0));
		pos_w = pos;
		myo::Vector3<float>(
		sin(pos.x() * M_PI),
		sin(pos.y() * M_PI),
		sin(pos.z() * M_PI) );
		//			if (__pos.Y > Mathf.Asin(60 * Mathf.Deg2Rad / 2) &&
		*/
	}

	// onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
	// making a fist, or not making a fist anymore.
	void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
	{
		currentPose = pose;


		if (pose != myo::Pose::unknown && pose != myo::Pose::rest) {
			//		if (true) {
			// Tell the Myo to stay unlocked until told otherwise. We do that here so you can hold the poses without the
			// Myo becoming locked.
			myo->unlock(myo::Myo::unlockHold);

			// Notify the Myo that the pose has resulted in an action, in this case changing
			// the text on the screen. The Myo will vibrate.
			myo->notifyUserAction();
		}
		else {
			// Tell the Myo to stay unlocked only for a short period. This allows the Myo to stay unlocked while poses
			// are being performed, but lock after inactivity.
			myo->unlock(myo::Myo::unlockTimed);
		}
	}


	// onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
	// arm. This lets Myo know which arm it's on and which way it's facing.
	/*	void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection, float rotation,
	myo::WarmupState warmupState)
	{
	onArm = true;
	whichArm = arm;
	}*/

	// onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
	// it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
	// when Myo is moved around on the arm.
	/*	void onArmUnsync(myo::Myo* myo, uint64_t timestamp)
	{
	onArm = false;
	}*/

	// onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
	/*	void onUnlock(myo::Myo* myo, uint64_t timestamp)
	{
	isUnlocked = true;
	}*/

	// onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
	/*	void onLock(myo::Myo* myo, uint64_t timestamp)
	{
	isUnlocked = false;
	}*/

	// There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
	// For this example, the functions overridden above are sufficient.


	//	void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
	void onGyroscopeData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& gyro)
	{
		Gyro = gyro;
	}

	bool isupdateReference(float z)
	{
		bool isupdate = false;
		//height
		if (-0.5 < z && z < 0.5)
			// finger spread
			if (currentPose == myo::Pose::fingersSpread)
				isupdate = true;

		return isupdate;
	}

	// We define this function to print the current values that were updated by the on...() functions above.
	void print()
	{
		int forward_w_x = 10 + forward.x() * 10.0f;
		int forward_w_y = 10 + forward.y() * 10.0f;
		int forward_w_z = 10 + forward.z() * 10.0f;

		int quat_w_w = 10 + Quat.w() * 10.0f;
		int quat_w_x = 10 + Quat.x() * 10.0f;
		int quat_w_y = 10 + Quat.y() * 10.0f;
		int quat_w_z = 10 + Quat.z() * 10.0f;

		int gyro_w_x = 10 + Gyro.x() / 300 * 10.0f;
		int gyro_w_y = 10 + Gyro.y() / 300 * 10.0f;
		int gyro_w_z = 10 + Gyro.z() / 300 * 10.0f;

		std::cout << '\r';
		// Clear the current line
		if (isupdateReference(forward.z()))
			std::cout << "_update";
		else
			std::cout << "!update";





		// Print out the orientation. Orientation data is always available, even if no arm is currently recognized.
		//		std::cout.fixed;
		//		std::cout.precision(4);
		//		std::cout << pos_w.x() << '\t' << pos_w.y() << '\t' << pos_w.z() << '\t';
		/*		std::cout
		<< '[' << std::string(roll_w, '*') << std::string(18 - roll_w, ' ') << ']'
		<< '[' << std::string(pitch_w, '*') << std::string(18 - pitch_w, ' ') << ']'
		<< '[' << std::string(yaw_w, '*') << std::string(18 - yaw_w, ' ') << ']';
		*/		if (17 < forward_w_z)
	std::cout << '[' << "   up   " << ']';
		else if (forward_w_z < 3)
			std::cout << '[' << "  down  " << ']';
		else
			std::cout << '[' << " unknown" << ']';


		//		std::cout << std::endl;

		/*		std::cout
		<< '[' << std::string(quat_w_w, '*') << std::string(20 - quat_w_w, ' ') << ']'
		<< '[' << std::string(quat_w_x, '*') << std::string(20 - quat_w_x, ' ') << ']'
		<< '[' << std::string(quat_w_y, '*') << std::string(20 - quat_w_y, ' ') << ']'
		<< '[' << std::string(quat_w_z, '*') << std::string(20 - quat_w_z, ' ') << ']';
		*/
		/*		std::cout
		<< '[' << std::string(gyro_w_x, '*') << std::string(20 - gyro_w_x, ' ') << ']'
		<< '[' << std::string(gyro_w_y, '*') << std::string(20 - gyro_w_y, ' ') << ']'
		<< '[' << std::string(gyro_w_z, '*') << std::string(20 - gyro_w_z, ' ') << ']';
		*//*		std::cout
		<< '[' << std::string(forward_w_x, '*') << std::string(20 - forward_w_x, ' ') << ']'
		<< '[' << std::string(forward_w_y, '*') << std::string(20 - forward_w_y, ' ') << ']'
		<< '[' << std::string(forward_w_z, '*') << std::string(20 - forward_w_z, ' ') << ']';
		*/		std::cout
			<< '[' << std::string(pos_w.x(), '*') << std::string(20 - pos_w.x(), ' ') << ']'
			<< '[' << std::string(pos_w.y(), '*') << std::string(20 - pos_w.y(), ' ') << ']'
			<< '[' << std::string(pos_w.z(), '*') << std::string(20 - pos_w.z(), ' ') << ']';

		//		std::cout << forward.x() << forward.y() << forward.z();
		/*
		if ( 15 < pitch_w )
		std::cout << "up";
		else if ( pitch_w <  3 )
		std::cout << "stop";
		else
		std::cout << "unknown";
		*/

		/*
		if (60 < pos_w.z() && pos_w.z() < 120)
		std::cout << "up";
		else if (-60 < pos_w.z() && pos_w.z() < -120)
		std::cout << "stop";
		else
		std::cout << "unknown";
		*/

		/*		if (onArm) {
		// Print out the lock state, the currently recognized pose, and which arm Myo is being worn on.

		// Pose::toString() provides the human-readable name of a pose. We can also output a Pose directly to an
		// output stream (e.g. std::cout << currentPose;). In this case we want to get the pose name's length so
		// that we can fill the rest of the field with spaces below, so we obtain it as a string using toString().
		std::string poseString = currentPose.toString();

		std::cout << '[' << (isUnlocked ? "unlocked" : "locked  ") << ']'
		<< '[' << (whichArm == myo::armLeft ? "L" : "R") << ']'
		<< '[' << poseString << std::string(14 - poseString.size(), ' ') << ']';
		}
		else {
		// Print out a placeholder for the arm and pose when Myo doesn't currently know which arm it's on.
		std::cout << '[' << std::string(8, ' ') << ']' << "[?]" << '[' << std::string(14, ' ') << ']';
		}
		*/
		std::cout << std::flush;
	}

	// These values are set by onArmSync() and onArmUnsync() above.
	//	bool onArm;
	//	myo::Arm whichArm;

	// This is set by onUnlocked() and onLocked() above.
	//	bool isUnlocked;

	// These values are set by onOrientationData() and onPose() above.
	int roll_w, pitch_w, yaw_w;
	myo::Quaternion<float> referenceQuat;
	myo::Vector3<float> forward;
	myo::Quaternion<float> Quat;
	myo::Vector3<float> Gyro;
	myo::Vector3<int> pos_w;// = new myo::Vector3<float>(0.0, 0.0, 0.0);
	myo::Pose currentPose;
};

int main(int argc, char** argv)
{
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
}
