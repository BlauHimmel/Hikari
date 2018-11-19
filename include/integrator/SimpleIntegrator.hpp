#pragma once

#include <core\Common.hpp>
#include <core\Integrator.hpp>

NAMESPACE_BEGIN

/**
*\brief This integrator simulates a single point light source located at a 3D 
* position and emits an amount of energy given by the parameter energy.
*/
class SimpleIntegrator : public Integrator
{
public:
	SimpleIntegrator(const PropertyList & PropList);

	/// Compute the radiance value for a given ray. Just return green here
	virtual Color3f Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const override;

	/// Return a human-readable description for debugging purposes
	virtual std::string ToString() const override;

protected:
	Point3f m_Position;
	Color3f m_Power;
};

NAMESPACE_END