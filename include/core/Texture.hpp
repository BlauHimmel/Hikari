#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

/**
* Basic class of all textures
*/
class Texture : public Object
{
public:
	/**
	* \brief Return the texture value at \c Isect
	* \param bFilter
	*    Specifies whether a filtered texture lookup is desired. Note
	*    that this does not mean that filtering will actually be used.
	*/
	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const;

	/// Return the component-wise average value of the texture over its domain
	virtual Color3f GetAverage() const;

	/// Return the component-wise minimum of the texture over its domain
	virtual Color3f GetMinimum() const;

	/// Return the component-wise maximum of the texture over its domain
	virtual Color3f GetMaximum() const;

	/// Return the dimension in pixels, if applicable
	virtual Vector3i GetDimension() const;

	/// Return whether the texture takes on a constant value everywhere
	virtual bool IsConstant() const;

	/// Return whether the texture is monochromatic / spectrally uniform
	virtual bool IsMonochromatic() const;

	/// Some textures are only proxies for an actual implementation.This function returns the actual texture implementation to be used.
	virtual Texture * ActualTexture();

	/**
	* \brief Return the type of object (i.e. Mesh/BSDF/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const override;

protected:
	Point2f m_UVOffset;
	Vector2f m_UVScale;
};

/**
*  Base class of all 2D textures
*/
class Texture2D : public Texture
{
public:
	/**
	* \brief Return the texture value at \c Isect
	* \param bFilter
	*    Specifies whether a filtered texture lookup is desired. Note
	*    that this does not mean that filtering will actually be used.
	*/
	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	/// Unfiltered texture lookup -- Texture2D subclasses must provide this function
	virtual Color3f Eval(const Point2f & UV) const = 0;

	/// Filtered texture lookup -- Texture2D subclasses must provide this function
	virtual Color3f Eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const = 0;

};

NAMESPACE_END