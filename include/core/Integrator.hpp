#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

/**
* \brief Abstract integrator (i.e. a rendering technique)
*
* In Nori, the different rendering techniques are collectively referred to as
* integrators, since they perform integration over a high-dimensional
* space. Each integrator represents a specific approach for solving
* the light transport equation---usually favored in certain scenarios, but
* at the same time affected by its own set of intrinsic limitations.
*/
class Integrator : public Object
{
public:
	/// Perform an (optional) preprocess step
	virtual void Preprocess(const Scene * pScene);

	/**
	* \brief Sample the incident radiance along a ray
	*
	* \param pScene
	*    A pointer to the underlying scene
	* \param pSampler
	*    A pointer to a sample generator
	* \param Ray
	*    The ray in question
	* \return
	*    A (usually) unbiased estimate of the radiance in this direction
	*/
	virtual Color3f Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const = 0;

	/**
	* \brief Return the type of object (i.e. Mesh/BSDF/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const override;
};

NAMESPACE_END