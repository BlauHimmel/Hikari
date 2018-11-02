#pragma once

#include <core\Common.hpp>
#include <core\ReconstructionFilter.hpp>

NAMESPACE_BEGIN

/// Tent filter 
class TentFilter : public ReconstructionFilter
{
public:
	TentFilter(const PropertyList & PropList);

	virtual float Eval(float X) const override;

	virtual std::string ToString() const override;
};

NAMESPACE_END