#pragma once

#include <core\Common.hpp>
#include <core\ReconstructionFilter.hpp>

NAMESPACE_BEGIN

/**
* Separable reconstruction filter by Mitchell and Netravali
*
* D. Mitchell, A. Netravali, Reconstruction filters for computer graphics,
* Proceedings of SIGGRAPH 88, Computer Graphics 22(4), pp. 221-228, 1988.
*/
class MitchellNetravaliFilter : public ReconstructionFilter
{
public:
	MitchellNetravaliFilter(const PropertyList & PropList); 

	virtual float Eval(float X) const override;

	virtual std::string ToString() const override;
	
protected:
	float m_B, m_C;
};

NAMESPACE_END