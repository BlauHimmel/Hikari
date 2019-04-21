#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>
#include <core\MicrofacetDistribution.hpp>

NAMESPACE_BEGIN

/// Rough plastoc BSDF
class RoughPlasticBSDF : public BSDF
{
public:
	RoughPlasticBSDF(const PropertyList & PropList);

	~RoughPlasticBSDF();

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsDiffuse() const override;

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const override;

protected:
	float m_IntIOR, m_ExtIOR;
	Texture * m_pKs;
	Texture * m_pKd;
	Texture * m_pAlpha;
	bool m_bNonlinear;
	MicrofacetDistribution::EType m_Type;

	RoughTransmittance * m_pIntData;
	RoughTransmittance * m_pExtData;

	float m_Eta, m_InvEta, m_InvEta2;
	float m_SpecularSamplingWeight;
};

NAMESPACE_END