#include <bsdf\PlasticBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampling.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PlasticBSDF, XML_BSDF_PLASTIC);

PlasticBSDF::PlasticBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_PLASTIC_INT_IOR, DEFAULT_BSDF_PLASTIC_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_PLASTIC_EXT_IOR, DEFAULT_BSDF_PLASTIC_EXT_IOR);

	/* Speculer reflectance  */
	m_pKs = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_PLASTIC_KS, DEFAULT_BSDF_PLASTIC_KS));

	/* Diffuse reflectance  */
	m_pKd = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_PLASTIC_KD, DEFAULT_BSDF_PLASTIC_KD));

	/* Account for nonlinear color shifts due to internal scattering ? */
	m_bNonlinear = PropList.GetBoolean(XML_BSDF_PLASTIC_NONLINEAR, DEFAULT_BSDF_PLASTIC_NONLINEAR);

	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
	m_InvEta2 = m_InvEta * m_InvEta;

	m_FresnelDiffuseReflectanceInt = ApproxFresnelDiffuseReflectance(m_InvEta);
	m_FresnelDiffuseReflectanceExt = ApproxFresnelDiffuseReflectance(m_Eta);
}

PlasticBSDF::~PlasticBSDF()
{
	delete m_pKs;
	delete m_pKd;
}

Color3f PlasticBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);

	if (CosThetaI <= 0.0f)
	{
		return Color3f(0.0f);
	}

	float CosThetaT;
	float FresnelTermI = FresnelDielectric(CosThetaI, m_Eta, m_InvEta, CosThetaT);

	float SpecularPDF = (FresnelTermI * m_SpecularSamplingWeight) /
		(FresnelTermI * m_SpecularSamplingWeight + (1.0f - FresnelTermI) * (1.0f - m_SpecularSamplingWeight));

	Record.Eta = 1.0f;
	
	if (Sample.x() < SpecularPDF)
	{
		Record.Measure = EMeasure::EDiscrete;
		Record.Wo = Reflect(Record.Wi);
		return m_pKs->Eval(Record.Isect) * FresnelTermI / SpecularPDF;
	}
	else
	{
		Record.Measure = EMeasure::ESolidAngle;
		Record.Wo = Sampling::SquareToCosineHemisphere(
			Point2f((Sample.x() - SpecularPDF) / (1.0f - SpecularPDF), Sample.y())
		);

		float FresnelTermO = FresnelDielectric(Frame::CosTheta(Record.Wo), m_Eta, m_InvEta, CosThetaT);

		Color3f Diffuse = m_pKd->Eval(Record.Isect);
		if (m_bNonlinear)
		{
			Diffuse /= (Color3f(1.0f) - Diffuse * m_FresnelDiffuseReflectanceInt);
		}
		else
		{
			Diffuse /= (1.0f - m_FresnelDiffuseReflectanceInt);
		}

		return Diffuse * (m_InvEta2 * (1.0f - FresnelTermI) * (1.0f - FresnelTermO) / (1.0f - SpecularPDF));
	}
}

Color3f PlasticBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);
	if (Record.Measure == EMeasure::EUnknownMeasure ||
		CosThetaI <= 0.0f ||
		CosThetaO <= 0.0f)
	{
		return Color3f(0.0f);
	}

	bool bSpecular = (Record.Measure == EMeasure::EDiscrete);
	
	float CosThetaT;
	float FresnelTermI = FresnelDielectric(CosThetaI, m_Eta, m_InvEta, CosThetaT);

	if (bSpecular)
	{
		if (std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			return m_pKs->Eval(Record.Isect) * FresnelTermI;
		}

		return Color3f(0.0f);
	}
	else
	{
		float FresnelTermO = FresnelDielectric(CosThetaO, m_Eta, m_InvEta, CosThetaT);
		Color3f Diffuse = m_pKd->Eval(Record.Isect);
		if (m_bNonlinear)
		{
			Diffuse /= (Color3f(1.0f) - Diffuse * m_FresnelDiffuseReflectanceInt);
		}
		else
		{
			Diffuse /= (1.0f - m_FresnelDiffuseReflectanceInt);
		}
	
		return Diffuse * (Sampling::SquareToCosineHemispherePdf(Record.Wo) * m_InvEta2 * (1.0f - FresnelTermI) * (1.0f - FresnelTermO));
	}
}

float PlasticBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (Record.Measure == EMeasure::EUnknownMeasure ||
		CosThetaI <= 0.0f ||
		CosThetaO <= 0.0f)
	{
		return 0.0f;
	}

	bool bSpecular = (Record.Measure == EMeasure::EDiscrete);

	if (bSpecular)
	{
		if (std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			float CosThetaT;
			float FresnelTermI = FresnelDielectric(CosThetaI, m_Eta, m_InvEta, CosThetaT);
			float SpecularPDF = (FresnelTermI * m_SpecularSamplingWeight) /
				(FresnelTermI * m_SpecularSamplingWeight + (1.0f - FresnelTermI) * (1.0f - m_SpecularSamplingWeight));
			return  1.0f - SpecularPDF;
		}
		return 0.0f;
	}
	else
	{
		float CosThetaT;
		float FresnelTermI = FresnelDielectric(CosThetaI, m_Eta, m_InvEta, CosThetaT);
		float SpecularPDF = (FresnelTermI * m_SpecularSamplingWeight) /
			(FresnelTermI * m_SpecularSamplingWeight + (1.0f - FresnelTermI) * (1.0f - m_SpecularSamplingWeight));
		return Sampling::SquareToCosineHemispherePdf(Record.Wo) * (1.0f - SpecularPDF);
	}
}

bool PlasticBSDF::IsDiffuse() const
{
	return true;
}

void PlasticBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_PLASTIC_KD)
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
			throw HikariException("PlasticBSDF: tried to specify multiple Kd texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_PLASTIC_KS)
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
			throw HikariException("PlasticBSDF: tried to specify multiple Ks texture");
		}
	}
	else
	{
		throw HikariException("PlasticBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
}

void PlasticBSDF::Activate()
{
	float KsAvg = m_pKs->GetAverage().GetLuminance();
	float KdAvg = m_pKd->GetAverage().GetLuminance();
	m_SpecularSamplingWeight = KsAvg / (KdAvg + KsAvg);
}

std::string PlasticBSDF::ToString() const
{
	return tfm::format(
		"Plastic[\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  ks = %f,\n"
		"  kd = %s,\n"
		"  nonlinear = %s\n"
		"]",
		m_IntIOR,
		m_ExtIOR,
		m_pKs->IsConstant() ? m_pKs->GetAverage().ToString() : Indent(m_pKs->ToString()),
		m_pKd->IsConstant() ? m_pKd->GetAverage().ToString() : Indent(m_pKd->ToString()),
		m_bNonlinear ? "true" : "false"
	);
}

NAMESPACE_END
