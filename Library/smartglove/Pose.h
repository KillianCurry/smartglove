#ifndef POSE_H
#define POSE_H

class Pose
{
private:
	vector<TolValue> Articulation;
	vector<TolValue> Orientation;

public:
	Pose();
	String writeOut();
	void readIn(string data);
}

#endif