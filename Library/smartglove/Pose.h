// Holds a hand pose, comprising stretch-sensor values
// and angles. Used primarily to hold data on specific
// poses of interest, such as a 'thumbs up'.
// Author: Tomas Andersen

#ifndef POSE_H
#define POSE_H

#include <string>
#include <vector>
#include "TolValue.h"

using std::vector;
using std::string;


class Pose
{
private:
	vector<TolValue<double>> Articulation;
	vector<TolValue<double>> Orientation;

public:
	Pose(vector<double> &ArticulationMeans, vector<double> &OrientationMeans);
	Pose(vector<TolValue<double>> &Articulation, vector<TolValue<double>> &Orientation);
	~Pose();

	bool operator==(Pose& curPose);
	string writeOut();
	void readIn(string data);
};

#endif