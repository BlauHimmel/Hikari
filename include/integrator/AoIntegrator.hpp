#pragma once

#include <core\Common.hpp>
#include <core\Integrator.hpp>

NAMESPACE_BEGIN

/**
*\brief This integrator simulates a ambient occlusion.
*/
class AoIntegrator : public Integrator
{
public:
	AoIntegrator(const PropertyList & PropList);

	/// Compute the radiance value for a given ray. Just return green here
	virtual Color3f Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const override;

	/// Return a human-readable description for debugging purposes
	virtual std::string ToString() const override;

protected:
	float m_Alpha;
	uint32_t m_SampleCount;
	float m_InvSampleCount;
};

NAMESPACE_END