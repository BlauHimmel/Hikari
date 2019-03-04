#include <core\MicrofacetDistribution.hpp>
#include <core\Frame.hpp>

NAMESPACE_BEGIN

MicrofacetDistribution::MicrofacetDistribution(EType Type, float Alpha) :
	m_Type(Type),
	m_AlphaU(Alpha),
	m_AlphaV(Alpha),
	m_ExponentU(0.0f),
	m_ExponentV(0.0f)
{
	m_AlphaU = std::max(m_AlphaU, 1e-4f);
	m_AlphaV = std::max(m_AlphaV, 1e-4f);

	if (m_Type == EPhong)
	{
		// Ref: Microfacet Models for Refraction through Rough Surfaces 
		// Sec 5.2 Phong Distribution
		// Walter et al.
		m_ExponentU = std::max(2.0f / (m_AlphaU * m_AlphaU) - 2.0f, 0.0f);
		m_ExponentV = std::max(2.0f / (m_AlphaV * m_AlphaV) - 2.0f, 0.0f);
	}
}

MicrofacetDistribution::MicrofacetDistribution(EType Type, float AlphaU, float AlphaV) :
	m_Type(Type),
	m_AlphaU(AlphaU),
	m_AlphaV(AlphaV),
	m_ExponentU(0.0f),
	m_ExponentV(0.0f)
{
	m_AlphaU = std::max(m_AlphaU, 1e-4f);
	m_AlphaV = std::max(m_AlphaV, 1e-4f);

	if (m_Type == EPhong)
	{
		// Ref: Microfacet Models for Refraction through Rough Surfaces 
		// Sec 5.2 Phong Distribution
		// Walter et al.
		m_ExponentU = std::max(2.0f / (m_AlphaU * m_AlphaU) - 2.0f, 0.0f);
		m_ExponentV = std::max(2.0f / (m_AlphaV * m_AlphaV) - 2.0f, 0.0f);

	}
}

void MicrofacetDistribution::ScaleAlpha(float Scale)
{
	m_AlphaU *= Scale;
	m_AlphaV *= Scale;

	if (m_Type == EPhong)
	{
		// Ref: Microfacet Models for Refraction through Rough Surfaces 
		// Sec 5.2 Phong Distribution
		// Walter et al.
		m_ExponentU = std::max(2.0f / (m_AlphaU * m_AlphaU) - 2.0f, 0.0f);
		m_ExponentV = std::max(2.0f / (m_AlphaV * m_AlphaV) - 2.0f, 0.0f);
	}
}

float MicrofacetDistribution::Eval(const Vector3f & M) const
{
	if (Frame::CosTheta(M) <= 0.0f)
	{
		return 0.0f;
	}

	float CosTheta2 = Frame::CosTheta2(M);
	float BeckmannExponent = ((M.x() * M.x()) / (m_AlphaU * m_AlphaU) + (M.y() * M.y()) / (m_AlphaV * m_AlphaV)) / CosTheta2;

	float Result;
	
	if (m_Type == EBeckmann)
	{
		/* Beckmann distribution function for Gaussian random surfaces - [Walter 2005] evaluation */
		Result = std::exp(-BeckmannExponent) / (float(M_PI) * m_AlphaU * m_AlphaV * CosTheta2 * CosTheta2);
	}
	else if (m_Type == EGGX)
	{
		/* GGX / Trowbridge-Reitz distribution function for rough surfaces */
		float Root = (1.0f + BeckmannExponent) * CosTheta2;
		Result = 1.0f / (float(M_PI) * m_AlphaU * m_AlphaV * Root * Root);
	}
	else if (m_Type == EPhong)
	{
		/* Isotropic case: Phong distribution. Anisotropic case: Ashikhmin-Shirley distribution */
		float Exponent = InterpolatePhongExponent(M);
		Result = std::sqrt((m_ExponentU + 2.0f) * (m_ExponentV + 2.0f)) * float(INV_TWOPI) * std::pow(Frame::CosTheta(M), Exponent);
	}
	else
	{
		LOG(ERROR) << "Invalid distribution type!";
		return -1.0f;
	}

	/* Prevent potential numerical issues in other stages of the model */
	if (Result * Frame::CosTheta(M) < 1e-20f)
	{
		Result = 0.0f;
	}

	return Result;
}

Vector3f MicrofacetDistribution::Sample(const Vector3f & Wi, const Point2f & Sample, float & Pdf) const
{
	return SampleAll(Sample, Pdf);
}

float MicrofacetDistribution::Pdf(const Vector3f & M) const
{
	return PdfAll(M);
}

float MicrofacetDistribution::SmithG1(const Vector3f & V, const Vector3f & M) const
{
	/* Ensure consistent orientation (can't see the back
	of the microfacet from the front and vice versa) */
	if (V.dot(M) * Frame::CosTheta(V) <= 0.0f)
	{
		return 0.0f;
	}

	/* Perpendicular incidence -- no shadowing/masking */
	float TanTheta = std::abs(Frame::TanTheta(V));
	if (TanTheta == 0.0f)
	{
		return 1.0f;
	}

	float Alpha = ProjectRoughness(V);

	if (m_Type == EPhong || m_Type == EBeckmann)
	{
		float A = 1.0f / (Alpha * TanTheta);
		if (A >= 1.6f)
		{
			return 1.0f;
		}

		float A2 = A * A;
		return (3.535f * A + 2.181f * A2) / (1.0f + 2.276f * A + 2.577f * A2);
	}
	else if (m_Type == EGGX)
	{
		float Root = Alpha * TanTheta;
		return 2.0f / (1.0f + Hypot2(1.0f, Root));
	}
	else
	{
		LOG(ERROR) << "Invalid distribution type!";
		return -1.0f;
	}
}

float MicrofacetDistribution::G(const Vector3f & Wi, const Vector3f & Wo, const Vector3f & M) const
{
	return SmithG1(Wi, M) * SmithG1(Wo, M);
}

std::string MicrofacetDistribution::TypeName(EType Type)
{
	switch (Type)
	{
		case EBeckmann: return "beckmann"; break;
		case EGGX: return "ggx"; break;
		case EPhong: return "phong"; break;
		default: return "invalid"; break;
	}
}

std::string MicrofacetDistribution::ToString() const
{
	return tfm::format(
		"MicrofacetDistribution[type = %s, AlphaU = %f, AlphaV = %f]",
		TypeName(m_Type),
		m_AlphaU,
		m_AlphaV
	);
}

Vector3f MicrofacetDistribution::SampleAll(const Point2f & Sample, float & Pdf) const
{
	/* The azimuthal component is always selected uniformly regardless of the distribution */
	float CosThetaM = 0.0f;
	float SinPhiM, CosPhiM;
	float AlphaSqr;

	if (m_Type == EBeckmann)
	{
		/* Beckmann distribution function for Gaussian random surfaces */
		if (IsIsotropic())
		{
			/* Sample phi component (isotropic case) */
			SinCos((2.0f * float(M_PI)) * Sample.y(), &SinPhiM, &CosPhiM);
			AlphaSqr = m_AlphaU * m_AlphaU;
		}
		else
		{
			/* Sample phi component (anisotropic case) */
			float PhiM = std::atan(m_AlphaV / m_AlphaU * std::tan(float(M_PI) + 2.0f * float(M_PI) * Sample.y())) + 
				float(M_PI) * std::floor(2 * Sample.y() + 0.5f);
			SinCos(PhiM, &SinPhiM, &CosPhiM);
			float CosSc = CosPhiM / m_AlphaU, SinSc = SinPhiM / m_AlphaV;
			AlphaSqr = 1.0f / (CosSc * CosSc + SinSc * SinSc);
		}

		/* Sample theta component */
		float TanThetaMSqr = AlphaSqr * -std::log(1.0f - Sample.x());
		CosThetaM = 1.0f / std::sqrt(1.0f + TanThetaMSqr);

		/* Compute probability density of the sampled position */
		Pdf = (1.0f - Sample.x()) / (float(M_PI) * m_AlphaU * m_AlphaV * CosThetaM * CosThetaM * CosThetaM);
	}
	else if (m_Type == EGGX)
	{
		/* GGX / Trowbridge-Reitz distribution function for rough surfaces */
		if (IsIsotropic())
		{
			/* Sample phi component (isotropic case) */
			SinCos((2.0f * float(M_PI)) * Sample.y(), &SinPhiM, &CosPhiM);
			/* Sample theta component */
			AlphaSqr = m_AlphaU * m_AlphaU;
		}
		else 
		{
			/* Sample phi component (anisotropic case) */
			float PhiM = std::atan(m_AlphaV / m_AlphaU * std::tan(float(M_PI) + 2.0f * float(M_PI) * Sample.y())) + 
				float(M_PI) * std::floor(2.0f * Sample.y() + 0.5f);
			SinCos(PhiM, &SinPhiM, &CosPhiM);
			float CosSc = CosPhiM / m_AlphaU, SinSc = SinPhiM / m_AlphaV;
			AlphaSqr = 1.0f / (CosSc * CosSc + SinSc * SinSc);
		}

		/* Sample theta component */
		float TanThetaMSqr = AlphaSqr * Sample.x() / (1.0f - Sample.x());
		CosThetaM = 1.0f / std::sqrt(1.0f + TanThetaMSqr);

		/* Compute probability density of the sampled position */
		float Temp = 1.0f + TanThetaMSqr / AlphaSqr;
		Pdf = float(INV_PI) / (m_AlphaU * m_AlphaV * CosThetaM * CosThetaM * CosThetaM * Temp * Temp);
	}
	else if (m_Type == EPhong)
	{
		float PhiM;
		float Exponent;

		if (IsIsotropic())
		{
			PhiM = (2.0f * float(M_PI)) * Sample.y();
			Exponent = m_ExponentU;
		}
		else
		{
			/* Sampling method based on code from PBRT */
			if (Sample.y() < 0.25f)
			{
				SampleFirstQuadrant(4.0f * Sample.y(), PhiM, Exponent);
			}
			else if (Sample.y() < 0.5f)
			{
				SampleFirstQuadrant(4.0f * (0.5f - Sample.y()), PhiM, Exponent);
				PhiM = float(M_PI) - PhiM;
			}
			else if (Sample.y() < 0.75f)
			{
				SampleFirstQuadrant(4.0f * (Sample.y() - 0.5f), PhiM, Exponent);
				PhiM += float(M_PI);
			}
			else
			{
				SampleFirstQuadrant(4.0f * (1.0f - Sample.y()), PhiM, Exponent);
				PhiM = 2.0f * float(M_PI) - PhiM;
			}
		}

		SinCos(PhiM, &SinPhiM, &CosPhiM);
		CosThetaM = std::pow(Sample.x(), 1.0f / (Exponent + 2.0f));
		Pdf = std::sqrt((m_ExponentU + 2.0f) * (m_ExponentV + 2.0f)) * float(INV_TWOPI) * std::pow(CosThetaM, Exponent + 1.0f);
	}
	else
	{
		LOG(ERROR) << "Invalid distribution type!";
		Pdf = -1.0f;
		return Vector3f(-1.0f);
	}

	/* Prevent potential numerical issues in other stages of the model */
	if (Pdf < 1e-20f)
	{
		Pdf = 0.0f;
	}

	float SinThetaM = std::sqrt(std::max(0.0f, 1.0f - CosThetaM * CosThetaM));

	return Vector3f(
		SinThetaM * CosPhiM,
		SinThetaM * SinPhiM,
		CosThetaM
	);
}

float MicrofacetDistribution::PdfAll(const Vector3f & M) const
{
	/* PDF is just D(m) * Cos(ThetaM) */
	return Eval(M) * Frame::CosTheta(M);
}

float MicrofacetDistribution::ProjectRoughness(const Vector3f & V) const
{
	float InvSinTheta2 = 1.0f / Frame::SinTheta2(V);

	if (IsIsotropic() || InvSinTheta2 <= 0.0f)
	{
		return m_AlphaU;
	}

	float CosPhi2 = V.x() * V.x() * InvSinTheta2;
	float SinPhi2 = V.y() * V.y() * InvSinTheta2;

	return std::sqrt(CosPhi2 * m_AlphaU * m_AlphaU + SinPhi2 * m_AlphaV * m_AlphaV);
}

float MicrofacetDistribution::InterpolatePhongExponent(const Vector3f & V) const
{
	float SinTheta2 = Frame::SinTheta2(V);

	if (IsIsotropic() || SinTheta2 <= 1.0f / std::numeric_limits<float>::max())
	{
		return m_ExponentU;
	}

	float InvSinTheta2 = 1.0f / SinTheta2;
	float CosPhi2 = V.x() * V.x() * InvSinTheta2;
	float SinPhi2 = V.y() * V.y() * InvSinTheta2;

	return m_ExponentU * CosPhi2 + m_ExponentV * SinPhi2;
}

void MicrofacetDistribution::SampleFirstQuadrant(float U1, float & Phi, float & Exponent) const
{
	float CosPhi, SinPhi;
	Phi = std::tan(
		std::sqrt((m_ExponentU + 2.0f) / (m_ExponentV + 2.0f)) * 
		std::tan(float(M_PI) * U1 * 0.5f)
	);
	SinCos(Phi, &SinPhi, &CosPhi);

	Exponent = m_ExponentU * CosPhi * CosPhi + m_ExponentV * SinPhi * SinPhi;
}

NAMESPACE_END