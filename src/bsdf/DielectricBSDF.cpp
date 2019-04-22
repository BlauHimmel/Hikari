#include <bsdf\DielectricBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(DielectricBSDF, XML_BSDF_DIELECTRIC);

DielectricBSDF::DielectricBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_DIELECTRIC_INT_IOR, DEFAULT_BSDF_DIELECTRIC_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_DIELECTRIC_EXT_IOR, DEFAULT_BSDF_DIELECTRIC_EXT_IOR);

	/* Specular reflectance */
	m_pKsReflect = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_DIELECTRIC_KS_REFLECT, DEFAULT_BSDF_DIELECTRIC_KS_REFLECT));

	/* Specular transmittance */
	m_pKsRefract = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_DIELECTRIC_KS_REFRACT, DEFAULT_BSDF_DIELECTRIC_KS_REFRACT));

	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
}

DielectricBSDF::~DielectricBSDF()
{
	delete m_pKsReflect;
	delete m_pKsRefract;
}

Color3f DielectricBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Record.Measure = EMeasure::EDiscrete;

	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaT;
	float FresnelTerm = FresnelDielectric(CosThetaI, m_Eta, m_InvEta, CosThetaT);

	// Reflection
	if (Sample.x() < FresnelTerm)
	{
		Record.Wo = Reflect(Record.Wi);
		Record.Eta = 1.0f;

		return m_pKsReflect->Eval(Record.Isect);
	}
	// Refraction
	else
	{
		Record.Wo = Refract(Record.Wi, CosThetaT, m_Eta, m_InvEta);
		Record.Eta = CosThetaT < 0.0f ? m_Eta : m_InvEta;
		/* Radiance must be scaled to account for the solid angle compression
		that occurs when crossing the interface. */
		float Factor = (Record.Mode == ETransportMode::ERadiance) ? (CosThetaT < 0.0f ? m_InvEta : m_Eta) : 1.0f;
		return m_pKsRefract->Eval(Record.Isect) * (Factor * Factor);
	}
}

Color3f DielectricBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);
	float CosThetaT;

	float FresnelTerm = FresnelDielectric(CosThetaI, m_Eta, m_InvEta, CosThetaT);

	if (CosThetaI * CosThetaO >= 0.0f)
	{
		if (std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			return m_pKsReflect->Eval(Record.Isect) * FresnelTerm;
		}
	}
	else
	{
		if (std::abs(Refract(Record.Wi, CosThetaT, m_Eta, m_InvEta).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			float Factor = (Record.Mode == ETransportMode::ERadiance) ? (CosThetaT < 0.0f ? m_InvEta : m_Eta) : 1.0f;
			return m_pKsRefract->Eval(Record.Isect) * (Factor * Factor) * (1.0f - FresnelTerm);
		}
	}

	return Color3f(0.0f);
}

float DielectricBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);
	float CosThetaT;

	float FresnelTerm = FresnelDielectric(CosThetaI, m_Eta, m_InvEta, CosThetaT);

	if (CosThetaI * CosThetaO >= 0.0f)
	{
		if (std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			return FresnelTerm;
		}
	}
	else
	{
		if (std::abs(Refract(Record.Wi, CosThetaT, m_Eta, m_InvEta).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			return 1.0f - FresnelTerm;
		}
	}

	return 0.0f;
}

void DielectricBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_DIELECTRIC_KS_REFLECT)
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
			throw HikariException("DielectricBSDF: tried to specify multiple KsReflect texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_DIELECTRIC_KS_REFRACT)
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
			throw HikariException("DielectricBSDF: tried to specify multiple KsRefract texture");
		}
	}
	else
	{
		throw HikariException("DielectricBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
}

void DielectricBSDF::Activate()
{
	AddBSDFType(EBSDFType::EDeltaReflection);
	AddBSDFType(EBSDFType::EDeltaTransmission);
	if (!m_pKsRefract->IsConstant() || !m_pKsReflect->IsConstant())
	{
		AddBSDFType(EBSDFType::EUVDependent);
	}
}

std::string DielectricBSDF::ToString() const
{
	return tfm::format(
		"Dielectric[\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  ksReflect = %s,\n"
		"  ksRefract = %s\n"
		"]", 
		m_IntIOR, 
		m_ExtIOR,
		m_pKsReflect->IsConstant() ? m_pKsReflect->GetAverage().ToString() : Indent(m_pKsReflect->ToString()),
		m_pKsRefract->IsConstant() ? m_pKsRefract->GetAverage().ToString() : Indent(m_pKsRefract->ToString())
	);
}

NAMESPACE_END
