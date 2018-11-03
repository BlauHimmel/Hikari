#include <filter\TentFilter.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(TentFilter, XML_FILTER_TENT);

TentFilter::TentFilter(const PropertyList & PropList)
{
	m_Radius = 1.0f;
}

float TentFilter::Eval(float X) const
{
	return std::max(0.0f, 1.0f - std::abs(X));
}

std::string TentFilter::ToString() const
{
	return "TentFilter[]";
}

NAMESPACE_END