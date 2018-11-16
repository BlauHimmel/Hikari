#pragma once

#include <core\Common.hpp>
#include <core\Integrator.hpp>

NAMESPACE_BEGIN

class NormalIntegrator : public Integrator
{
public:
	NormalIntegrator(const PropertyList & PropList);

	/// Compute the radiance value for a given ray. Just return green here
	virtual Color3f Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const override;

	/// Return a human-readable description for debugging purposes
	virtual std::string ToString() const override;
};

NAMESPACE_END