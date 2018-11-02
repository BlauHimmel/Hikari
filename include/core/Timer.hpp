#pragma once

#include <core\Common.hpp>
#include <chrono>

NAMESPACE_BEGIN

/**
* \brief Simple timer with millisecond precision
*
* This class is convenient for collecting performance data
*/
class Timer
{
public:
	/// Create a new timer and reset it
	Timer();

	/// Reset the timer to the current time
	void Reset();

	/// Return the number of milliseconds elapsed since the timer was last reset
	double Elapsed() const;

	/// Like \ref Elapsed(), but return a human-readable string
	std::string ElapsedString(bool bPrecise = false) const;

	/// Return the number of milliseconds elapsed since the timer was last reset and then reset it
	double Lap();

	/// Like \ref Lap(), but return a human-readable string
	std::string LapString(bool bPrecise = false);

private:
	std::chrono::system_clock::time_point m_Start;
};

NAMESPACE_END