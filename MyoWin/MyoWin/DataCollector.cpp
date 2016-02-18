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
	: referenceQuat(0,0,0,1), pos(-1,0,0), currentPose()
	//		: onArm(false), isUnlocked(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose()
{
}

// onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
// as a unit quaternion.
void DataCollector::onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
{
	using std::atan2;
	using std::asin;
	using std::sqrt;
	using std::max;
	using std::min;

	Quat = quat;

	if (isupdateReference(pos.z()))
	{
		float angle = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
			1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
		referenceQuat = Quat.fromAxisAngle(myo::Vector3<float>(0, 0, 1), angle);
		/*
		float angle = atan2(z_ref.y(), z_ref.x());

		myo::Quaternion<float> _antiYaw = myo::rotate(myo::Vector3<float>(1, 0, 0),
			myo::Vector3<float>(z_ref.x(), z_ref.y(), 0));
		//			referenceQuat = referenceQuat.conjugate();
		//			referenceQuat = quat.conjugate();
		referenceQuat = _antiYaw;
		referenceQuat = referenceQuat.conjugate();
		*/
	}

//	Quat = referenceQuat * quat;
//	Quat = Quat.normalized();

//	pos = myo::rotate(Quat, myo::Vector3<float>(-1, 0, 0));

//	Quat = referenceQuat * Quat * referenceQuat.conjugate();
//	Quat.normalized();
	pos = myo::rotate(Quat, myo::Vector3<float>(-1, 0, 0));
	pos = myo::rotate(referenceQuat.conjugate(), pos);
	pos = myo::rotate(myo::Quaternion<float>(0, 0, 1, 0), pos);
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
	myo::Vector3<int> pos_w = myo::Vector3<int>(10 + 10 * pos.x(), 10 + 10 * pos.y(), 10 + 10 * pos.z());

	int quat_w_w = 10 + Quat.w() * 10.0f;
	int quat_w_x = 10 + Quat.x() * 10.0f;
	int quat_w_y = 10 + Quat.y() * 10.0f;
	int quat_w_z = 10 + Quat.z() * 10.0f;

	std::cout << '\r';
	// Clear the current line

	myo::Quaternion<float> toOrigin = myo::rotate(pos, myo::Vector3<float>(-1, 0, 0));
	toOrigin = toOrigin * Quat * toOrigin.conjugate();
	toOrigin = toOrigin.normalized();
	myo::Quaternion<int> rotate = myo::Quaternion<int>(10 + toOrigin.x() * 10, 10 + toOrigin.y() * 10, 10 + toOrigin.z() * 10, 10 + toOrigin.w() * 10);
	/*
	int rotate = toOrigin.w() * 10;
	myo::Quaternion<float> Origin = myo::Quaternion<float>(z_ref.x(), z_ref.y(), z_ref.z(), 0);
	Origin = Quat.conjugate() * Origin * Quat;
	Origin = Origin.normalized();
	myo::Quaternion<int> rotate = myo::Quaternion<int>(10 + Origin.x() * 10, 10 + Origin.y() * 10, 10 + Origin.z() * 10, 10 + Origin.w() * 10);
	*/

	std::cout
		<< '[' << std::string(pos_w.x(), '*') << std::string(20 - pos_w.x(), ' ') << ']'
		<< '[' << std::string(pos_w.y(), '*') << std::string(20 - pos_w.y(), ' ') << ']'
		<< '[' << std::string(pos_w.z(), '*') << std::string(20 - pos_w.z(), ' ') << ']'
	/*
	std::cout
		<< '[' << std::string(rotate.x(), '*') << std::string(20 - rotate.x(), ' ') << ']'
		<< '[' << std::string(rotate.y(), '*') << std::string(20 - rotate.y(), ' ') << ']'
		<< '[' << std::string(rotate.z(), '*') << std::string(20 - rotate.z(), ' ') << ']'
	*/
		<< '[' << std::string(rotate.w(), '*') << std::string(20 - rotate.w(), ' ') << ']';
//	std::cout << std::atan2(z_ref.y(), z_ref.x()) / M_PI;


	/*
	if (isupdateReference(z_ref.z()))
		std::cout << "_update";
	else
		std::cout << "!update";
	*/




	// Print out the orientation. Orientation data is always available, even if no arm is currently recognized.
	//		std::cout.fixed;
	//		std::cout.precision(4);
	//		std::cout << pos_w.x() << '\t' << pos_w.y() << '\t' << pos_w.z() << '\t';
	/*		std::cout
	<< '[' << std::string(roll_w, '*') << std::string(18 - roll_w, ' ') << ']'
	<< '[' << std::string(pitch_w, '*') << std::string(18 - pitch_w, ' ') << ']'
	<< '[' << std::string(yaw_w, '*') << std::string(18 - yaw_w, ' ') << ']';
	*/
	/*
	if (17 < z_ref_w.z())
		std::cout << '[' << "   up   " << ']';
	else if (z_ref_w.z() < 3)
		std::cout << '[' << "  down  " << ']';
	else
		std::cout << '[' << " unknown" << ']';
	*/

	//		std::cout << std::endl;

	/*		
	std::cout
	<< '[' << std::string(quat_w_w, '*') << std::string(20 - quat_w_w, ' ') << ']'
	<< '[' << std::string(quat_w_x, '*') << std::string(20 - quat_w_x, ' ') << ']'
	<< '[' << std::string(quat_w_y, '*') << std::string(20 - quat_w_y, ' ') << ']'
	<< '[' << std::string(quat_w_z, '*') << std::string(20 - quat_w_z, ' ') << ']';
	*/
	/*
	std::cout
	<< '[' << std::string(gyro_w_x, '*') << std::string(20 - gyro_w_x, ' ') << ']'
	<< '[' << std::string(gyro_w_y, '*') << std::string(20 - gyro_w_y, ' ') << ']'
	<< '[' << std::string(gyro_w_z, '*') << std::string(20 - gyro_w_z, ' ') << ']';
	*/
	/*
	std::cout
	<< '[' << std::string(forward_w_x, '*') << std::string(20 - forward_w_x, ' ') << ']'
	<< '[' << std::string(forward_w_y, '*') << std::string(20 - forward_w_y, ' ') << ']'
	<< '[' << std::string(forward_w_z, '*') << std::string(20 - forward_w_z, ' ') << ']';
	*/
	/*	
	std::cout
		<< '[' << std::string(pos_w.x(), '*') << std::string(20 - pos_w.x(), ' ') << ']'
		<< '[' << std::string(pos_w.y(), '*') << std::string(20 - pos_w.y(), ' ') << ']'
		<< '[' << std::string(pos_w.z(), '*') << std::string(20 - pos_w.z(), ' ') << ']';
	*/

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