#include <bsdf\RoughDielectricBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampler.hpp>
#include <pcg32.h>

NAMESPACE_BEGIN

REGISTER_CLASS(RoughDielectricBSDF, XML_BSDF_ROUGH_DIELECTRIC);

RoughDielectricBSDF::RoughDielectricBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_ROUGH_DIELECTRIC_INT_IOR, DEFAULT_BSDF_ROUGH_DIELECTRIC_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_ROUGH_DIELECTRIC_EXT_IOR, DEFAULT_BSDF_ROUGH_DIELECTRIC_EXT_IOR);

	/* Speculer reflectance */
	m_KsReflect = PropList.GetColor(XML_BSDF_ROUGH_DIELECTRIC_KS_REFLECT, DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFLECT);
	
	/* Speculer transmitance */
	m_KsRefract = PropList.GetColor(XML_BSDF_ROUGH_DIELECTRIC_KS_REFRACT, DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFRACT);

	/* Distribution type */
	std::string TypeStr = PropList.GetString(XML_BSDF_ROUGH_DIELECTRIC_TYPE, DEFAULT_BSDF_ROUGH_DIELECTRIC_TYPE);
	if (TypeStr == "beckmann") { m_Type = MicrofacetDistribution::EBeckmann; }
	else if (TypeStr == "ggx") { m_Type = MicrofacetDistribution::EGGX; }
	else { throw HikariException("Unexpected distribution type : %s", TypeStr); }

	bool bAs = PropList.GetBoolean(XML_BSDF_ROUGH_DIELECTRIC_AS, DEFAULT_BSDF_ROUGH_DIELECTRIC_AS);

	if (bAs)
	{
		m_AlphaU = PropList.GetFloat(XML_BSDF_ROUGH_DIELECTRIC_ALPHA_U, DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA_U);
		m_AlphaV = PropList.GetFloat(XML_BSDF_ROUGH_DIELECTRIC_ALPHA_V, DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA_V);
	}
	else
	{
		m_AlphaU = PropList.GetFloat(XML_BSDF_ROUGH_DIELECTRIC_ALPHA, DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA);
		m_AlphaV = m_AlphaU;
	}

	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
}

Color3f RoughDielectricBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Record.Measure = EMeasure::ESolidAngle;
	float CosThetaI = Frame::CosTheta(Record.Wi);

	/* Construct the microfacet distribution matching the
	roughness values at the current surface position.
	(texture will be implemented later) */
	MicrofacetDistribution Distribution(m_Type, m_AlphaU, m_AlphaV);

	/* Trick by Walter et al.: slightly scale the roughness values to
	   reduce importance sampling weights. */
	MicrofacetDistribution SampleDistribution(Distribution);
	SampleDistribution.ScaleAlpha(1.2f - 0.2f * std::sqrt(std::abs(CosThetaI)));

	float Pdf;
	Vector3f M = SampleDistribution.Sample(Signum(CosThetaI) * Record.Wi, Sample, Pdf);

	if (Pdf == 0.0f)
	{
		return Color3f(0.0f);
	}

	float CosThetaT;
	float F = FresnelDielectric(Record.Wi.dot(M), m_Eta, m_InvEta, CosThetaT);

	float Sample1D;
	if (Record.pSampler != nullptr)
	{
		Sample1D = Record.pSampler->Next1D();
	}
	else
	{
		pcg32 Random;
		Sample1D = Random.nextFloat();
		LOG(WARNING) << "Sampler not specified in the BSDFQueryRecord, use pcg32 instead!.";
	}

	bool bSampleReflection;
	if (Sample1D > F)
	{
		bSampleReflection = false;
	}
	else
	{
		bSampleReflection = true;
	}

	Color3f W(1.0f);

	if (bSampleReflection)
	{
		Record.Wo = Reflect(Record.Wi, M);
		Record.Eta = 1.0f;

		float CosThetaO = Frame::CosTheta(Record.Wo);
		if (CosThetaO * CosThetaI <= 0.0f)
		{
			return Color3f(0.0f);
		}

		W *= m_KsReflect;
	}
	else
	{
		if (CosThetaT == 0.0f)
		{
			return Color3f(0.0f);
		}

		/* Perfect specular transmission based on the microfacet normal */
		Record.Wo = Refract(Record.Wi, M, CosThetaT, m_Eta, m_InvEta);
		Record.Eta = (CosThetaT < 0.0f ? m_Eta : m_InvEta);

		float CosThetaO = Frame::CosTheta(Record.Wo);
		if (CosThetaO * CosThetaI >= 0.0f)
		{
			return Color3f(0.0f);
		}

		float Factor = (Record.Mode == ETransportMode::ERadiance) ? (CosThetaT < 0.0f ? m_InvEta : m_Eta) : 1.0f;
		W *= m_KsRefract * (Factor * Factor);
	}

	W *= std::abs(Distribution.Eval(M) * Distribution.G(Record.Wi, Record.Wo, M) * Record.Wi.dot(M) / (Pdf * CosThetaI));
	return W;
}

Color3f RoughDielectricBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (Record.Measure != EMeasure::ESolidAngle || CosThetaI == 0.0f)
	{
		return Color3f(0.0f);
	}

	/* Determine the type of interaction */
	bool bReflect = CosThetaI * CosThetaO > 0.0f;

	Vector3f H;
	if (bReflect)
	{
		H = (Record.Wi + Record.Wo).normalized();
	}
	else
	{
		/* Calculate the transmission half-vector */
		float Eta = CosThetaI > 0.0f ? m_Eta : m_InvEta;
		H = (Record.Wi + Record.Wo * Eta).normalized();
	}

	/* Ensure that the half-vector points into the 
	   same hemisphere as the macrosurface normal */
	H *= Signum(Frame::CosTheta(H));

	/* Construct the microfacet distribution matching the
	roughness values at the current surface position.
	(texture will be implemented later) */
	MicrofacetDistribution Distribution(m_Type, m_AlphaU, m_AlphaV);

	float D = Distribution.Eval(H);

	if (D == 0.0f)
	{
		return Color3f(0.0f);
	}

	float CosThetaT;
	float F = FresnelDielectric(Record.Wi.dot(H), m_Eta, m_InvEta, CosThetaT);
	float G = Distribution.G(Record.Wi, Record.Wo, H);

	if (bReflect)
	{
		return F * D * G / (4.0f * std::abs(CosThetaI)) * m_KsReflect;
	}
	else
	{
		float Eta = CosThetaI > 0.0f ? m_Eta : m_InvEta;
	
		/* Calculate the total amount of transmission */
		float SqrtDenom = Record.Wi.dot(H) + Eta * Record.Wo.dot(H);
		float Value = ((1.0f - F) * D * G * Eta * Eta * Record.Wi.dot(H) * Record.Wo.dot(H)) /
			(CosThetaI * SqrtDenom * SqrtDenom);

		float Factor = (Record.Mode == ETransportMode::ERadiance) ? (CosThetaI < 0.0f ? m_InvEta : m_Eta) : 1.0f;

		return std::abs(Value * Factor * Factor) * m_KsRefract;
	}
}

float RoughDielectricBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	if (Record.Measure != EMeasure::ESolidAngle)
	{
		return 0.0f;
	}
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	bool bReflect = (CosThetaI * CosThetaO > 0.0f);

	Vector3f H;
	float J;

	if (bReflect)
	{
		/* Calculate the reflection half-vector */
		H = (Record.Wi + Record.Wo).normalized();

		/* Jacobian of the half-direction mapping */
		J = 1.0f / (4.0f * Record.Wo.dot(H));
	}
	else
	{
		/* Calculate the transmission half-vector */
		float Eta = CosThetaI > 0.0f ? m_Eta : m_InvEta;
		H = (Record.Wi + Record.Wo * Eta).normalized();

		/* Jacobian of the half-direction mapping */
		float SqrtDenom = Record.Wi.dot(H) + Eta * Record.Wo.dot(H);
		J = (Eta * Eta * Record.Wo.dot(H)) / (SqrtDenom * SqrtDenom);
	}

	/* Ensure that the half-vector points into the
	   same hemisphere as the macrosurface normal */
	H *= Signum(Frame::CosTheta(H));

	/* Construct the microfacet distribution matching the
	  roughness values at the current surface position.
	  (texture will be implemented later) */
	MicrofacetDistribution Distribution(m_Type, m_AlphaU, m_AlphaV);

	/* Trick by Walter et al.: slightly scale the roughness values to
	   reduce importance sampling weights.*/
	Distribution.ScaleAlpha(1.2f - 0.2f * std::sqrt(std::abs(CosThetaI)));

	float CosThetaT;
	float F = FresnelDielectric(Record.Wi.dot(H), m_Eta, m_InvEta, CosThetaT);

	float Pdf = Distribution.Pdf(H) * (bReflect ? F : (1.0f - F));

	return std::abs(Pdf * J);
}

bool RoughDielectricBSDF::IsAnisotropic() const
{
	return true;
}

std::string RoughDielectricBSDF::ToString() const
{
	return tfm::format(
		"RoughDielectric[\n"
		"  type = %s,\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  ksReflect = %s,\n"
		"  ksRefract = %s,\n"
		"  alphaU = %f,\n"
		"  alphaV = %f\n"
		"]",
		MicrofacetDistribution::TypeName(m_Type),
		m_IntIOR,
		m_ExtIOR,
		m_KsReflect.ToString(),
		m_KsRefract.ToString(),
		m_AlphaU,
		m_AlphaV
	);
}

NAMESPACE_END

