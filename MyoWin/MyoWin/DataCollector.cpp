// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "DataCollector.h"

DataCollector::DataCollector()
	: referenceQuat(0,0,0,1), referenceRoll(0), pos(-1,0,0), Ez(Down), theta(0), speed(0), currentPose()
	//		: onArm(false), isUnlocked(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose()
{
}

// onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
// as a unit quaternion.
void DataCollector::onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
{
	Quat = quat;

	float z_theta = asin(pos.z());

//	if (z_theta > 60)
	if (pos.z() > 0.5)
		Ez = Up;
//	else if (z_theta < -60)
	else if (pos.z() < -0.5)
		Ez = Down;
//	else if ( -30 < z_theta && z_theta < 30 ){
	else {
		if (Ez != Mid){
			float angle = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
				1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
			referenceQuat = Quat.fromAxisAngle(myo::Vector3<float>(0, 0, 1), angle);

			referenceRoll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
				1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
			referenceRoll *= 10;
			referenceRoll += speed;
		}
		Ez = Mid;
	}

//	Quat = referenceQuat * quat;
//	Quat = Quat.normalized();

//	pos = myo::rotate(Quat, myo::Vector3<float>(-1, 0, 0));

//	Quat = referenceQuat * Quat * referenceQuat.conjugate();
//	Quat.normalized();
	pos = myo::rotate(Quat, myo::Vector3<float>(-1, 0, 0));
	pos = myo::rotate(referenceQuat.conjugate(), pos);
	pos = myo::rotate(myo::Quaternion<float>(0, 0, 1, 0), pos);

	if (Ez == Mid) {
		theta = atan2(pos.y(), pos.x()) * 180 / M_PI;

//		myo::Quaternion<float> toOrigin = myo::rotate(pos, myo::Vector3<float>(-1, 0, 0));
//		toOrigin = Quat * referenceQuat.conjugate();
		float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
			1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
		roll *= 10;
		speed = (int) referenceRoll - roll;
		speed = speed < 0 ? 0 : (speed > 20 ? 20 : speed);
	}

}

// onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
// making a fist, or not making a fist anymore.
void DataCollector::onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
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

bool DataCollector::isupdateReference(float z)
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
void DataCollector::print()
{
//	int z_ref_w = 10 + z_ref.z() * 10.0f;

	myo::Vector3<int> pos_w = myo::Vector3<int>(10 + 10 * pos.x(), 10 + 10 * pos.y(), 10 + 10 * pos.z());

	std::cout << '\r';
	// Clear the current line
/*
	if (Ez == Mid)
		theta = atan2(pos.y(), pos.x()) * 180 / M_PI;

	if (-15 < theta && theta < 15)
		speed = roll;
		*/
	
	std::cout << (Ez == Up ? "Set" : (Ez == Down ? "Stop" : "Change"));
	
/*	if (Ez != Down) {
		std::cout << '\t' << "angle: " << (int)theta << '\t';
		std::cout << "speed: "
//			<< '[' << std::string(1, '*') << std::string(20 - 1, ' ') << ']';
			<< '[' << std::string(speed, '*') << std::string(20 - speed, ' ') << ']';
	}
	else std::cout << "\t\t\t\t\t\t\t\t\t";
*/	// Print out the orientation. Orientation data is always available, even if no arm is currently recognized.

	std::cout
		<< '[' << std::string(pos_w.x(), '*') << std::string(20 - pos_w.x(), ' ') << ']'
		<< '[' << std::string(pos_w.y(), '*') << std::string(20 - pos_w.y(), ' ') << ']'
		<< '[' << std::string(pos_w.z(), '*') << std::string(20 - pos_w.z(), ' ') << ']';
//	<< '[' << std::string(roll, '*') << std::string(20 - roll, ' ') << ']';


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