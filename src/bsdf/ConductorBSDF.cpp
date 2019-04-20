#include <bsdf\ConductorBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(ConductorBSDF, XML_BSDF_CONDUCTOR);

ConductorBSDF::ConductorBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_CONDUCTOR_INT_IOR, DEFAULT_BSDF_CONDUCTOR_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_CONDUCTOR_EXT_IOR, DEFAULT_BSDF_CONDUCTOR_EXT_IOR);

	/* Extinction coefficient */
	m_K = PropList.GetColor(XML_BSDF_CONDUCTOR_K, DEFAULT_BSDF_CONDUCTOR_K);

	/* Speculer reflectance  */
	m_pKs = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_CONDUCTOR_KS, DEFAULT_BSDF_CONDUCTOR_KS));

	m_Eta = Color3f(m_IntIOR / m_ExtIOR);
	m_EtaK = m_K / m_ExtIOR;
}

ConductorBSDF::~ConductorBSDF()
{
	delete m_pKs;
}

Color3f ConductorBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Record.Measure = EMeasure::EDiscrete;

	float CosThetaI = Frame::CosTheta(Record.Wi);

	if (CosThetaI <= 0.0f)
	{
		return Color3f(0.0f);
	}

	Color3f FresnelTerm = FresnelConductor(CosThetaI, m_Eta, m_EtaK);

	Record.Wo = Reflect(Record.Wi);
	Record.Eta = 1.0f;

	return m_pKs->Eval(Record.Isect) * Color3f(FresnelTerm);
}

Color3f ConductorBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (CosThetaI > 0.0f && CosThetaO > 0.0f && Record.Measure == EMeasure::EDiscrete &&
		std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
	{
		return m_pKs->Eval(Record.Isect) * FresnelConductor(CosThetaI, m_Eta, m_EtaK);
	}

	return Color3f(0.0f);
}

float ConductorBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (CosThetaI > 0.0f && CosThetaO > 0.0f && Record.Measure == EMeasure::EDiscrete &&
		std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
	{
		return 1.0f;
	}

	return 0.0f;
}

void ConductorBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_CONDUCTOR_KS)
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
			throw HikariException("ConductorBSDF: tried to specify multiple ks texture");
		}
	}
	else
	{
		throw HikariException("ConductorBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
}

std::string ConductorBSDF::ToString() const
{
	return tfm::format(
		"Conductor[\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  k = %s,\n"
		"  ks = %s\n"
		"]", 
		m_IntIOR,
		m_ExtIOR,
		m_K.ToString(),
		m_pKs->IsConstant() ? m_pKs->GetAverage().ToString() : Indent(m_pKs->ToString())
	);
}

NAMESPACE_END