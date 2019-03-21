#include <emitter\ConstantLight.hpp>
#include <core\Frame.hpp>
#include <core\Sampling.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(ConstantLight, XML_EMITTER_CONSTANT_LIGHT);

ConstantLight::ConstantLight(const PropertyList & PropList)
{
	m_Radiance = PropList.GetColor(XML_EMITTER_CONSTANT_LIGHT_RADIANCE);

	m_Type = EEmitterType::EEnvironment;
}

Color3f ConstantLight::Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const
{
	float Theta = Sample2D.y() * float(M_PI);
	float SinTheta = std::sin(Theta);

	if (SinTheta != 0.0f)
	{
		Record.Pdf = 1.0f / (2.0f * float(M_PI * M_PI) * SinTheta);
	}
	else
	{
		return Color3f(0.0f);
	}

	float Phi = Sample2D.x() * float(2.0 * M_PI);
	Record.Wi = SphericalDirection(Theta, Phi);
	Record.N = -Record.Wi;
	Record.pEmitter = this;
	Record.P = Record.Ref + Record.Wi * Record.Distance * 2.0f;

	return m_Radiance / Record.Pdf;
}

float ConstantLight::Pdf(const EmitterQueryRecord & Record) const
{
	Point2f Spherical = SphericalCoordinates(Record.Wi);
	float Theta = Spherical.x();
	float SinTheta = std::sin(Theta);

	if (SinTheta == 0.0f)
	{
		return 0.0f;
	}

	return 1.0f / (2.0f * float(M_PI * M_PI) * SinTheta);
}

Color3f ConstantLight::Eval(const EmitterQueryRecord & Record) const
{
	return m_Radiance;
}

std::string ConstantLight::ToString() const
{
	return tfm::format(
		"ConstantLight[radiance = %s]",
		m_Radiance.ToString()
	);
}

NAMESPACE_END

