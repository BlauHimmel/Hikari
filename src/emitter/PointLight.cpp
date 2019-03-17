#include <emitter\PointLight.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PointLight, XML_EMITTER_POINT_LIGHT);

PointLight::PointLight(const PropertyList & PropList)
{
	m_Power = PropList.GetColor(XML_EMITTER_POINT_LIGHT_POWER);
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

	return m_Power * (1.0f / ((4.0f * float(M_PI)) * (Record.Distance * Record.Distance)));
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
		"PointLight[power = %s, position = %s]",
		m_Power.ToString(),
		m_Position.ToString()
	);
}

NAMESPACE_END


