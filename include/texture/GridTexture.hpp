#pragma once

#include <core\Common.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

class GridTexture : public Texture2D
{
public:
	GridTexture(const PropertyList & PropList);

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
	Color3f m_BackgroundColor, m_LineColor;
	float m_LineWidth;
	int m_Lines;
};

NAMESPACE_END