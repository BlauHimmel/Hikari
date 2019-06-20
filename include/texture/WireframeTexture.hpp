#pragma once

#include <core\Common.hpp>
#include <core\Texture.hpp>
#include <mutex>

NAMESPACE_BEGIN

class WireframeTexture : public Texture
{
public:
	WireframeTexture(const PropertyList & PropList);

	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const;

protected:
	Color3f m_InteriorColor, m_EdgeColor;
	float m_TransitionWidth;
	mutable float m_EdgeWidth;
	mutable std::mutex m_Mutex;
};

NAMESPACE_END