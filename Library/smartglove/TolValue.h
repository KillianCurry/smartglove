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

	TolValue(T const& mean);
	TolValue(T const& mean, T const &tolerance);
	
	bool inRange(T const &value); // Check if a single value is in-range of the tolerance.
	bool overlapIncl(const TolValue<T>& value); // Check if a pair of toleranced values overlap at all.
	bool overlapExcl(const TolValue<T>& value); // Check if a pair of toleranced values both have their means within each others' ranges.
	bool operator==(const TolValue<T>& value); // Overload of the default equality-check operator. Uses inclusive overlap.
	bool operator!=(const TolValue<T>& value);
};

// If no tolerance is specified, set it to 5% of the mean.
// This is not always applicable, but is useful in some situations.
template <class T>
TolValue<T>::TolValue(T const &mean)
{
	this->mean = mean;
	this->tolerance = 0.05 * mean;
}

template <class T>
TolValue<T>::TolValue(T const &mean, T const &tolerance)
{
	this->mean = mean;
	this->tolerance = tolerance;
}

// Check if an individual value is within the range.
template <class T>
bool TolValue<T>::inRange(T const &value)
{
	return (value >= mean - tolerance) && (value <= mean + tolerance);
}

// Inclusive overlap. Any overlap is fine.
template <class T>
bool TolValue<T>::overlapIncl(const TolValue<T>& value)
{
	bool temp1, temp2;
	temp1 = mean - tolerance <= value.mean + value.tolerance; // Check if the lower limit of the first value is below the upper limit of the second.
	temp2 = value.mean - value.tolerance <= mean + tolerance; // Check the same condition the other way around.
	return temp1 && temp2;
}

// Exclusive overlap. Both means must be within each others' range.
template <class T>
bool TolValue<T>::overlapExcl(const TolValue<T>& value)
{
	bool temp1, temp2;
	temp1 = (value->mean >= mean - tolerance) && (value->mean <= mean + tolerance); // Check if the other mean is within this value's range.
	temp2 = (mean >= value->mean - value->tolerance) && (mean <= value->mean + value->tolerance); // Check the same condition the other way around.
	return temp1 && temp2;
}

template <class T>
bool TolValue<T>::operator==(const TolValue<T>& value)
{
	return overlapIncl(value);
}

template <class T>
bool TolValue<T>::operator!=(const TolValue<T>& value)
{
	return !(*this == value);
}

#endif