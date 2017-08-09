#include "Pose.h"
//#include <string>
//#include <vector>
//#include "TolValue.h"
//
//using std::vector;
//using std::string;

Pose::Pose()
{

}

Pose::Pose(vector<TolValue<double>> &Articulation, vector<TolValue<double>> &Orientation)
{

}

Pose::~Pose()
{

}

bool Pose::operator==(const Pose& curPose)
{

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
}

void Pose::readIn(string data)
{

}