#pragma once

#include <core\Common.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

class CheckerboardTexture : public Texture2D
{
public:
	CheckerboardTexture(const PropertyList & PropList);

	virtual Color3f Eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const override;

	virtual Color3f Eval(const Point2f & UV) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const;

protected:
	Color3f m_ColorA, m_ColorB;
	int m_Blocks;
};

NAMESPACE_END