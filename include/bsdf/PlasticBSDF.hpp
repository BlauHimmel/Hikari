#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

/// Smooth plastic BSDF
class PlasticBSDF : public BSDF
{
public:
	PlasticBSDF(const PropertyList & PropList);

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsDiffuse() const override;

	virtual std::string ToString() const override;

protected:
	float m_IntIOR, m_ExtIOR;
	Color3f m_Ks, m_Kd;
	bool m_bNonlinear;

	float m_Eta, m_InvEta, m_InvEta2;
	float m_SpecularSamplingWeight;
	float m_FresnelDiffuseReflectanceInt, m_FresnelDiffuseReflectanceExt;
};

NAMESPACE_END