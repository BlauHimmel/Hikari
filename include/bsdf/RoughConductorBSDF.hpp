#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>
#include <core\MicrofacetDistribution.hpp>

NAMESPACE_BEGIN

/// Rough conductor BSDF
class RoughConductorBSDF : public BSDF
{
public:
	RoughConductorBSDF(const PropertyList & PropList);

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsAnisotropic() const override;

	virtual std::string ToString() const override;

protected:
	float m_IntIOR, m_ExtIOR;
	Color3f m_K, m_Ks;
	float m_AlphaU, m_AlphaV;
	MicrofacetDistribution::EType m_Type;

	Color3f m_Eta, m_EtaK;
};

NAMESPACE_END