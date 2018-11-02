#pragma once

#include <core\Common.hpp>

NAMESPACE_BEGIN

/**
* \brief Generic N-dimensional vector data structure based on Eigen::Matrix
*/
template <typename TScalar, int TDimension>
struct TVector : public Eigen::Matrix<TScalar, TDimension, 1>
{
public:
	enum { Dimension = TDimension };
	using Scalar     = TScalar;
	using Base       = Eigen::Matrix<Scalar, Dimension, 1>;
	using VectorType = TVector<Scalar, Dimension>;
	using PointType  = TPoint<Scalar, Dimension>;

	/// Create a new vector with constant component vlaues
	TVector(Scalar Value = Scalar(0)) { Base::setConstant(Value); }

	/// Create a new 2D vector (type error if \c Dimension != 2)
	TVector(Scalar X, Scalar Y) : Base(X, Y) { }

	/// Create a new 3D vector (type error if \c Dimension != 3)
	TVector(Scalar X, Scalar Y, Scalar Z) : Base(X, Y, Z) { }

	/// Create a new 4D vector (type error if \c Dimension != 4)
	TVector(Scalar X, Scalar Y, Scalar Z, Scalar W) : Base(X, Y, Z, W) { }

	/// Construct a vector from MatrixBase (needed to play nice with Eigen)
	template <typename Derived>
	TVector(const Eigen::MatrixBase<Derived> & Rhs) : Base(Rhs) { }

	/// Assign a vector from MatrixBase (needed to play nice with Eigen)
	template <typename Derived>
	TVector & operator=(const Eigen::MatrixBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

	/// Return a human-readable string summary
	std::string ToString() const
	{
		std::string Result;
		for (size_t i = 0; i < Dimension; ++i)
		{
			Result += std::to_string(this->coeff(i));
			if (i + 1 < Dimension)
			{
				Result += ", ";
			}
		}
		return "[" + Result + "]";
	}
};

/**
* \brief Generic N-dimensional point data structure based on Eigen::Matrix
*/
template <typename TScalar, int TDimension>
struct TPoint : public Eigen::Matrix<TScalar, TDimension, 1>
{
public:
	enum { Dimension = TDimension };
	using Scalar     = TScalar;
	using Base       = Eigen::Matrix<Scalar, Dimension, 1>;
	using VectorType = TVector<Scalar, Dimension>;
	using PointType  = TPoint<Scalar, Dimension>;

	/// Create a new point with constant component values
	TPoint(Scalar Value = Scalar(0)) { Base::setConstant(Value); }

	/// Create a new 2D point (type error if \c Dimension != 2)
	TPoint(Scalar X, Scalar Y) : Base(X, Y) { }

	/// Create a new 3D point (type error if \c Dimension != 3)
	TPoint(Scalar X, Scalar Y, Scalar Z) : Base(X, Y, Z) { }

	/// Create a new 4D point (type error if \c Dimension != 4)
	TPoint(Scalar X, Scalar Y, Scalar Z, Scalar W) : Base(X, Y, Z, W) { }

	/// Construct a point from MatrixBase (needed to play nice with Eigen)
	template <typename Derived>
	TPoint(const Eigen::MatrixBase<Derived> & Rhs) : Base(Rhs) { }

	/// Assign a point from MatrixBase (needed to play nice with Eigen)
	template <typename Derived>
	TPoint & operator=(const Eigen::MatrixBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

	/// Return a human-readable string summary
	std::string ToString() const
	{
		std::string Result;
		for (size_t i = 0; i < Dimension; ++i) 
		{
			Result += std::to_string(this->coeff(i));
			if (i + 1 < Dimension)
			{
				Result += ", ";
			}
		}
		return "[" + Result + "]";
	}
};

/**
* \brief 3-dimensional surface normal representation
*/
struct Normal3f : public Eigen::Matrix<float, 3, 1>
{
public:
	enum { Dimension = 3 };
	using Scalar     = float;
	using Base       = Eigen::Matrix<Scalar, Dimension, 1>;
	using VectorType = TVector<Scalar, Dimension>;
	using PointType  = TPoint<Scalar, Dimension>;

	/// Create a new normal with constant component vlaues
	Normal3f(Scalar Value = 0.0f) { Base::setConstant(Value); }

	/// Create a new 3D normal
	Normal3f(Scalar X, Scalar Y, Scalar Z) : Base(X, Y, Z) { }

	/// Construct a normal from MatrixBase (needed to play nice with Eigen)
	template <typename Derived>
	Normal3f(const Eigen::MatrixBase<Derived> & Rhs) : Base(Rhs) { }

	/// Assign a normal from MatrixBase (needed to play nice with Eigen)
	template <typename Derived>
	Normal3f & operator=(const Eigen::MatrixBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

	/// Return a human-readable string summary
	std::string ToString() const
	{
		return tfm::format("[%f, %f, %f]", coeff(0), coeff(1), coeff(2));
	}
};

NAMESPACE_END