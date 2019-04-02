#include <bsdf\DiffuseBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampling.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(DiffuseBSDF, XML_BSDF_DIFFUSE);

DiffuseBSDF::DiffuseBSDF(const PropertyList & PropList)
{
	m_pAlbedo = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_DIFFUSE_ALBEDO, DEFAULT_BSDF_DIFFUSE_ALBEDO));
}

DiffuseBSDF::~DiffuseBSDF()
{
	delete m_pAlbedo;
}

Color3f DiffuseBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	if (Frame::CosTheta(Record.Wi) <= 0)
	{
		return Color3f(0.0f);
	}

	Record.Measure = EMeasure::ESolidAngle;

	/* Warp a uniformly distributed sample on [0,1]^2
	to a direction on a cosine-weighted hemisphere */
	Record.Wo = Sampling::SquareToCosineHemisphere(Sample);

	/* Relative index of refraction: no change */
	Record.Eta = 1.0f;

	/* Eval() / Pdf() * Cos(Theta) = Albedo. There
	is no need to call these functions. */
	return m_pAlbedo->Eval(Record.Isect);
}

Color3f DiffuseBSDF::Eval(const BSDFQueryRecord & Record) const
{
	/* This is a smooth BRDF -- return zero if the measure
	is wrong, or when queried for illumination on the backside */
	if (Record.Measure != EMeasure::ESolidAngle || Frame::CosTheta(Record.Wi) <= 0 || Frame::CosTheta(Record.Wo) <= 0)
	{
		return Color3f(0.0f);
	}

	/* The BRDF is simply the albedo / pi */
	return m_pAlbedo->Eval(Record.Isect) * INV_PI;
}

float DiffuseBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	/* This is a smooth BRDF -- return zero if the measure
	is wrong, or when queried for illumination on the backside */
	if (Record.Measure != EMeasure::ESolidAngle || Frame::CosTheta(Record.Wi) <= 0 || Frame::CosTheta(Record.Wo) <= 0)
	{
		return 0.0f;
	}

	/* Importance sampling density with respect to solid angles:
	Cos(Theta) / pi.
	Note that the directions in 'Record' are in local coordinates,
	so Frame::CosTheta() actually just returns the 'z' component.
	*/
	return float(INV_PI) * Frame::CosTheta(Record.Wo);
}

bool DiffuseBSDF::IsDiffuse() const
{
	return true;
}

void DiffuseBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_DIFFUSE_ALBEDO)
	{
		if (m_pAlbedo != nullptr)
		{
			m_pAlbedo = (Texture *)(pChildObj);
			if (m_pAlbedo->IsMonochromatic())
			{
				LOG(WARNING) << "Albedo texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("DiffuseBSDF: tried to specify multiple albedo texture");
		}
	}
	else
	{
		throw HikariException("DiffuseBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
}

std::string DiffuseBSDF::ToString() const
{
	return tfm::format(
		"Diffuse[\n"
		"  albedo = %s\n"
		"]", 
		m_pAlbedo->IsConstant() ? m_pAlbedo->GetAverage().ToString() : Indent(m_pAlbedo->ToString())
	);
}

NAMESPACE_END
