#pragma once

#include <core\Common.hpp>
#include <core\Texture.hpp>
#include <core\MipMap.hpp>

NAMESPACE_BEGIN

class BitmapTexture : public Texture2D
{
public:
	BitmapTexture(const PropertyList & PropList);

	virtual Color3f Eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const override;

	virtual Color3f Eval(const Point2f & UV) const override;

	virtual void EvalGradient(const Point2f & UV, Color3f * pGradients) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const;

protected:
	std::string m_Filename;
	float m_Gamma;
	EWrapMode m_UWrapMode;
	EWrapMode m_VWrapMode;
	EFilterType m_FilterType;
	float m_MaxAnisotropy;
	uint32_t m_Channel;
	std::unique_ptr<MipMap1f> m_Texture1f;
	std::unique_ptr<MipMap3f> m_Texture3f;

	Color3f m_Average;
	Color3f m_Maximum;
	Color3f m_Minimum;
	uint32_t m_Width;
	uint32_t m_Height;
};

NAMESPACE_END