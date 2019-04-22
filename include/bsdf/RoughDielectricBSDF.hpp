#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>
#include <core\MicrofacetDistribution.hpp>

NAMESPACE_BEGIN

/// Rough dielectric BSDF
class RoughDielectricBSDF : public BSDF
{
public:
	RoughDielectricBSDF(const PropertyList & PropList);

	~RoughDielectricBSDF();

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsAnisotropic() const override;

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const override;

protected:
	float m_IntIOR, m_ExtIOR;
	Texture * m_pKsReflect;
	Texture * m_pKsRefract;
	Texture * m_pAlphaU;
	Texture * m_pAlphaV;
	MicrofacetDistribution::EType m_Type;

	bool m_bAnisotropic;

	float m_Eta, m_InvEta;
};

NAMESPACE_END