#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

class BumpMapBSDF : public BSDF
{
public:
	BumpMapBSDF(const PropertyList & PropList);

	~BumpMapBSDF();

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsDiffuse() const override;

	virtual bool IsAnisotropic() const override;

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const override;

private:
	Frame GetPerturbebFrame(const Intersection & Isect) const;

protected:
	Texture * m_pBumpMap;
	BSDF * m_pNestedBSDF;
};

NAMESPACE_END