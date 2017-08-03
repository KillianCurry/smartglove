#include "TolValue.h"

template <class T>
TolValue<T>::TolValue(T const &mean, T const &tolerance)
{
	this->mean = mean;
	this->tolerance = tolerance;
}

template <class T>
bool TolValue<T>::inRange(T const &value)
{
	if ((value >= mean - tolerance) && (value <= mean + tolerance))
	{
		return true;
	}
	else
	{
		return false;
	}
}