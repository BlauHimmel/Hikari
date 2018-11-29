#include <emitter\AreaLight.hpp>
#include <core\Mesh.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(AreaLight, XML_EMITTER_AREA_LIGHT);

AreaLight::AreaLight(const PropertyList & PropList)
{
	m_Radiance = PropList.GetColor(XML_EMITTER_AREA_LIGHT_RADIANCE);
	m_Type = EEmitterType::EArea;
}

Color3f AreaLight::Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const
{
	if (m_pMesh == nullptr)
	{
		throw HikariException("There is no shape attached to this AreaLight!");
	}

	m_pMesh->SamplePosition(Sample1D, Sample2D, Record.P, Record.N);

	Vector3f Wi = Record.P - Record.Ref;

	Record.Distance = Wi.norm();
	Record.Wi = Wi.normalized();
	Record.pEmitter = this;
	Record.Pdf = Pdf(Record);

	if (Record.Pdf == 0.0f || std::isinf(Record.Pdf))
	{
		return Color3f(0.0f);
	}

	return Eval(Record) / Record.Pdf;
}

float AreaLight::Pdf(const EmitterQueryRecord & Record) const
{
	if (m_pMesh == nullptr)
	{
		throw HikariException("There is no shape attached to this AreaLight!");
	}

	/* Transform the integration variable from the position domain to solid angle domain */
	float GDenominator = std::abs((-1.0f * Record.Wi).dot(Record.N));
	
	if (GDenominator == 0.0f)
	{
		return 0.0f;
	}

	float GNumerator = Record.Distance * Record.Distance;

	return m_pMesh->Pdf() *
		(GNumerator / GDenominator);
}

Color3f AreaLight::Eval(const EmitterQueryRecord & Record) const
{
	if (m_pMesh == nullptr)
	{
		throw HikariException("There is no shape attached to this AreaLight!");
	}

	// Check if the associated normal in emitter query record 
	// and incoming direction are not backfacing
	if (Record.N.dot(Record.Wi) < 0.0f)
	{
		return m_Radiance;
	}

	return Color3f(0.0f);
}

void AreaLight::SetParent(Object * pParentObj)
{
	EClassType ClzType = pParentObj->GetClassType();
	if (ClzType == EClassType::EMesh)
	{
		m_pMesh = (Mesh*)(pParentObj);
	}
	else
	{
		throw HikariException("AreaLight::SetParent(<%s>) is not supported!", ClassTypeName(pParentObj->GetClassType()));
	}
}

std::string AreaLight::ToString() const
{
	return tfm::format(
		"AreaLight[radiance = %s]",
		m_Radiance.ToString()
	);
}

NAMESPACE_END