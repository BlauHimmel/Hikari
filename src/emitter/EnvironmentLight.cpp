#include <emitter\EnvironmentLight.hpp>
#include <core\DiscretePDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(EnvironmentLight, XML_EMITTER_ENVIRONMENT_LIGHT);

EnvironmentLight::EnvironmentLight(const PropertyList & PropList)
{
	m_Name = PropList.GetString(XML_EMITTER_ENVIRONMENT_LIGHT_FILENAME);
	m_Scale = PropList.GetFloat(XML_EMITTER_ENVIRONMENT_LIGHT_SCALE, DEFAULT_EMITTER_ENVIRONMENT_SCALE);
	m_ToWorld = PropList.GetTransform(XML_EMITTER_ENVIRONMENT_LIGHT_TO_WORLD, DEFAULT_EMITTER_ENVIRONMENT_TO_WORLD);
	m_Type = EEmitterType::EEnvironment;

	m_pEnvironmentMap = new Bitmap(m_Name);
	m_pEnvironmentMap->data();

	std::unique_ptr<float[]> pLuminance(new float[m_pEnvironmentMap->size()]);
	
	for (std::ptrdiff_t y = 0; y < m_pEnvironmentMap->rows(); y++)
	{
		for (std::ptrdiff_t x = 0; x < m_pEnvironmentMap->cols(); x++)
		{
			pLuminance[y * m_pEnvironmentMap->cols() + x] = m_pEnvironmentMap->coeff(y, x).GetLuminance();
		}
	}

	m_pPdf = new DiscretePDF2D(pLuminance.get(), m_pEnvironmentMap->cols(), m_pEnvironmentMap->rows());
}

EnvironmentLight::~EnvironmentLight()
{
	delete m_pEnvironmentMap;
	delete m_pPdf;
}

Color3f EnvironmentLight::Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const
{
	return Color3f();
}

float EnvironmentLight::Pdf(const EmitterQueryRecord & Record) const
{
	return 0.0f;
}

Color3f EnvironmentLight::Eval(const EmitterQueryRecord & Record) const
{
	return Color3f(0.0f);
}

std::string EnvironmentLight::ToString() const
{
	return tfm::format(
		"EnvironmentLight["
		"  filename = %s,\n"
		"  scale = %f,\n"
		"  toWorld = %s\n"
		"]",
		m_Name,
		m_Scale,
		Indent(m_ToWorld.ToString())
	);
}

NAMESPACE_END