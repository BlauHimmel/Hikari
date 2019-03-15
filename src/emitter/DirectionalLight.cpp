#include <emitter/DirectionalLight.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(DirectionalLight, XML_EMITTER_DIRECTIONAL_LIGHT);

DirectionalLight::DirectionalLight(const PropertyList & PropList)
{
	/* Amount of power per unit area received by a hypothetical surface normal to the speciied direction */
	m_Irradiance = PropList.GetColor(XML_EMITTER_DIRECTIONAL_LIGHT_IRRADIANCE);

	/* Direction of the emitter */
	m_Direction = PropList.GetVector(XML_EMITTER_DIRECTIONAL_LIGHT_DIRECTION);

	m_Type = EEmitterType::EDirectional;
}

Color3f DirectionalLight::Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const
{
	return Color3f();
}

float DirectionalLight::Pdf(const EmitterQueryRecord & Record) const
{
	return 0.0f;
}

Color3f DirectionalLight::Eval(const EmitterQueryRecord & Record) const
{
	return Color3f();
}

std::string DirectionalLight::ToString() const
{
	return tfm::format(
		"DirectionalLight[irradiance = %s, direction = %s]",
		m_Irradiance.ToString(),
		m_Direction.ToString()
	);
}

NAMESPACE_END

