#pragma once

#include <core/Common.hpp>
#include <core/Vector.hpp>

NAMESPACE_BEGIN

template <typename TPointType, typename TVectorType>
struct TRay
{
	using PointType = TPointType;
	using VectorType = TVectorType;
	using Scaler = PointType::Scaler;

	PointType Origin;
	VectorType Direction;
	VectorType DirectionReciprocal;
	Scaler MinT;
	Scaler MaxT;

	TRay() : MinT(Epsilon), MaxT(std::numeric_limits<Scalar>::infinity()) { }

	TRay(const PointType & Origin, const VectorType & Direction) :
		Origin(Origin), Direction(Direction),
		MinT(Epsilon), MaxT(std::numeric_limits<Scalar>::infinity())
	{
		Update();
	}

	TRay(const PointType & Origin, const VectorType & Direction, Scalar MinT, Scalar MaxT) :
		Origin(Origin), Direction(Direction), MinT(MinT), MaxT(MaxT)
	{
		Update();
	}

	TRay(const TRay & Ray) : 
		Origin(Ray.Origin), Direction(Ray.Direction), 
		DirectionReciprocal(Ray.DirectionReciprocal), 
		MinT(Ray.MinT), MaxT(Ray.MaxT) { }

	TRay(const TRay & Ray, Scalar MinT, Scalar MaxT) :
		Origin(ray.Origin), Direction(ray.Direction),
		DirectionReciprocal(ray.DirectionReciprocal),
		MinT(Ray.MinT), MaxT(Ray.MaxT) { }

	void update() 
	{
		DirectionReciprocal = Direction.cwiseInverse();
	}

	PointType operator() (Scalar T) const { return Origin + T * Direction; }

	TRay Reverse() const
	{
		TRay Result;
		Result.Origin = Origin;
		Result.Direction = -Direction;
		Result.DirectionReciprocal = -DirectionReciprocal;
		Result.MinT = MinT;
		Result.MaxT = MaxT;
		return Result;
	}

	std::string ToString() const
	{
		return tfm::format(
			"Ray[\n"
			"  Origin = %s,\n"
			"  Direction = %s,\n"
			"  MinT = %f,\n"
			"  MaxT = %f\n"
			"]", 
			Origin.ToString(), Direction.ToString(), MinT, MaxT
		);
	}
};

NAMESPACE_END