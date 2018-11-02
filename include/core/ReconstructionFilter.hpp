#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

/// Reconstruction filters will be tabulated at this resolution
#define HIKARI_FILTER_RESOLUTION 32

NAMESPACE_BEGIN

/**
* \brief Generic radially symmetric image reconstruction filter
*
* When adding radiance-valued samples to the rendered image, it
* was convolved with a so-called image reconstruction filter.
*/
class ReconstructionFilter : public Object
{
public:
	/// Return the filter radius in fractional pixels
	float GetRadius() const;

	/// Evaluate the filter function
	virtual float Eval(float X) const = 0;

	/**
	* \brief Return the type of object (i.e. Mesh/Camera/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const override;

protected:
	float m_Radius;
};

NAMESPACE_END