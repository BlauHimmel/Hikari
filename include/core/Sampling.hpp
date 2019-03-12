#pragma once

#include <core\Common.hpp>
#include <core\Vector.hpp>

NAMESPACE_BEGIN

class Sampling
{
public:
	/// Takes uniformly distributed points in a square and just returns them
	static Point2f SquareToUniformSquare(const Point2f & Sample);

	/// Probability density of \ref SquareToUniformSquare()
	static float SquareToUniformSquarePdf(const Point2f & Sample);

	/// Sample a 2D tent distribution
	static Point2f SquareToTent(const Point2f & Sample);

	/// Probability density of \ref SquareToTent()
	static float SquareToTentPdf(const Point2f & Pt);

	/// Uniformly sample a vector on a 2D disk with radius 1, centered around the origin
	static Point2f SquareToUniformDisk(const Point2f & Sample);

	/// Probability density of \ref SquareToUniformDisk()
	static float SquareToUniformDiskPdf(const Point2f & Pt);

	/// Uniformly sample a vector on the unit sphere with respect to solid angles
	static Vector3f SquareToUniformSphere(const Point2f & Sample);

	/// Probability density of \ref SquareToUniformSphere()
	static float SquareToUniformSpherePdf(const Vector3f & V);

	/// Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to solid angles
	static Vector3f SquareToUniformHemisphere(const Point2f & Sample);

	/// Probability density of \ref SquareToUniformHemisphere()
	static float SquareToUniformHemispherePdf(const Vector3f & V);

	/// Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to projected solid angles
	static Vector3f SquareToCosineHemisphere(const Point2f & Sample);

	/// Probability density of \ref SquareToCosineHemisphere()
	static float SquareToCosineHemispherePdf(const Vector3f & V);

	/// Warp a uniformly distributed square sample to a Beckmann distribution * cosine for the given 'alpha' parameter
	/// (Deprecated : Sampling methods relevant to microfacet are wrapped in MicrofacetDistribution class now.)
	static Vector3f SquareToBeckmann(const Point2f & Sample, float Alpha);

	/// Probability density of \ref SquareToBeckmann()
	/// (Deprecated : Sampling methods relevant to microfacet are wrapped in MicrofacetDistribution class now.)
	static float SquareToBeckmannPdf(const Vector3f & M, float Alpha);
};

NAMESPACE_END