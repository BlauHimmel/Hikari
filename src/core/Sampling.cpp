#include <core\Sampling.hpp>

NAMESPACE_BEGIN

Point2f Sampling::SquareToUniformSquare(const Point2f & Sample)
{
	return Sample;
}

float Sampling::SquareToUniformSquarePdf(const Point2f & Pt)
{
	return ((Pt.array() >= 0.0f).all() && (Pt.array() <= 1.0f).all()) ? 1.0f : 0.0f;
}

Point2f Sampling::SquareToTent(const Point2f & Sample)
{
	throw HikariException("Sampling::SquareToTent(const Point2f & Sample) is not yet implemented!");
}

float Sampling::SquareToTentPdf(const Point2f & Pt)
{
	throw HikariException("Sampling::SquareToTentPdf(const Point2f & Pt) is not yet implemented!");
}

Point2f Sampling::SquareToUniformDisk(const Point2f & Sample)
{
	throw HikariException("Sampling::SquareToUniformDisk(const Point2f & Sample) is not yet implemented!");
}

float Sampling::SquareToUniformDiskPdf(const Point2f & Pt)
{
	throw HikariException("Sampling::SquareToUniformDiskPdf(const Point2f & Pt) is not yet implemented!");
}

Vector3f Sampling::SquareToUniformSphere(const Point2f & Sample)
{
	throw HikariException("Sampling::SquareToUniformSphere(const Point2f & Sample) is not yet implemented!");
}

float Sampling::SquareToUniformSpherePdf(const Vector3f & V)
{
	throw HikariException("Sampling::SquareToUniformSpherePdf(const Vector3f & V) is not yet implemented!");
}

Vector3f Sampling::SquareToUniformHemisphere(const Point2f & Sample)
{
	throw HikariException("Sampling::SquareToUniformHemisphere(const Point2f & Sample) is not yet implemented!");
}

float Sampling::SquareToUniformHemispherePdf(const Vector3f & V)
{
	throw HikariException("Sampling::SquareToUniformHemispherePdf(const Vector3f & V) is not yet implemented!");
}

Vector3f Sampling::SquareToCosineHemisphere(const Point2f & Sample)
{
	throw HikariException("Sampling::SquareToCosineHemisphere(const Point2f & Sample) is not yet implemented!");
}

float Sampling::SquareToCosineHemispherePdf(const Vector3f & V)
{
	throw HikariException("Sampling::SquareToCosineHemispherePdf(const Vector3f & V) is not yet implemented!");
}

Vector3f Sampling::SquareToBeckmann(const Point2f & Sample, float Alpha)
{
	throw HikariException("Sampling::SquareToBeckmann(const Point2f & Sample, float Alpha) is not yet implemented!");
}

float Sampling::SquareToBeckmannPdf(const Vector3f & M, float Alpha)
{
	throw HikariException("Sampling::SquareToBeckmannPdf(const Vector3f & M, float Alpha) is not yet implemented!");
}

NAMESPACE_END