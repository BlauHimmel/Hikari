#include <emitter\PointLight.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PointLight, XML_EMITTER_POINT_LIGHT)

PointLight::PointLight(const PropertyList & PropList)
{
	m_Radiance = PropList.GetColor(XML_EMITTER_POINT_LIGHT_RADIANCE);
	m_Position = PropList.GetPoint(XML_EMITTER_POINT_LIGHT_POSITION);
	m_Type = EEmitterType::EPoint;
}

Color3f PointLight::Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const
{
	Record.P = m_Position;

	Vector3f Wi = Record.P - Record.Ref;

	Record.Distance = Wi.norm();
	Record.Wi = Wi.normalized();
	Record.N = -Record.Wi;
	Record.pEmitter = this;
	Record.Pdf = 1.0f;

	if (Record.Pdf == 0.0f || std::isinf(Record.Pdf))
	{
		return Color3f(0.0f);
	}

	/* Transform the integration variable from the position domain to solid angle domain */
	return m_Radiance * 
		(1.0f / (Record.Distance * Record.Distance));
}

float PointLight::Pdf(const EmitterQueryRecord & Record) const
{
	return 0.0f;
}

Color3f PointLight::Eval(const EmitterQueryRecord & Record) const
{
	return Color3f(0.0f);
}

std::string PointLight::ToString() const
{
	return tfm::format(
		"PointLight=[radiance = %s, position = %s]",
		m_Radiance.ToString(),
		m_Position.ToString()
	);
}

NAMESPACE_END


