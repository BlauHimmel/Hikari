#include <bsdf\RoughDielectricBSDF.hpp>
#include <core\Frame.hpp>

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
	return Color3f();
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
	return Color3f();
}

float RoughDielectricBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	return 0.0f;
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

