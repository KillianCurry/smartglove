#include "Pose.h"
//#include <string>
//#include <vector>
//#include "TolValue.h"
//
//using std::vector;
//using std::string;

Pose::Pose(string data)
{

}

Pose::Pose(vector<double> &ArticulationMeans, vector<double> &OrientationMeans)
{
	for (vector<double>::iterator it = ArticulationMeans.begin(); it != ArticulationMeans.end(); ++it)
	{
		//TolValue<double> temp = *(new TolValue<double>(*it));
		//this->Articulation.push_back(temp);
	}
	
	for (vector<double>::iterator it = OrientationMeans.begin(); it != OrientationMeans.end(); ++it)
	{
		//TolValue<double> temp = *(new TolValue<double>(*it));
		//this->Orientation.push_back(temp);
	}
}

Pose::Pose(vector<TolValue<double>> &Articulation, vector<TolValue<double>> &Orientation)
{
	this->Articulation = Articulation;
	this->Orientation = Orientation;
}

Pose::~Pose()
{

}

bool Pose::operator==(Pose& curPose)
{
	// Run through the vector of articulations and return false if any of the values are not within range of each other.
	vector<TolValue<double>>::iterator it1 = Articulation.begin();
	vector<TolValue<double>>::iterator it2 = curPose.Articulation.begin();
	for (; (it1 != Articulation.end() && it2 != curPose.Articulation.end()); ++it1, ++it2)
	{
		if (*it1 != *it2)
		{
			return false;
		}
	}

	// Run through the vector of orientations and return false if any of the values are not within range of each other.
	it1 = Orientation.begin();
	it2 = curPose.Orientation.begin();
	for (; (it1 != Orientation.end() && it2 != curPose.Orientation.end()); ++it1, ++it2)
	{
		if (*it1 != *it2)
		{
			return false;
		}
	}

	return true;
}

string Pose::writeOut()
{
	string output = "";
	output += "Pose,";
	output += "S"; //Stretch Sensors
	output += Orientation.size();
	output += ",";
	for (vector<TolValue<double>>::iterator it = Articulation.begin(); it != Articulation.end(); ++it)
	{
		output += it->mean;
		output += ",";
		output += it->tolerance;
		output += ",";
	}
	output += "A"; //Angles
	output += Orientation.size();
	output += ",";
	for (vector<TolValue<double>>::iterator it = Orientation.begin(); it != Orientation.end(); ++it)
	{
		output += it->mean;
		output += ",";
		output += it->tolerance;
		output += ",";
	}

	return output;
}