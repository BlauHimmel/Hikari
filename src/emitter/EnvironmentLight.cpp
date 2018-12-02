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
			// Ref : PBRT P845-850
			float Theta = float(M_PI) - y / m_pEnvironmentMap->rows() * float(M_PI);
			pLuminance[y * m_pEnvironmentMap->cols() + x] = m_pEnvironmentMap->coeff(y, x).GetLuminance() * std::sin(Theta);
		}
	}

	m_pPdf = new DiscretePDF2D(pLuminance.get(), int(m_pEnvironmentMap->cols()), int(m_pEnvironmentMap->rows()));
}

EnvironmentLight::~EnvironmentLight()
{
	delete m_pEnvironmentMap;
	delete m_pPdf;
}

Color3f EnvironmentLight::Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const
{
	// Ref : PBRT P845-850
	float MapPdf;
	Point2i Idx = m_pPdf->Sample(Sample2D, MapPdf);
	float Theta = float(M_PI) - Idx.y() / m_pEnvironmentMap->rows() * float(M_PI);
	float SinTheta = std::sin(Theta);

	if (MapPdf == 0.0f)
	{
		return Color3f(0.0f);
	}

	if (SinTheta != 0.0f)
	{
		Record.Pdf = MapPdf / (2.0f * float(M_PI * M_PI) * SinTheta);
	}
	else
	{
		Record.Pdf = 0.0f;
	}

	float Phi = Idx.x() / m_pEnvironmentMap->cols() * float(2.0 * M_PI);
	Record.Wi = m_ToWorld * SphericalDirection(Theta, Phi);
	Record.N = -Record.Wi;
	Record.pEmitter = this;
	Record.Distance = INFINITY * 0.45f;
	Record.P = Record.Ref + Record.Wi * INFINITY * 0.45f;

	return m_pEnvironmentMap->coeff(Idx.y(), Idx.x()) * m_Scale / Record.Pdf;
}

float EnvironmentLight::Pdf(const EmitterQueryRecord & Record) const
{
	// Ref : PBRT P845-850
	Point2f Spherical = SphericalCoordinates(Record.Wi);
	float Theta = Spherical.x();
	float Phi = Spherical.y();
	float SinTheta = std::sin(Theta);
	if (SinTheta == 0.0f)
	{
		return 0.0f;
	}
	float X = Phi / float(2.0 * M_PI) * m_pEnvironmentMap->cols();
	float Y = m_pEnvironmentMap->rows() - Theta / float(M_PI) * m_pEnvironmentMap->rows();
	return m_pPdf->Pdf(Point2i(int(X), int(Y))) / (2.0f * float(M_PI * M_PI) * SinTheta);;
}

Color3f EnvironmentLight::Eval(const EmitterQueryRecord & Record) const
{
	Point2f Spherical = SphericalCoordinates(Record.Wi);
	float Theta = Spherical.x();
	float Phi = Spherical.y();
	float X = Phi / float(2.0 * M_PI) * m_pEnvironmentMap->cols();
	float Y = m_pEnvironmentMap->rows() - Theta / float(M_PI) * m_pEnvironmentMap->rows();
	return m_pEnvironmentMap->coeff(int(Y), int(X)) * m_Scale;
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