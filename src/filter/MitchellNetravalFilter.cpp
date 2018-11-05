#include <filter\MitchellNetravalFilter.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(MitchellNetravaliFilter, XML_FILTER_MITCHELL_NETRAVALI);

MitchellNetravaliFilter::MitchellNetravaliFilter(const PropertyList & PropList)
{
	/* Filter size in pixels */
	m_Radius = PropList.GetFloat(XML_FILTER_MITCHELL_NETRAVALI_RADIUS, DEFAULT_FILTER_MITCHELL_RADIUS);
	/* B parameter from the paper */
	m_B = PropList.GetFloat(XML_FILTER_MITCHELL_NETRAVALI_B, DEFAULT_FILTER_MITCHELL_B);
	/* C parameter from the paper */
	m_C = PropList.GetFloat(XML_FILTER_MITCHELL_NETRAVALI_C, DEFAULT_FILTER_MITCHELL_C);
}

float MitchellNetravaliFilter::Eval(float X) const
{
	X = std::abs(2.0f * X / m_Radius);
	float X2 = X * X, X3 = X2 * X;

	if (X < 1.0f)
	{
		return 1.0f / 6.0f * ((12.0f - 9.0f * m_B - 6.0f * m_C) * X3
			+ (-18.0f + 12.0f * m_B + 6.0f * m_C) * X2 + (6.0f - 2.0f * m_B));
	}
	else if (X < 2.0f)
	{
		return 1.0f / 6.0f * ((-m_B - 6 * m_C) * X3 + (6.0f * m_B + 30.0f * m_C) * X2
			+ (-12.0f * m_B - 48.0f * m_C)*X + (8.0f * m_B + 24.0f * m_C));
	}
	else
	{
		return 0.0f;
	}
}

std::string MitchellNetravaliFilter::ToString() const
{
	return tfm::format(
		"MitchellNetravaliFilter[radius = %f, B = %f, C = %f]",
		m_Radius, m_B, m_C
	);
}

NAMESPACE_END

