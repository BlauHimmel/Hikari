#include <filter\GaussianFilter.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(GaussianFilter, XML_FILTER_GAUSSION);

GaussianFilter::GaussianFilter(const PropertyList & PropList)
{
	/* Half filter size */
	m_Radius = PropList.GetFloat(XML_FILTER_GAUSSION_RADIUS, DEFAULT_FILTER_GAUSSIAN_RADIUS);
	/* Standard deviation of the Gaussian */
	m_Stddev = PropList.GetFloat(XML_FILTER_GAUSSION_STDDEV, DEFAULT_FILTER_GAUSSIAN_STDDEV);
}

float GaussianFilter::Eval(float X) const
{
	float Alpha = -1.0f / (2.0f * m_Stddev * m_Stddev);
	return std::max(
		0.0f,
		std::exp(Alpha * X * X) - std::exp(Alpha * m_Radius * m_Radius)
	);
}

std::string GaussianFilter::ToString() const
{
	return tfm::format(
		"GaussianFilter[radius = %f, stddev = %f]",
		m_Radius, m_Stddev
	);
}

NAMESPACE_END