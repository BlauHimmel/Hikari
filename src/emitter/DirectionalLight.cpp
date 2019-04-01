#include <emitter/DirectionalLight.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(DirectionalLight, XML_EMITTER_DIRECTIONAL_LIGHT);

DirectionalLight::DirectionalLight(const PropertyList & PropList)
{
	/* Amount of power per unit area received by a hypothetical surface normal to the speciied direction */
	m_Power = PropList.GetColor(XML_EMITTER_DIRECTIONAL_LIGHT_POWER);

	/* Direction of the emitter */
	m_Direction = PropList.GetVector(XML_EMITTER_DIRECTIONAL_LIGHT_DIRECTION);

	m_Type = EEmitterType::EDirectional;
}

Color3f DirectionalLight::Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const
{
	Record.Wi = -m_Direction.normalized();
	Record.P = Record.Ref + (2.0f * Record.Distance) * Record.Wi;
	Record.N = -Record.Wi;
	Record.pEmitter = this;
	Record.Pdf = 1.0f;

	return m_Power;
}

float DirectionalLight::Pdf(const EmitterQueryRecord & Record) const
{
	return 1.0f;
}

Color3f DirectionalLight::Eval(const EmitterQueryRecord & Record) const
{
	return m_Power;
}

std::string DirectionalLight::ToString() const
{
	return tfm::format(
		"DirectionalLight[power = %s, direction = %s]",
		m_Power.ToString(),
		m_Direction.ToString()
	);
}

NAMESPACE_END
