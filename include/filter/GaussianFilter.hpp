#pragma once

#include <core\Common.hpp>
#include <core\ReconstructionFilter.hpp>

NAMESPACE_BEGIN

/**
* Windowed Gaussian filter with configurable extent
* and standard deviation. Often produces pleasing
* results, but may introduce too much blurring.
*/
class GaussianFilter : public ReconstructionFilter
{
public:
	GaussianFilter(const PropertyList & PropList);

	virtual float Eval(float X) const override;

	virtual std::string ToString() const override;

protected:
	float m_Stddev;
};

NAMESPACE_END