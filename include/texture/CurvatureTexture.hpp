#pragma once

#include <core\Common.hpp>
#include <core\Texture.hpp>
#include <mutex>

NAMESPACE_BEGIN

class CurvatureTexture : public Texture
{
public:
	CurvatureTexture(const PropertyList & PropList);

	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const;

protected:
	Color3f m_PositiveColor, m_NegativeColor;
	float m_Scale;
	std::string m_CurvatureType;

	bool m_bGaussianCurvature;
};

NAMESPACE_END