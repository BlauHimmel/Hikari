#pragma once

#include <core/Common.hpp>

NAMESPACE_BEGIN

template <typename TScalar, int TDimension>
struct TVector : public Eigen::Matrix<TScalar, TDimension, 1>
{
public:
	enum { Dimension = TDimension };
	using Scaler     = TScalar;
	using Base       = Eigen::Matrix<Scaler, Dimension, 1>;
	using VectorType = TVector<Scaler, Dimension>;
	using PointType  = TPoint<Scaler, Dimension>;

	TVector(Scalar Value = Scalar(0)) { Base::setConstant(Value); }

	TVector(Scalar X, Scalar Y) : Base(X, Y) { }

	TVector(Scalar X, Scalar Y, Scalar Z) : Base(X, Y, Z) { }

	TVector(Scalar X, Scalar Y, Scalar Z, Scalar w) : Base(X, Y, Z, W) { }

	template <typename Derived>
	TVector(const Eigen::MatrixBase<Derived> & Rhs) : Base(Rhs) { }

	template <typename Derived>
	TVector & operator=(const Eigen::MatrixBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

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

template <typename TScalar, int TDimension>
struct TPoint : public Eigen::Matrix<TScalar, TDimension, 1>
{
public:
	enum { Dimension = TDimension };
	using Scaler     = TScalar;
	using Base       = Eigen::Matrix<Scaler, Dimension, 1>;
	using VectorType = TVector<Scaler, Dimension>;
	using PointType  = TPoint<Scaler, Dimension>;

	TPoint(Scalar Value = Scalar(0)) { Base::setConstant(Value); }

	TPoint(Scalar X, Scalar Y) : Base(X, Y) { }

	TPoint(Scalar X, Scalar Y, Scalar Z) : Base(X, Y, Z) { }

	TPoint(Scalar X, Scalar Y, Scalar Z, Scalar w) : Base(X, Y, Z, W) { }

	template <typename Derived>
	TPoint(const Eigen::MatrixBase<Derived> & Rhs) : Base(Rhs) { }

	template <typename Derived>
	TPoint & operator=(const Eigen::MatrixBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

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

struct Normal3f : public Eigen::Matrix<float, 3, 1>
{
public:
	enum { Dimension = 3 };
	using Scaler     = float;
	using Base       = Eigen::Matrix<Scaler, Dimension, 1>;
	using VectorType = TVector<Scaler, Dimension>;
	using PointType  = TPoint<Scaler, Dimension>;

	Normal3f(Scalar Value = 0.0f) { Base::setConstant(Value); }

	Normal3f(Scalar X, Scalar Y, Scalar Z) : Base(X, Y, Z) { }

	template <typename Derived>
	Normal3f(const Eigen::MatrixBase<Derived> & Rhs) : Base(Rhs) { }

	template <typename Derived>
	Normal3f & operator=(const Eigen::MatrixBase<Derived> & Rhs)
	{
		this->Base::operator=(Rhs);
		return *this;
	}

	std::string ToString() const
	{
		return tfm::format("[%f, %f, %f]", coeff(0), coeff(1), coeff(2));
	}
};

void CoordinateSystem(const Vector3f & Va, Vector3f & Vb, Vector3f & Vc);

NAMESPACE_END