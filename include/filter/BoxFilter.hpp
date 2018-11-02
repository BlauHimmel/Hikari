#pragma once

#include <core\Common.hpp>
#include <core\ReconstructionFilter.hpp>

NAMESPACE_BEGIN

/// Box filter -- fastest, but prone to aliasing
class BoxFilter : public ReconstructionFilter
{
public:
	BoxFilter(const PropertyList & PropList);

	virtual float Eval(float X) const override;

	virtual std::string ToString() const override;
};

NAMESPACE_END