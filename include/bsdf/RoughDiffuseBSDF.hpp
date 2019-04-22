#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

/// Rough diffuse BSDF, aka. Oren¨CNayar BSDF
class RoughDiffuseBSDF : public BSDF
{
public:
	RoughDiffuseBSDF(const PropertyList & PropList);

	~RoughDiffuseBSDF();

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const override;

protected:
	bool m_bFastApprox;
	Texture * m_pAlbedo;
	Texture * m_pAlpha;
};

NAMESPACE_END