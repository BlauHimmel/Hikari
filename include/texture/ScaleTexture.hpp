#pragma once

#include <core\Common.hpp>
#include <core\Texture.hpp>
#include <mutex>

NAMESPACE_BEGIN

class ScaleTexture : public Texture
{
public:
	ScaleTexture(const PropertyList & PropList);

	~ScaleTexture();

	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual void EvalGradient(const Intersection & Isect, Color3f * pGradients) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual Texture * ActualTexture() override;

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const;

protected:
	Texture * m_pNestedTexture;
	Color3f m_Scale, m_Coeff;
};

NAMESPACE_END