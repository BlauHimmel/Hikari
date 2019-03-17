#pragma once

#include <core\Common.hpp>
#include <core\Integrator.hpp>

NAMESPACE_BEGIN

class WhittedIntegrator : public Integrator
{
public:
	WhittedIntegrator(const PropertyList & PropList);

	/// Compute the radiance value for a given ray. Just return green here
	virtual Color3f Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const override;

	/// Return a human-readable description for debugging purposes
	virtual std::string ToString() const override;

protected:
	Color3f LiRecursive(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray, uint32_t Depth) const;

	uint32_t m_Depth;
};

NAMESPACE_END