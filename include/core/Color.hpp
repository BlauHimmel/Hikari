#pragma once

#include <core\Common.hpp>

NAMESPACE_BEGIN

/**
* \brief Represents a linear RGB color value
*/
struct Color3f : public Eigen::Array3f
{
public:
	using Base = Eigen::Array3f;

	/// Initialize the color vector with a uniform value
	Color3f(float Value = 0.0f);

	/// Initialize the color vector with specific per-channel values
	Color3f(float R, float G, float B);

	/// Construct a color vector from ArrayBase (needed to play nice with Eigen)
	template <typename Derived>
	Color3f(const Eigen::ArrayBase<Derived> & Rhs) : Base(Rhs) { }

	/// Assign a color vector from ArrayBase (needed to play nice with Eigen)
	template <typename Derived>
	Color3f & operator=(const Eigen::ArrayBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

	/// Return a reference to the red channel
	float & R();

	/// Return a reference to the red channel (const version)
	const float & R() const;

	/// Return a reference to the green channel
	float & G();

	/// Return a reference to the green channel (const version)
	const float & G() const;

	/// Return a reference to the blue channel
	float & B();

	/// Return a reference to the blue channel (const version)
	const float & B() const;

	/// Clamp to the positive range
	Color3f Clamp() const;
	
	/// Check if the color vector contains a NaN/Inf/negative value
	bool IsValid() const;

	/// Convert from sRGB to linear RGB
	Color3f ToLinearRGB() const;

	/// Convert from linear RGB to sRGB
	Color3f ToSRGB() const;

	/// Return the associated luminance
	float GetLuminance() const;

	/// Return a human-readable string summary
	std::string ToString() const;
};

/**
* \brief Represents a linear RGB color and a weight
*
* This is used by image reconstruction filter
*/
struct Color4f : public Eigen::Array4f
{
public:
	using Base = Eigen::Array4f;

	/// Create an zero value
	Color4f();

	/// Create from a 3-channel color
	Color4f(const Color3f & Color);

	/// Initialize the color vector with specific per-channel values
	Color4f(float R, float G, float B, float W);

	/// Construct a color vector from ArrayBase (needed to play nice with Eigen)
	template <typename Derived>
	Color4f(const Eigen::ArrayBase<Derived> & Rhs) : Base(Rhs) { }

	/// Assign a color vector from ArrayBase (needed to play nice with Eigen)
	template <typename Derived>
	Color4f &operator=(const Eigen::ArrayBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

	/// Divide by the filter weight and convert into a \ref Color3f value
	Color3f DivideByFilterWeight() const;

	/// Return a human-readable string summary
	std::string ToString() const;
};


NAMESPACE_END