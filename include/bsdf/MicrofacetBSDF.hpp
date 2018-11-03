#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

class MicrofacetBSDF : public BSDF
{
public:
	MicrofacetBSDF(const PropertyList & PropList);

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsDiffuse() const override;

	virtual std::string ToString() const override;

protected:
	float m_Alpha;
	float m_IntIOR;
	float m_ExtIOR;
	float m_Ks;
	Color3f m_Kd;
};

NAMESPACE_END