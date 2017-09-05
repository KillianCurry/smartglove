#include "Pose.h"
//#include <string>
//#include <vector>
//#include "TolValue.h"
//
//using std::vector;
//using std::string;

Pose::Pose(string name, string data)
{

}

Pose::Pose(string name, vector<double> &ArticulationMeans/*, vector<double> &OrientationMeans*/)
{
	TolValue<double> temp(100);
	this->Articulation.push_back(temp);

	//for (int i = 0; i < ArticulationMeans.size(); ++i)
	//{
	//	//TolValue<double> temp(ArticulationMeans[i]);
	//	//this->Articulation.push_back(temp);
	//}
	/*for (vector<double>::iterator it = ArticulationMeans.begin(); it != ArticulationMeans.end(); ++it)
	{
		TolValue<double> temp(*it);
		this->Articulation.push_back(temp);
	}*/
	
	//for (vector<double>::iterator it = OrientationMeans.begin(); it != OrientationMeans.end(); ++it)
	//{
	//	TolValue<double> temp(*it);
	//	this->Orientation.push_back(temp);
	//}
}

////For testing
//Pose::Pose(string name, vector<double> &ArticulationMeans/*, vector<unsigned short> &OrientationMeans*/)
//{
//	for (vector<double>::iterator it = ArticulationMeans.begin(); it != ArticulationMeans.end(); ++it)
//	{
//		TolValue<double> temp = *(new TolValue<double>(*it));
//		this->Articulation.push_back(temp);
//	}
//
//	for (vector<unsigned short>::iterator it = OrientationMeans.begin(); it != OrientationMeans.end(); ++it)
//	{
//		TolValue<unsigned short> temp = *(new TolValue<unsigned short>((unsigned short)*it));
//		this->Orientation.push_back(temp);
//	}
//}

Pose::Pose(string name, vector<TolValue<double>> &Articulation/*, vector<TolValue<unsigned short>> &Orientation*/)
{
	this->Articulation = Articulation;
	//this->Orientation = Orientation;
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

	//// Run through the vector of orientations and return false if any of the values are not within range of each other.
	//it1 = Orientation.begin();
	//it2 = curPose.Orientation.begin();
	//for (; (it1 != Orientation.end() && it2 != curPose.Orientation.end()); ++it1, ++it2)
	//{
	//	if (*it1 != *it2)
	//	{
	//		return false;
	//	}
	//}

	return true;
}

string Pose::writeOut()
{
	string output = "";
	output += "Pose:";
	output += name;
	output += ",S"; //Stretch Sensors
	output += Articulation.size();
	output += ",";
	for (vector<TolValue<double>>::iterator it = Articulation.begin(); it != Articulation.end(); ++it)
	{
		output += it->mean;
		output += ",";
		output += it->tolerance;
		output += ",";
	}
	//output += "A"; //Angles
	//output += Orientation.size();
	//output += ",";
	//for (vector<TolValue<double>>::iterator it = Orientation.begin(); it != Orientation.end(); ++it)
	//{
	//	output += it->mean;
	//	output += ",";
	//	output += it->tolerance;
	//	output += ",";
	//}
	output += '\n';
	return output;
}