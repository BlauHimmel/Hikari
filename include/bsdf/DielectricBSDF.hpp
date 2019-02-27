#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

/// Ideal dielectric BSDF
class DielectricBSDF : public BSDF
{
public:
	DielectricBSDF(const PropertyList & PropList);

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual std::string ToString() const override;

protected:
	float m_IntIOR, m_ExtIOR;
	Color3f m_KsReflect, m_KsRefract;

	float m_Eta, m_InvEta;
};

NAMESPACE_END