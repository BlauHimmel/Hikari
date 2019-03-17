#include <emitter\EnvironmentLight.hpp>
#include <core\DiscretePDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(EnvironmentLight, XML_EMITTER_ENVIRONMENT_LIGHT);

EnvironmentLight::EnvironmentLight(const PropertyList & PropList)
{
	m_Scale = PropList.GetFloat(XML_EMITTER_ENVIRONMENT_LIGHT_SCALE, DEFAULT_EMITTER_ENVIRONMENT_SCALE);
	m_ToWorld = PropList.GetTransform(XML_EMITTER_ENVIRONMENT_LIGHT_TO_WORLD, DEFAULT_EMITTER_ENVIRONMENT_TO_WORLD);
	m_Type = EEmitterType::EEnvironment;
	filesystem::path Filename = GetFileResolver()->resolve(PropList.GetString(XML_EMITTER_ENVIRONMENT_LIGHT_FILENAME));
	m_Name = Filename.str();

	m_pEnvironmentMap = new Bitmap(m_Name);
	m_ToLocal = m_ToWorld.Inverse();

	std::vector<float> Luminance(m_pEnvironmentMap->size());
	
	for (std::ptrdiff_t y = 0; y < m_pEnvironmentMap->rows(); y++)
	{
		float Theta = float(y + 0.5f) / m_pEnvironmentMap->rows() * float(M_PI);
		float SinTheta = std::sin(Theta);
		for (std::ptrdiff_t x = 0; x < m_pEnvironmentMap->cols(); x++)
		{
			// Ref : PBRT P845-850
			Luminance[y * m_pEnvironmentMap->cols() + x] = m_pEnvironmentMap->coeff(y, x).GetLuminance() * SinTheta;
		}
	}

	m_pPdf = new DiscretePDF2D(Luminance.data(), int(m_pEnvironmentMap->cols()), int(m_pEnvironmentMap->rows()));
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
	Point2i Idx;
	Point2f UV = m_pPdf->SampleContinuous(Sample2D, &MapPdf, &Idx);
	
	float Theta = UV.y() * float(M_PI);
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
		return Color3f(0.0f);
	}

	float Phi = UV.x() * float(2.0 * M_PI);
	Record.Wi = m_ToWorld * SphericalDirection(Theta, Phi);
	Record.N = -Record.Wi;
	Record.pEmitter = this;
	Record.P = Record.Ref + Record.Wi * Record.Distance * 2.0f;
	Color3f Radiance = m_pEnvironmentMap->coeff(Idx.y(), Idx.x());
	if (!Radiance.IsValid())
	{
		return Color3f(0.0f);
	}
	return Radiance * m_Scale / Record.Pdf;
}

float EnvironmentLight::Pdf(const EmitterQueryRecord & Record) const
{
	// Ref : PBRT P845-850
	Point2f Spherical = SphericalCoordinates(m_ToLocal * Record.Wi);
	float Theta = Spherical.x();
	float Phi = Spherical.y();
	float SinTheta = std::sin(Theta);
	if (SinTheta == 0.0f)
	{
		return 0.0f;
	}
	return m_pPdf->Pdf(Point2f(Phi / float(2.0 * M_PI), Theta / float(M_PI))) / (2.0f * float(M_PI * M_PI) * SinTheta);
}

Color3f EnvironmentLight::Eval(const EmitterQueryRecord & Record) const
{
	Point2f Spherical = SphericalCoordinates(m_ToLocal * Record.Wi);
	float Theta = Spherical.x();
	float Phi = Spherical.y();

	if (std::isnan(Theta) || std::isnan(Phi))
	{
		return Color3f(0.0f);
	}

	float X = Clamp(Phi / float(2.0 * M_PI) * m_pEnvironmentMap->cols(), 0.0f, float(m_pEnvironmentMap->cols() - 1.0f));
	float Y = Clamp(Theta / float(M_PI) * m_pEnvironmentMap->rows(), 0.0f, float(m_pEnvironmentMap->rows() - 1.0f));
	Color3f Radiance = m_pEnvironmentMap->coeff(int(Y), int(X));
	if (!Radiance.IsValid())
	{
		return Color3f(0.0f);
	}
	return Radiance * m_Scale;
}

std::string EnvironmentLight::ToString() const
{
	return tfm::format(
		"EnvironmentLight[\n"
		"  filename = %s,\n"
		"  scale = %f,\n"
		"  toWorld = %s\n"
		"]",
		m_Name,
		m_Scale,
		Indent(m_ToWorld.ToString(), 12)
	);
}

NAMESPACE_END