#include <bsdf\RoughDiffuseBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampling.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(RoughDiffuseBSDF, XML_BSDF_ROUGH_DIFFUSE);

RoughDiffuseBSDF::RoughDiffuseBSDF(const PropertyList & PropList)
{
	/*  The difuse albedo of the material */
	m_Albedo = PropList.GetColor(XML_BSDF_ROUGH_DIFFUSE_ALBEDO, DEFAULT_BSDF_ROUGH_DIFFUSE_ALBEDO);

	/* Roughness */
	m_Alpha = PropList.GetFloat(XML_BSDF_ROUGH_DIFFUSE_ALPHA, DEFAULT_BSDF_ROUGH_DIFFUSE_ALPHA);

	/* Whether to use full version of the model or a fast approximation */
	m_bFastApprox = PropList.GetBoolean(XML_BSDF_ROUGH_DIFFUSE_FAST_APPROX, DEFAULT_BSDF_ROUGH_DIFFUSE_FAST_APPROX);
}

Color3f RoughDiffuseBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	Record.Measure = EMeasure::ESolidAngle;

	if (CosThetaI <= 0.0f)
	{
		return Color3f(0.0f);
	}

	Record.Wo = Sampling::SquareToCosineHemisphere(Sample);
	Record.Eta = 1.0f;

	float Pdf = Sampling::SquareToCosineHemispherePdf(Record.Wo);

	if (Pdf == 0.0f)
	{
		return Color3f(0.0f);
	}

	return Eval(Record) / Pdf;
}

Color3f RoughDiffuseBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (CosThetaI <= 0.0f || CosThetaO <= 0.0f || Record.Measure != EMeasure::ESolidAngle)
	{
		return Color3f(0.0f);
	}

	/* Conversion from Beckmann-style RMS roughness to
	Oren Nayar style slope-area variance. The factor
	of 1/sqrt(2) was found to be a perfect fit up
	to extreme roughness values (>0.5), after which
	the match is not as good anymore */
	float ConversionFactor = 0.70710678118655f;

	float Sigma = m_Alpha * ConversionFactor;
	float Sigma2 = Sigma * Sigma;

	float SinThetaI = Frame::SinTheta(Record.Wi);
	float SinThetaO = Frame::SinTheta(Record.Wo);

	float CosPhiDiff = 0.0f;
	if (SinThetaI > float(Epsilon) && SinThetaO > float(Epsilon))
	{
		/* Compute cos(PhiO-PhiI) using the half-angle formulae */
		float SinPhiI = Frame::SinPhi(Record.Wi);
		float CosPhiI = Frame::CosPhi(Record.Wi);
		float SinPhiO = Frame::SinPhi(Record.Wo);
		float CosPhiO = Frame::CosPhi(Record.Wo);
		CosPhiDiff = CosPhiI * CosPhiO + SinPhiI * SinPhiO;
	}

	if (m_bFastApprox)
	{
		float A = 1.0f - 0.5f * Sigma2 / (Sigma2 + 0.33f);
		float B = 0.45f * Sigma2 / (Sigma2 + 0.09f);
		float SinAlpha, TanBeta;

		if (Frame::CosTheta(Record.Wi) > Frame::CosTheta(Record.Wo))
		{
			SinAlpha = SinThetaO;
			TanBeta = SinThetaI / Frame::CosTheta(Record.Wi);
		}
		else
		{
			SinAlpha = SinThetaI;
			TanBeta = SinThetaO / Frame::CosTheta(Record.Wo);
		}

		return m_Albedo * (float(INV_PI) * Frame::CosTheta(Record.Wo) *
			(A + B * std::max(CosPhiDiff, 0.0f) * SinAlpha * TanBeta));
	}
	else
	{
		float ThetaI = SafeAcos(Frame::CosTheta(Record.Wi));
		float ThetaO = SafeAcos(Frame::CosTheta(Record.Wo));
		float Alpha = std::max(ThetaI, ThetaO);
		float Beta = std::min(ThetaI, ThetaO);

		float SinAlpha, SinBeta, TanBeta;
		if (Frame::CosTheta(Record.Wi) > Frame::CosTheta(Record.Wo))
		{
			SinAlpha = SinThetaO; 
			SinBeta = SinThetaI;
			TanBeta = SinThetaI / Frame::CosTheta(Record.Wi);
		}
		else
		{
			SinAlpha = SinThetaI;
			SinBeta = SinThetaO;
			TanBeta = SinThetaO / Frame::CosTheta(Record.Wo);
		}

		float Tmp1 = Sigma2 / (Sigma2 + 0.09f);
		float Tmp2 = (4.0f * float(INV_PI * INV_PI)) * Alpha * Beta;
		float Tmp3 = 2.0f * Beta * float(INV_PI);

		float C1 = 1.0f - 0.5f * Sigma2 / (Sigma2 + 0.33f);
		float C2 = 0.45f * Tmp1;
		float C3 = 0.125f * Tmp1 * Tmp2 * Tmp2;
		float C4 = 0.17f * Sigma2 / (Sigma2 + 0.13f);

		if (CosPhiDiff > 0.0f)
		{
			C2 *= SinAlpha;
		}
		else
		{
			C2 *= SinAlpha - Tmp3 * Tmp3 * Tmp3;
		}

		/* Compute tan(0.5 * (Alpha + Beta)) using the half-angle formulae */
		float TanHalf = (SinAlpha + SinBeta) / 
			(SafeSqrt(1.0f - SinAlpha * SinAlpha) + SafeSqrt(1.0f - SinBeta * SinBeta));

		if (std::isinf(TanHalf) || std::isnan(TanHalf))
		{
			return Color3f(0.0f);
		}

		Color3f Rho = m_Albedo;
		Color3f	Lr1 = Rho * (C1 + CosPhiDiff * C2 * TanBeta + (1.0f - std::abs(CosPhiDiff)) * C3 * TanHalf);
		Color3f	Lr2 = Rho * Rho * (C4 * (1.0f - CosPhiDiff * Tmp3 * Tmp3));

		return (Lr1 + Lr2) * (float(INV_PI) * Frame::CosTheta(Record.Wo));
	}
}

float RoughDiffuseBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (CosThetaI <= 0.0f || CosThetaO <= 0.0f || Record.Measure != EMeasure::ESolidAngle)
	{
		return 0.0f;
	}

	return Sampling::SquareToCosineHemispherePdf(Record.Wo);
}

bool RoughDiffuseBSDF::IsDiffuse() const
{
	return true;
}

std::string RoughDiffuseBSDF::ToString() const
{
	return tfm::format(
		"RoughDiffuse[\n"
		"  albedo = %s,\n"
		"  alpha = %f,\n"
		"  fastApprox = %s\n"
		"]",
		m_Albedo.ToString(),
		m_Alpha,
		m_bFastApprox ? "true" : "false"
	);
}

NAMESPACE_END
