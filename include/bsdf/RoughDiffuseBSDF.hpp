#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

/// Rough diffuse BSDF, aka. Oren¨CNayar BSDF
class RoughDiffuseBSDF : public BSDF
{
public:
	RoughDiffuseBSDF(const PropertyList & PropList);

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsDiffuse() const override;

	virtual std::string ToString() const override;

public:
	bool m_bFastApprox;
	Color3f m_Albedo;
	float m_Alpha;
};

NAMESPACE_END