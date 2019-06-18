#include <bsdf\RoughCoatingBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>
#include <core\RoughTransmitance.hpp>
#include <core\Intersection.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(RoughCoatingBSDF, XML_BSDF_ROUGH_COATING);

RoughCoatingBSDF::RoughCoatingBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_ROUGH_COATING_INT_IOR, DEFAULT_BSDF_ROUGH_COATING_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_ROUGH_COATING_EXT_IOR, DEFAULT_BSDF_ROUGH_COATING_EXT_IOR);

	/* Layer's thickness using the inverse units of sigmaA */
	m_Thickness = PropList.GetFloat(XML_BSDF_ROUGH_COATING_THICKNESS, DEFAULT_BSDF_ROUGH_COATING_THICKNESS);

	/* Absorption within the layer */
	m_pSigmaA = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_ROUGH_COATING_SIGMA_A, DEFAULT_BSDF_ROUGH_COATING_SIGMA_A));

	/* Specular reflectance */
	m_pKs = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_ROUGH_COATING_KS, DEFAULT_BSDF_ROUGH_COATING_KS));

	/* Roughness */
	m_pAlpha = new ConstantFloatTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_COATING_ALPHA, DEFAULT_BSDF_ROUGH_COATING_ALPHA), float(MIN_ALPHA), float(MAX_ALPHA)));

	/* Distribution type */
	std::string TypeStr = PropList.GetString(XML_BSDF_ROUGH_COATING_TYPE, DEFAULT_BSDF_ROUGH_COATING_TYPE);
	if (TypeStr == XML_BSDF_BECKMANN)
	{ 
		m_Type = MicrofacetDistribution::EBeckmann;
		filesystem::path Filename = GetFileResolver()->resolve(XML_BSDF_ROUGH_COATING_BECKMANN_RFT_DATA);
		m_pData = new RoughTransmittance(Filename.str());
	}
	else if (TypeStr == XML_BSDF_GGX)
	{ 
		m_Type = MicrofacetDistribution::EGGX;
		filesystem::path Filename = GetFileResolver()->resolve(XML_BSDF_ROUGH_COATING_GGX_RFT_DATA);
		m_pData = new RoughTransmittance(Filename.str());
	}
	else { throw HikariException("Unexpected distribution type : %s", TypeStr); }

	m_pNestedBSDF = nullptr;

	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
}

RoughCoatingBSDF::~RoughCoatingBSDF()
{
	delete m_pAlpha;
	delete m_pSigmaA;
	delete m_pKs;
	delete m_pNestedBSDF;
	delete m_pData;
}

Color3f RoughCoatingBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	MicrofacetDistribution Distribution(m_Type, Alpha, Alpha);

	float AbsCosThetaI = std::abs(Frame::CosTheta(Record.Wi));
	float R12 = 1.0f - m_pData->Eval(AbsCosThetaI, Distribution.GetAlpha());
	float SpecularPDF = (R12 * m_SpecularSamplingWeight) /
		(R12 * m_SpecularSamplingWeight + (1.0f - R12) * (1.0f - m_SpecularSamplingWeight));

	bool bSpecular;

	Point2f Sample2D(Sample);

	if (Sample2D.x() < SpecularPDF)
	{
		bSpecular = true;
		Sample2D.x() /= SpecularPDF;
	}
	else
	{
		bSpecular = false;
		Sample2D.x() = (Sample2D.x() - SpecularPDF) / (1.0f - SpecularPDF);
	}

	if (bSpecular)
	{
		float Pdf;
		Vector3f M = Distribution.Sample(Record.Wi, Sample2D, Pdf);
		Record.Wo = Reflect(Record.Wi, M);
		Record.Measure = EMeasure::EDiscrete;
		Record.Eta = 1.0f;

		if (Frame::CosTheta(Record.Wo) * Frame::CosTheta(Record.Wo) <= 0.0f)
		{
			return Color3f(0.0f);
		}
	}
	else
	{
		Vector3f WiBackup = Record.Wi;
		Record.Wi = RefractIn(Record.Wi);
		Color3f Result = m_pNestedBSDF->Sample(Record, Sample2D);
		Record.Wi = WiBackup;

		if (Result.isZero())
		{
			return Color3f(0.0f);
		}

		Record.Wo = RefractOut(Record.Wo);

		if (Record.Wo.isZero())
		{
			return Color3f(0.0f);
		}
	}

	float PDF = Pdf(Record);
	if (PDF == 0.0f)
	{
		return Color3f(0.0f);
	}

	return Eval(Record) / PDF;
}

Color3f RoughCoatingBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (CosThetaI * CosThetaO <= 0.0f)
	{
		return Color3f(0.0f);
	}

	Color3f Result(0.0f);

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	MicrofacetDistribution Distribution(m_Type, Alpha, Alpha);

	Vector3f H = (Record.Wi + Record.Wo).normalized() * Signum(Frame::CosTheta(Record.Wo));
	
	float CosThetaT;

	float D = Distribution.Eval(H);
	float F = FresnelDielectric(std::abs(Record.Wi.dot(H)), m_Eta, m_InvEta, CosThetaT);
	float G = Distribution.G(Record.Wi, Record.Wo, H);

	Color3f SpecularValue = m_pKs->Eval(Record.Isect) * F * D * G / (4.0f * std::abs(CosThetaI));

	Result += SpecularValue;

	BSDFQueryRecord RecordInt(Record);
	RecordInt.Wi = RefractIn(Record.Wi);
	RecordInt.Wo = RefractIn(Record.Wo);

	Color3f NestedValue = m_pNestedBSDF->Eval(RecordInt) *
		m_pData->Eval(std::abs(CosThetaI), Distribution.GetAlpha()) *
		m_pData->Eval(std::abs(CosThetaO), Distribution.GetAlpha());

	Color3f SigmaA = m_pSigmaA->Eval(Record.Isect) * m_Thickness;
	if (!SigmaA.isZero())
	{
		NestedValue *= (-SigmaA *
			(1.0f / std::abs(Frame::CosTheta(RecordInt.Wi)) + 1.0f / std::abs(Frame::CosTheta(RecordInt.Wo)))
			).exp();
	}

	if (Record.Measure == EMeasure::ESolidAngle)
	{
		NestedValue *= (m_InvEta * m_InvEta * Frame::CosTheta(Record.Wo) / Frame::CosTheta(RecordInt.Wo));
	}

	Result += NestedValue;

	return Result;
}

float RoughCoatingBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	Vector3f H = (Record.Wi + Record.Wo).normalized() * Signum(Frame::CosTheta(Record.Wo));

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	MicrofacetDistribution Distribution(m_Type, Alpha, Alpha);

	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	float AbsCosThetaI = std::abs(CosThetaI);
	float R12 = 1.0f - m_pData->Eval(AbsCosThetaI, Distribution.GetAlpha());
	float SpecularPDF = (R12 * m_SpecularSamplingWeight) /
		(R12 * m_SpecularSamplingWeight + (1.0f - R12) * (1.0f - m_SpecularSamplingWeight));
	float NestedPDF = 1.0f - SpecularPDF;

	float Result = 0.0f;
	if (CosThetaI * CosThetaO > 0.0f)
	{
		float J = 1.0f / (4.0f * std::abs(Record.Wo.dot(H)));
		/* Pdf with respect to H */
		float PDF = Distribution.Pdf(H);
		Result = PDF * J * SpecularPDF;
	}

	BSDFQueryRecord RecordInt(Record);
	RecordInt.Wi = RefractIn(Record.Wi);
	RecordInt.Wo = RefractIn(Record.Wo);

	float PDF = m_pNestedBSDF->Pdf(RecordInt);

	if (RecordInt.Measure == EMeasure::ESolidAngle)
	{
		PDF *= m_InvEta * m_InvEta * CosThetaO / Frame::CosTheta(RecordInt.Wo);
	}

	Result += PDF * NestedPDF;

	return Result;
}

bool RoughCoatingBSDF::IsDiffuse() const
{
	return m_pNestedBSDF->IsDiffuse();
}

bool RoughCoatingBSDF::IsAnisotropic() const
{
	return m_pNestedBSDF->IsAnisotropic();
}

void RoughCoatingBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_COATING_KS)
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
			throw HikariException("RoughCoatingBSDF: tried to specify multiple Ks texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_COATING_SIGMA_A)
	{
		if (m_pSigmaA->IsConstant())
		{
			delete m_pSigmaA;
			m_pSigmaA = (Texture *)(pChildObj);
			if (m_pSigmaA->IsMonochromatic())
			{
				LOG(WARNING) << "SigmaA texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("RoughCoatingBSDF: tried to specify multiple SigmaA texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_COATING_ALPHA)
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
			throw HikariException("RoughCoatingBSDF: tried to specify multiple alpha texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::EBSDF)
	{
		if (m_pNestedBSDF == nullptr)
		{
			m_pNestedBSDF = (BSDF *)(pChildObj);
		}
		else
		{
			throw HikariException("RoughCoatingBSDF: tried to specify multiple nested BSDF");
		}
	}
}

void RoughCoatingBSDF::Activate()
{
	Color3f Temp = (m_pSigmaA->GetAverage() * (-2.0f * m_Thickness)).exp();
	float AvgAbsorption = (Temp[0] + Temp[1] + Temp[2]) / 3.0f;
	m_SpecularSamplingWeight = 1.0f / (AvgAbsorption + 1.0f);

	m_pData->CheckEta(m_Eta);
	m_pData->CheckAlpha(m_pAlpha->GetMinimum()[0]);
	m_pData->CheckAlpha(m_pAlpha->GetMaximum()[0]);
	m_pData->SetEta(m_Eta);

	if (m_pAlpha->IsConstant())
	{
		m_pData->SetAlpha(m_pAlpha->Eval(Intersection())[0]);
	}

	if (m_pNestedBSDF == nullptr)
	{
		throw HikariException("RoughCoatingBSDF needs a nested BSDF!");
	}

	AddBSDFType(m_pNestedBSDF->GetBSDFTypes());
	AddBSDFType(EBSDFType::EGlossyReflection);
	if (!m_pSigmaA->IsConstant() || !m_pKs->IsConstant() || !m_pAlpha->IsConstant())
	{
		AddBSDFType(EBSDFType::EUVDependent);
	}
}

std::string RoughCoatingBSDF::ToString() const
{
	return tfm::format(
		"RoughCoatingBSDF[\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  thickness = %f,\n"
		"  sigmaA = %s,\n"
		"  alpha = %s,\n"
		"  ks = %s,\n"
		"  nestedBSDF = %s\n"
		"]",
		m_IntIOR,
		m_ExtIOR,
		m_Thickness,
		m_pSigmaA->IsConstant() ? m_pSigmaA->GetAverage().ToString() : Indent(m_pSigmaA->ToString()),
		m_pAlpha->IsConstant() ? std::to_string(m_pAlpha->GetAverage()[0]) : Indent(m_pAlpha->ToString()),
		m_pKs->IsConstant() ? m_pKs->GetAverage().ToString() : Indent(m_pKs->ToString()),
		Indent(m_pNestedBSDF->ToString())
	);
}

Vector3f RoughCoatingBSDF::RefractIn(const Vector3f & Wi) const
{
	float CosThetaI = Frame::CosTheta(Wi);
	float InvEta = m_InvEta;

	bool bEntering = CosThetaI > 0.0f;

	/* Snell's law */
	float SinThetaTSqr = InvEta * InvEta * Frame::SinTheta2(Wi);

	if (SinThetaTSqr >= 1.0f)
	{
		/* Total internal reflection */
		return Vector3f(0.0f);
	}

	float CosThetaT = std::sqrt(1.0f - SinThetaTSqr);
	return Vector3f(InvEta * Wi.x(), InvEta * Wi.y(), bEntering ? CosThetaT : -CosThetaT);
}

Vector3f RoughCoatingBSDF::RefractOut(const Vector3f & Wi) const
{
	float CosThetaI = Frame::CosTheta(Wi);
	float InvEta = m_Eta;

	bool bEntering = CosThetaI > 0.0f;

	/* Snell's law */
	float SinThetaTSqr = InvEta * InvEta * Frame::SinTheta2(Wi);

	if (SinThetaTSqr >= 1.0f)
	{
		/* Total internal reflection */
		return Vector3f(0.0f);
	}

	float CosThetaT = std::sqrt(1.0f - SinThetaTSqr);
	return Vector3f(InvEta * Wi.x(), InvEta * Wi.y(), bEntering ? CosThetaT : -CosThetaT);
}

NAMESPACE_END