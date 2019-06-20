#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\MemoryArena.hpp>

NAMESPACE_BEGIN

std::unique_ptr<float[]> LoadImageFromFileR(
	const std::string & Filename,
	float Gamma,
	int & Width,
	int & Height,
	float * pAverage = nullptr,
	float * pMaximum = nullptr,
	float * pMinimum = nullptr
);

std::unique_ptr<Color3f[]> LoadImageFromFileRGB(
	const std::string & Filename,
	float Gamma,
	int & Width,
	int & Height,
	Color3f * pAverage = nullptr,
	Color3f * pMaximum = nullptr,
	Color3f * pMinimum = nullptr
);

/**
* Basic class of all textures
*/
class Texture : public Object
{
public:
	/**
	* \brief Return the texture value at \c Isect
	*/
	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const;

	/**
	* \brief Return the texture gradient at Isect. This function is usually 
	* implemented pointwise without any kind of filtering. Length of the array 
	* 'pGradients' should be at least 2 (U and V component respectively).
	*/
	virtual void EvalGradient(const Intersection & Isect, Color3f * pGradients) const;

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

	/// Filtered texture lookup -- Texture2D subclasses must provide this function
	virtual Color3f Eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const = 0;

	/// Unfiltered texture lookup -- Texture2D subclasses must provide this function
	virtual Color3f Eval(const Point2f & UV) const = 0;

	/**
	* \brief Return the texture gradient at Isect. This function is usually
	* implemented pointwise without any kind of filtering. Length of the array
	* 'pGradients' should be at least 2 (U and V component respectively).
	*/
	virtual void EvalGradient(const Intersection & Isect, Color3f * pGradients) const override;

	/// Unfiltered texture lookup
	virtual void EvalGradient(const Point2f & UV, Color3f * pGradients) const;

};

/* ============================================================ */
/*                  Some very basic textures                    */
/* ============================================================ */

class ConstantColor3fTexture : public Texture
{
public:
	ConstantColor3fTexture(const Color3f & Value);

	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const override;

protected:
	Color3f m_Value;
};

class ConstantFloatTexture : public Texture
{
public:
	ConstantFloatTexture(float Value);
	
	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const override;

protected:
	float m_Value;
};

class Color3fAdditionTexture : public Texture
{
public:
	Color3fAdditionTexture(const Texture * pTextureA, const Texture * pTextureB);

	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const override;

protected:
	const Texture * m_pTextureA;
	const Texture * m_pTextureB;
};

class Color3fSubtractionTexture : public Texture
{
public:
	Color3fSubtractionTexture(const Texture * pTextureA, const Texture * pTextureB);

	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const override;

protected:
	const Texture * m_pTextureA;
	const Texture * m_pTextureB;
};

class Color3fProductTexture : public Texture
{
public:
	Color3fProductTexture(const Texture * pTextureA, const Texture * pTextureB);

	virtual Color3f Eval(const Intersection & Isect, bool bFilter = true) const override;

	virtual Color3f GetAverage() const override;

	virtual Color3f GetMinimum() const override;

	virtual Color3f GetMaximum() const override;

	virtual Vector3i GetDimension() const override;

	virtual bool IsConstant() const override;

	virtual bool IsMonochromatic() const override;

	virtual std::string ToString() const override;

protected:
	const Texture * m_pTextureA;
	const Texture * m_pTextureB;
};

NAMESPACE_END