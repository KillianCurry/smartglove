// A class to encode a value that has some mean and a range around this mean.
// The range is +/- the tolerance from the mean.
// Author: Tomas Andersen

#ifndef TOLVALUE_H
#define TOLVALUE_H

template <class T> class TolValue
{
public:
	T mean;
	T tolerance;

	TolValue(T const &mean);
	TolValue(T const &mean, T const &tolerance);
	
	bool inRange(T const &value); // Check if a single value is in-range of the tolerance.
	bool overlapIncl(const TolValue<T>& value); // Check if a pair of toleranced values overlap at all.
	bool overlapExcl(const TolValue<T>& value); // Check if a pair of toleranced values both have their means within each others' ranges.
	bool operator==(const TolValue<T>& value); // Overload of the default equality-check operator. Uses inclusive overlap.
};

#endif