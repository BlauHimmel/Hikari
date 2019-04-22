#include <bsdf\RoughPlasticBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>
#include <core\Timer.hpp>
#include <core\Intersection.hpp>
#include <core\Sampling.hpp>
#include <core\RoughTransmitance.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(RoughPlasticBSDF, XML_BSDF_ROUGH_PLASTIC);

RoughPlasticBSDF::RoughPlasticBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_ROUGH_PLASTIC_INT_IOR, DEFAULT_BSDF_ROUGH_PLASTIC_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_ROUGH_PLASTIC_EXT_IOR, DEFAULT_BSDF_ROUGH_PLASTIC_EXT_IOR);

	/* Speculer reflectance  */
	m_pKs = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_ROUGH_PLASTIC_KS, DEFAULT_BSDF_ROUGH_PLASTIC_KS));

	/* Diffuse reflectance  */
	m_pKd = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_ROUGH_PLASTIC_KD, DEFAULT_BSDF_ROUGH_PLASTIC_KD));

	/* Roughness  */
	m_pAlpha = new ConstantFloatTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_PLASTIC_ALPHA, DEFAULT_BSDF_ROUGH_PLASTIC_ALPHA), float(MIN_ALPHA), float(MAX_ALPHA)));

	/* Account for nonlinear color shifts due to internal scattering ? */
	m_bNonlinear = PropList.GetBoolean(XML_BSDF_ROUGH_PLASTIC_NONLINEAR, DEFAULT_BSDF_ROUGH_PLASTIC_NONLINEAR);

	/* Distribution type */
	std::string TypeStr = PropList.GetString(XML_BSDF_ROUGH_PLASTIC_TYPE, DEFAULT_BSDF_ROUGH_PLASTIC_TYPE);
	if (TypeStr == XML_BSDF_BECKMANN)
	{
		m_Type = MicrofacetDistribution::EBeckmann;
		filesystem::path Filename = GetFileResolver()->resolve(XML_BSDF_ROUGH_PLASTIC_BECKMANN_RFT_DATA);
		m_pExtData = new RoughTransmittance(Filename.str());
	}
	else if (TypeStr == XML_BSDF_GGX)
	{
		m_Type = MicrofacetDistribution::EGGX;
		filesystem::path Filename = GetFileResolver()->resolve(XML_BSDF_ROUGH_PLASTIC_GGX_RFT_DATA);
		m_pExtData = new RoughTransmittance(Filename.str());
	}
	else
	{
		throw HikariException("Unexpected distribution type : %s", TypeStr);
	}

	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
	m_InvEta2 = m_InvEta * m_InvEta;
}

RoughPlasticBSDF::~RoughPlasticBSDF()
{
	delete m_pKs;
	delete m_pKd;
	delete m_pAlpha;
	delete m_pIntData;
	delete m_pExtData;
}

Color3f RoughPlasticBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);

	if (CosThetaI <= 0.0f)
	{
		return Color3f(0.0f);
	}

	Point2f Sample2D(Sample);

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	MicrofacetDistribution Distribution(m_Type, Alpha, Alpha);

	float SpecularPDF = 1.0f - m_pExtData->Eval(CosThetaI, Distribution.GetAlpha());
	SpecularPDF = (SpecularPDF * m_SpecularSamplingWeight) /
		(SpecularPDF * m_SpecularSamplingWeight + (1.0f - SpecularPDF) * (1.0f - m_SpecularSamplingWeight));
	float DiffusePDF = 1.0f - SpecularPDF;

	bool bSpecular;
	if (Sample2D.y() < SpecularPDF)
	{
		Sample2D.y() /= SpecularPDF;
		bSpecular = true;
	}
	else
	{
		Sample2D.y() = (Sample2D.y() - SpecularPDF) / (1.0f - SpecularPDF);
		bSpecular = false;
	}

	if (bSpecular)
	{
		float Pdf;
		Vector3f M = Distribution.Sample(Record.Wi, Sample2D, Pdf);
		Record.Wo = Reflect(Record.Wi, M);
		Record.Measure = EMeasure::EDiscrete;
	}
	else
	{
		Record.Wo = Sampling::SquareToCosineHemisphere(Sample2D);
		Record.Measure = EMeasure::ESolidAngle;
	}

	Record.Eta = 1.0f;

	Vector3f H = (Record.Wi + Record.Wo).normalized();
	float J = 1.0f / (4.0f * Record.Wo.dot(H));
	float PDF = Distribution.Pdf(H) * J * SpecularPDF + DiffusePDF * Sampling::SquareToCosineHemispherePdf(Record.Wo);

	if (PDF == 0.0f)
	{
		return Color3f(0.0f);
	}
	else
	{
		return Eval(Record) / PDF;
	}
}

Color3f RoughPlasticBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (Record.Measure == EMeasure::EUnknownMeasure || 
		CosThetaI <= 0.0f || CosThetaO <= 0.0f)
	{
		return Color3f(0.0f);
	}

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	MicrofacetDistribution Distribution(m_Type, Alpha, Alpha);
	
	Color3f Result(0.0f);

	float CosThetaT;
	Vector3f H = (Record.Wi + Record.Wo).normalized();
	float D = Distribution.Eval(H);
	float F = FresnelDielectric(Record.Wi.dot(H), m_Eta, m_InvEta, CosThetaT);
	float G = Distribution.G(Record.Wi, Record.Wi, H);
	float Value = F * D* G / (4.0f * CosThetaI);
	Result += m_pKs->Eval(Record.Isect) * Value;

	Color3f Diffuse = m_pKd->Eval(Record.Isect);
	float T12 = m_pExtData->Eval(CosThetaI, Distribution.GetAlpha());
	float T21 = m_pExtData->Eval(CosThetaO, Distribution.GetAlpha());
	float FDR = 1.0f - m_pIntData->EvalDiffuse(Distribution.GetAlpha());

	if (m_bNonlinear)
	{
		Diffuse /= (Color3f(1.0f) - Diffuse * FDR);
	}
	else
	{
		Diffuse /= (1.0f - FDR);
	}
	Result += Diffuse * (INV_PI * CosThetaO * T12 * T21 * m_InvEta2);

	return Result;
}

float RoughPlasticBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (Record.Measure == EMeasure::EUnknownMeasure ||
		CosThetaI <= 0.0f || CosThetaO <= 0.0f)
	{
		return 0.0f;
	}

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	MicrofacetDistribution Distribution(m_Type, Alpha, Alpha);

	Vector3f H = (Record.Wi + Record.Wo).normalized();
	
	float SpecularPDF = 1.0f - m_pExtData->Eval(CosThetaI, Distribution.GetAlpha());
	SpecularPDF = (SpecularPDF * m_SpecularSamplingWeight) /
		(SpecularPDF * m_SpecularSamplingWeight + (1.0f - SpecularPDF) * (1.0f - m_SpecularSamplingWeight));
	float DiffusePDF = 1.0f - SpecularPDF;

	float Result = 0.0f;
	float J = 1.0f / (4.0f * Record.Wo.dot(H));
	float PDF = Distribution.Pdf(H);
	Result = PDF * J * SpecularPDF;
	Result += DiffusePDF * Sampling::SquareToCosineHemispherePdf(Record.Wo);

	return Result;
}

bool RoughPlasticBSDF::IsDiffuse() const
{
	return true;
}

void RoughPlasticBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_PLASTIC_KS)
	{
		if (m_pKs->IsConstant())
		{
			delete m_pKs;
			m_pKs = (Texture *)(pChildObj);
			if (m_pKs->IsMonochromatic())
			{
				LOG(WARNING) << "Ks texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("RoughPlasticBSDF: tried to specify multiple Ks texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_PLASTIC_KD)
	{
		if (m_pKd->IsConstant())
		{
			delete m_pKd;
			m_pKd = (Texture *)(pChildObj);
			if (m_pKd->IsMonochromatic())
			{
				LOG(WARNING) << "Kd texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("RoughPlasticBSDF: tried to specify multiple Kd texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_PLASTIC_ALPHA)
	{
		if (m_pAlpha->IsConstant())
		{
			delete m_pAlpha;
			m_pAlpha = (Texture *)(pChildObj);
			if (!m_pAlpha->IsMonochromatic())
			{
				LOG(WARNING) << "Alpha texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("RoughPlasticBSDF: tried to specify multiple alpha texture");
		}
	}
	else
	{
		throw HikariException("RoughPlasticBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
}

void RoughPlasticBSDF::Activate()
{
	float KsAvg = m_pKs->GetAverage().GetLuminance();
	float KdAvg = m_pKd->GetAverage().GetLuminance();
	m_SpecularSamplingWeight = KsAvg / (KdAvg + KsAvg);

	m_pExtData->CheckEta(m_Eta);
	m_pExtData->CheckAlpha(m_pAlpha->GetMinimum()[0]);
	m_pExtData->CheckAlpha(m_pAlpha->GetMaximum()[0]);

	m_pIntData = m_pExtData->Clone().release();

	m_pExtData->SetEta(m_Eta);
	m_pIntData->SetEta(m_InvEta);

	if (m_pAlpha->IsConstant())
	{
		m_pExtData->SetAlpha(m_pAlpha->Eval(Intersection())[0]);
	}

	AddBSDFType(EBSDFType::EGlossyReflection);
	AddBSDFType(EBSDFType::EDiffuseReflection);
	if (!m_pKs->IsConstant() || !m_pKd->IsConstant() || !m_pAlpha->IsConstant())
	{
		AddBSDFType(EBSDFType::EUVDependent);
	}
}

std::string RoughPlasticBSDF::ToString() const
{
	return tfm::format(
		"RoughPlasticBSDF[\n"
		"  type = %s,\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  ks = %s,\n"
		"  kd = %s,\n"
		"  alpha = %f,\n"
		"  nonlinear = %s,\n"
		"]",
		MicrofacetDistribution::TypeName(m_Type),
		m_IntIOR,
		m_ExtIOR,
		m_pKs->IsConstant() ? m_pKs->GetAverage().ToString() : Indent(m_pKs->ToString()),
		m_pKd->IsConstant() ? m_pKd->GetAverage().ToString() : Indent(m_pKd->ToString()),
		m_pAlpha->IsConstant() ? std::to_string(m_pAlpha->GetAverage()[0]) : Indent(m_pAlpha->ToString()),
		m_bNonlinear ? "true" : "false"
	);
}

NAMESPACE_END

