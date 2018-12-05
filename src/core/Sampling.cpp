#include <core\Sampling.hpp>
#include <core\Frame.hpp>

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
	// PBRT : P776-779
	Point2f OffsetSample = 2.0f * Sample - Point2f(1.0f);
	if (OffsetSample.x() == 0.0f && OffsetSample.y() == 0.0f)
	{
		return Point2f(0.0f);
	}

	float Theta, R;

	if (std::abs(OffsetSample.x()) > std::abs(OffsetSample.y()))
	{
		R = OffsetSample.x();
		Theta = float(M_PI) * 0.25f * (OffsetSample.y() / OffsetSample.x());
	}
	else
	{
		R = OffsetSample.y();
		Theta = float(M_PI) * 0.5f - float(M_PI) * 0.25f * (OffsetSample.x() / OffsetSample.y());
	}

	return Point2f(
		R * std::cosf(Theta),
		R * std::sinf(Theta)
	);
}

float Sampling::SquareToUniformDiskPdf(const Point2f & Pt)
{
	// PBRT : P776-779
	return (Pt.norm() <= 1.0f) ? 1.0f / float(M_PI) : 0.0f;
}

Vector3f Sampling::SquareToUniformSphere(const Point2f & Sample)
{
	float Z = 1.0f - 2.0f * Sample.x();
	float SinTheta = std::sqrt(std::max(0.0f, 1.0f - Z * Z));
	float Phi = 2.0f * float(M_PI) * Sample.y();
	return Vector3f(
		SinTheta * std::cosf(Phi),
		SinTheta * std::sinf(Phi),
		Z
	);
}

float Sampling::SquareToUniformSpherePdf(const Vector3f & V)
{
	return (V.norm() - 1.0f <= 1e-6) ? 1.0f / (4.0f * float(M_PI)) : 0.0f;
}

Vector3f Sampling::SquareToUniformHemisphere(const Point2f & Sample)
{
	float Z = 1.0f - Sample.x();
	float SinTheta = std::sqrt(std::max(0.0f, 1.0f - Z * Z));
	float Phi = 2.0f * float(M_PI) * Sample.y();
	return Vector3f(
		SinTheta * std::cosf(Phi),
		SinTheta * std::sinf(Phi),
		Z
	);
}

float Sampling::SquareToUniformHemispherePdf(const Vector3f & V)
{
	return (V.norm() - 1.0f <= 1e-6f && V.z() >= 0.0f) ? 1.0f / (2.0f * float(M_PI)) : 0.0f;
}

Vector3f Sampling::SquareToCosineHemisphere(const Point2f & Sample)
{
	Point2f Disk = SquareToUniformDisk(Sample);
	float Z = std::sqrt(1.0f - Disk.x() * Disk.x() - Disk.y() * Disk.y());

	if (std::isnan(Z))
	{
		Z = 0.0f;
	}

	return Vector3f(Disk.x(), Disk.y(), Z);
}

float Sampling::SquareToCosineHemispherePdf(const Vector3f & V)
{
	return (V.norm() - 1.0f <= 1e-6f && V.z() >= 0.0f) ? V.z() / float(M_PI) : 0.0f;
}

Vector3f Sampling::SquareToBeckmann(const Point2f & Sample, float Alpha)
{
	float Ln = std::logf(1.0f - Sample.x());
	if (std::isnan(Ln))
	{
		Ln = 1.0f;
	}

	float Tan2Theta = -1.0f * Alpha * Alpha * Ln;
	float CosTheta = 1.0f / std::sqrtf(1.0f + Tan2Theta);
	float SinTheta = std::sqrtf(1.0f - CosTheta * CosTheta);
	float Phi = 2.0f * float(M_PI) * Sample.y();

	return Vector3f(
		SinTheta * std::cosf(Phi),
		SinTheta * std::sinf(Phi),
		CosTheta
	);
}

float Sampling::SquareToBeckmannPdf(const Vector3f & M, float Alpha)
{
	if (M.norm() - 1.0f <= 1e-6 && M.z() >= 0.0f)
	{
		float CosTheta = Frame::CosTheta(M);

		if (CosTheta == 0.0f)
		{
			return 0.0f;
		}

		float TanTheta = Frame::TanTheta(M);

		if (std::isinf(TanTheta))
		{
			return 0.0f;
		}

		float Alpha2 = Alpha * Alpha;

		return std::expf(-1.0f * std::powf(TanTheta, 2.0f) / Alpha2) / (float(M_PI) * Alpha2 * std::powf(CosTheta, 3.0f));
	}

	return 0.0f;
}

NAMESPACE_END