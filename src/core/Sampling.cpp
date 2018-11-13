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
	return Point2f(
		(Sample.x() < 0.5f ? (std::sqrtf(2.0f * Sample.x()) - 1.0f) : (1.0f - std::sqrtf(2.0f - 2.0f * Sample.x()))),
		(Sample.y() < 0.5f ? (std::sqrtf(2.0f * Sample.y()) - 1.0f) : (1.0f - std::sqrtf(2.0f - 2.0f * Sample.y())))
	);
}

float Sampling::SquareToTentPdf(const Point2f & Pt)
{
	return (1.0f - std::abs(Pt.x())) * (1.0f - std::abs(Pt.y()));
}

Point2f Sampling::SquareToUniformDisk(const Point2f & Sample)
{
	Point2f OffsetSample = 2.0f * Sample - Point2f(1.0f);
	if (OffsetSample.x() == 0.0f && OffsetSample.y() == 0.0f)
	{
		return Point2f(0.0f);
	}

	float Theta, R;

	if (std::abs(OffsetSample.x()) > std::abs(OffsetSample.y()))
	{
		R = OffsetSample.x();
		Theta = M_PI * 0.25f * (OffsetSample.y() / OffsetSample.x());
	}
	else
	{
		R = OffsetSample.y();
		Theta = M_PI * 0.5f - M_PI * 0.25f * (OffsetSample.x() / OffsetSample.y());
	}

	return Point2f(
		R * std::cosf(Theta),
		R * std::sinf(Theta)
	);
}

float Sampling::SquareToUniformDiskPdf(const Point2f & Pt)
{
	return (Pt.norm() <= 1.0f) ? 1.0f / M_PI : 0.0f;
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