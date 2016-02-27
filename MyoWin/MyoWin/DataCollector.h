#pragma once

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener
{
public:
	DataCollector();
	void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat);
	void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose);
	bool isupdateReference(float z);
	void print();

	// These values are set by onOrientationData() and onPose() above.
	myo::Quaternion<float> referenceQuat;
	myo::Quaternion<float> Quat;
	float referenceRoll;

//	myo::Vector3<float> z_ref;
	myo::Vector3<float> pos;// = new myo::Vector3<float>(0.0, 0.0, 0.0);

	myo::Pose currentPose;

	enum { Up, Down, Mid } Ez;
	float theta;
	float speed;
};
