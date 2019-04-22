#include <bsdf\RoughDielectricBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampler.hpp>
#include <core\Texture.hpp>
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
	m_pKsReflect = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_ROUGH_DIELECTRIC_KS_REFLECT, DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFLECT));
	
	/* Speculer transmitance */
	m_pKsRefract = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_ROUGH_DIELECTRIC_KS_REFRACT, DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFRACT));

	/* Distribution type */
	std::string TypeStr = PropList.GetString(XML_BSDF_ROUGH_DIELECTRIC_TYPE, DEFAULT_BSDF_ROUGH_DIELECTRIC_TYPE);
	if (TypeStr == XML_BSDF_BECKMANN) { m_Type = MicrofacetDistribution::EBeckmann; }
	else if (TypeStr == XML_BSDF_GGX) { m_Type = MicrofacetDistribution::EGGX; }
	else { throw HikariException("Unexpected distribution type : %s", TypeStr); }

	/* Whether anisotropic */
	m_bAnisotropic = PropList.GetBoolean(XML_BSDF_ROUGH_DIELECTRIC_AS, DEFAULT_BSDF_ROUGH_DIELECTRIC_AS);

	if (m_bAnisotropic)
	{
		m_pAlphaU = new ConstantFloatTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA_U, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_U), float(MIN_ALPHA), float(MAX_ALPHA)));
		m_pAlphaV = new ConstantFloatTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA_V, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_V), float(MIN_ALPHA), float(MAX_ALPHA)));
	}
	else
	{
		m_pAlphaU = new ConstantFloatTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA), float(MIN_ALPHA), float(MAX_ALPHA)));
		m_pAlphaV = m_pAlphaU;
	}

	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
}

RoughDielectricBSDF::~RoughDielectricBSDF()
{
	delete m_pKsReflect;
	delete m_pKsRefract;
	delete m_pAlphaU;
	delete m_pAlphaV;
}

Color3f RoughDielectricBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Record.Measure = EMeasure::ESolidAngle;
	float CosThetaI = Frame::CosTheta(Record.Wi);

	if (CosThetaI == 0.0f)
	{
		return Color3f(0.0f);
	}

	float AlphaU = Clamp(m_pAlphaU->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	float AlphaV = AlphaU;
	if (m_bAnisotropic)
	{
		AlphaV = Clamp(m_pAlphaV->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	}
	MicrofacetDistribution Distribution(m_Type, AlphaU, AlphaV);

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

		W *= m_pKsReflect->Eval(Record.Isect);
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
		W *= m_pKsRefract->Eval(Record.Isect) * (Factor * Factor);
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

	float AlphaU = Clamp(m_pAlphaU->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	float AlphaV = AlphaU;
	if (m_bAnisotropic)
	{
		AlphaV = Clamp(m_pAlphaV->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	}
	MicrofacetDistribution Distribution(m_Type, AlphaU, AlphaV);

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
		return F * D * G / (4.0f * std::abs(CosThetaI)) * m_pKsReflect->Eval(Record.Isect);
	}
	else
	{
		float Eta = CosThetaI > 0.0f ? m_Eta : m_InvEta;
	
		/* Calculate the total amount of transmission */
		float SqrtDenom = Record.Wi.dot(H) + Eta * Record.Wo.dot(H);
		float Value = ((1.0f - F) * D * G * Eta * Eta * Record.Wi.dot(H) * Record.Wo.dot(H)) /
			(CosThetaI * SqrtDenom * SqrtDenom);

		float Factor = (Record.Mode == ETransportMode::ERadiance) ? (CosThetaI < 0.0f ? m_InvEta : m_Eta) : 1.0f;

		return std::abs(Value * Factor * Factor) * m_pKsRefract->Eval(Record.Isect);
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

	if (CosThetaI == 0.0f || CosThetaO == 0.0f)
	{
		return 0.0f;
	}

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

	float AlphaU = Clamp(m_pAlphaU->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	float AlphaV = AlphaU;
	if (m_bAnisotropic)
	{
		AlphaV = Clamp(m_pAlphaV->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	}
	MicrofacetDistribution Distribution(m_Type, AlphaU, AlphaV);

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
	return m_bAnisotropic;
}

void RoughDielectricBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_DIELECTRIC_KS_REFLECT)
	{
		if (m_pKsReflect->IsConstant())
		{
			delete m_pKsReflect;
			m_pKsReflect = (Texture *)(pChildObj);
			if (m_pKsReflect->IsMonochromatic())
			{
				LOG(WARNING) << "KsReflect texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("RoughDielectricBSDF: tried to specify multiple KsReflect texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_DIELECTRIC_KS_REFRACT)
	{
		if (m_pKsRefract->IsConstant())
		{
			delete m_pKsRefract;
			m_pKsRefract = (Texture *)(pChildObj);
			if (m_pKsRefract->IsMonochromatic())
			{
				LOG(WARNING) << "KsRefract texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("RoughDielectricBSDF: tried to specify multiple KsRefract texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && !m_bAnisotropic && Name == XML_BSDF_ROUGH_DIELECTRIC_ALPHA)
	{
		if (m_pAlphaU->IsConstant() && m_pAlphaV->IsConstant())
		{
			delete m_pAlphaU;
			m_pAlphaV = nullptr;
			m_pAlphaU = (Texture *)(pChildObj);
			m_pAlphaV = m_pAlphaU;
			if (!m_pAlphaU->IsMonochromatic())
			{
				LOG(WARNING) << "Alpha texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("RoughDielectricBSDF: tried to specify multiple Alpha texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && m_bAnisotropic && Name == XML_BSDF_ROUGH_DIELECTRIC_ALPHA_U)
	{
		if (m_pAlphaU->IsConstant())
		{
			delete m_pAlphaU;
			m_pAlphaU = (Texture *)(pChildObj);
			if (!m_pAlphaU->IsMonochromatic())
			{
				LOG(WARNING) << "AlphaU texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("RoughDielectricBSDF: tried to specify multiple AlphaU texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && m_bAnisotropic && Name == XML_BSDF_ROUGH_DIELECTRIC_ALPHA_V)
	{
		if (m_pAlphaV->IsConstant())
		{
			delete m_pAlphaV;
			m_pAlphaV = (Texture *)(pChildObj);
			if (!m_pAlphaV->IsMonochromatic())
			{
				LOG(WARNING) << "AlphaV texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("RoughDielectricBSDF: tried to specify multiple AlphaV texture");
		}
	}
	else
	{
		throw HikariException("RoughDielectricBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
}

void RoughDielectricBSDF::Activate()
{
	AddBSDFType(EBSDFType::EGlossyReflection);
	AddBSDFType(EBSDFType::EGlossyTransmission);
	AddBSDFType(EBSDFType::EExtraSampling);
	if (!m_pKsReflect->IsConstant() || !m_pKsRefract->IsConstant() ||
		!m_pAlphaU->IsConstant() || !m_pAlphaV->IsConstant())
	{
		AddBSDFType(EBSDFType::EUVDependent);
	}
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
		m_pKsReflect->IsConstant() ? m_pKsReflect->GetAverage().ToString() : Indent(m_pKsReflect->ToString()),
		m_pKsRefract->IsConstant() ? m_pKsRefract->GetAverage().ToString() : Indent(m_pKsRefract->ToString()),
		m_pAlphaU->IsConstant() ? std::to_string(m_pAlphaU->GetAverage()[0]) : Indent(m_pAlphaU->ToString()),
		m_pAlphaV->IsConstant() ? std::to_string(m_pAlphaV->GetAverage()[0]) : Indent(m_pAlphaV->ToString())
	);
}

NAMESPACE_END

