#include <core\Timer.hpp>

NAMESPACE_BEGIN

Timer::Timer()
{
	Reset();
}

void Timer::Reset()
{
	m_Start = std::chrono::system_clock::now();
}

double Timer::Elapsed() const
{
	auto Now = std::chrono::system_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::milliseconds>(Now - m_Start);
	return double(Duration.count());
}

std::string Timer::ElapsedString(bool bPrecise) const
{
	return TimeString(Elapsed(), bPrecise);
}

double Timer::Lap()
{
	auto Now = std::chrono::system_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::milliseconds>(Now - m_Start);
	m_Start = Now;
	return double(Duration.count());
}

std::string Timer::LapString(bool bPrecise)
{
	return TimeString(Lap(), bPrecise);
}

NAMESPACE_END