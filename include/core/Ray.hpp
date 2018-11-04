#pragma once

#include <core\Common.hpp>
#include <core\Vector.hpp>

NAMESPACE_BEGIN

/**
* \brief Simple n-dimensional ray segment data structure
*
* Along with the ray origin and direction, this data structure additionally
* stores a ray segment [MinT, MaxT] (whose entries may include positive/negative
* infinity), as well as the componentwise reciprocals of the ray direction.
* That is just done for convenience, as these values are frequently required.
*
* \remark Important: be careful when changing the ray direction. You must
* call \ref Update() to compute the componentwise reciprocals as well, or
* ray-triangle intersection code will go haywire.
*/
template <typename TPointType, typename TVectorType>
struct TRay
{
	using PointType = TPointType;
	using VectorType = TVectorType;
	using Scalar = typename PointType::Scalar;

	PointType Origin;               ///< Ray origin
	VectorType Direction;           ///< Ray direction
	VectorType DirectionReciprocal; ///< Componentwise reciprocals of the ray direction
	Scalar MinT;                    ///< Minimum position on the ray segment
	Scalar MaxT;                    ///< Maximum position on the ray segment

	/// Construct a new ray
	TRay() : MinT(float(Epsilon)), MaxT(std::numeric_limits<Scalar>::infinity()) { }
	
	/// Construct a new ray
	TRay(const PointType & Origin, const VectorType & Direction) :
		Origin(Origin), Direction(Direction),
		MinT(float(Epsilon)), MaxT(std::numeric_limits<Scalar>::infinity())
	{
		Update();
	}

	/// Construct a new ray
	TRay(const PointType & Origin, const VectorType & Direction, Scalar MinT, Scalar MaxT) :
		Origin(Origin), Direction(Direction), MinT(MinT), MaxT(MaxT)
	{
		Update();
	}

	/// Copy constructor
	TRay(const TRay & Ray) :
		Origin(Ray.Origin), Direction(Ray.Direction), 
		DirectionReciprocal(Ray.DirectionReciprocal), 
		MinT(Ray.MinT), MaxT(Ray.MaxT) { }

	/// Copy a ray, but change the covered segment of the copy
	TRay(const TRay & Ray, Scalar MinT, Scalar MaxT) :
		Origin(ray.Origin), Direction(ray.Direction),
		DirectionReciprocal(ray.DirectionReciprocal),
		MinT(Ray.MinT), MaxT(Ray.MaxT) { }

	/// Update the reciprocal ray directions after changing 'Direction'
	void Update()
	{
		DirectionReciprocal = Direction.cwiseInverse();
	}

	/// Return the position of a point along the ray
	PointType operator() (Scalar T) const { return Origin + T * Direction; }

	/// Return a ray that points into the opposite direction
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

	/// Return a human-readable string summary of this ray
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