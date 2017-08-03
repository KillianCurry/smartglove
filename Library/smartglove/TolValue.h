//A class to encode a value that has some mean and a range around this mean.
//The range is +/- the tolerance from the mean.
//Author: Tomas Andersen

#ifndef TOLVALUE_H
#define TOLVALUE_H

template <class T> class TolValue
{
private:
	T mean;
	T tolerance;

public:
	TolValue(T const &mean, T const &tolerance)
	bool inRange(T const &value);
}

#endif