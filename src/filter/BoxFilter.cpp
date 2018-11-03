#include <filter\BoxFilter.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(BoxFilter, XML_FILTER_BOX);

BoxFilter::BoxFilter(const PropertyList & PropList)
{
	m_Radius = 0.5f;
}

float BoxFilter::Eval(float X) const
{
	return 1.0f;
}

std::string BoxFilter::ToString() const
{
	return "BoxFilter[]";
}

NAMESPACE_END