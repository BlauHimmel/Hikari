#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

/// Ideal mirror BRDF
class MirrorBSDF : public BSDF
{
public:
	MirrorBSDF(const PropertyList & PropList);

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;
	
	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;
	
	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual void Activate() override;
	
	virtual std::string ToString() const override;
};

NAMESPACE_END