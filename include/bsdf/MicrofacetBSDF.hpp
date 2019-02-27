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

private:
	float BeckmannD(const Normal3f & M) const;

	float SmithBeckmannG1(const Vector3f & V, const Normal3f & M) const;

protected:
	float m_Alpha;
	float m_IntIOR;
	float m_ExtIOR;
	Color3f m_Kd;

	float m_Ks;
	float m_Eta, m_InvEta;
};

NAMESPACE_END