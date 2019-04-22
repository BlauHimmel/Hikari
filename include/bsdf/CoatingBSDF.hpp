#pragma once

#include <core\Common.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

class CoatingBSDF : public BSDF
{
public:
	CoatingBSDF(const PropertyList & PropList);

	~CoatingBSDF();

	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const override;

	virtual Color3f Eval(const BSDFQueryRecord & Record) const override;

	virtual float Pdf(const BSDFQueryRecord & Record) const override;

	virtual bool IsDiffuse() const override;

	virtual bool IsAnisotropic() const override;

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const override;

private:
	Vector3f RefractIn(const Vector3f & Wi, float & Reflectance) const;

	Vector3f RefractOut(const Vector3f & Wi, float & Reflectance) const;

protected:
	float m_IntIOR, m_ExtIOR;
	float m_Thickness;
	Texture * m_pSigmaA;
	Texture * m_pKs;
	BSDF * m_pNestedBSDF;

	float m_Eta, m_InvEta;
	float m_SpecularSamplingWeight;
};

NAMESPACE_END