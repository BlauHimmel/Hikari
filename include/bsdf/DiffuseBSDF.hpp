#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

/**
* \brief Diffuse / Lambertian BRDF model
*/
class DiffuseBSDF : public BSDF
{
public:
	DiffuseBSDF(const PropertyList & PropList);

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsDiffuse() const override;

	virtual std::string ToString() const override;

protected:
	Color3f m_Albedo;
};

NAMESPACE_END