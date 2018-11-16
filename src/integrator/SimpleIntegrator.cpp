#include <integrator\SimpleIntegrator.hpp>
#include <core\Mesh.hpp>
#include <core\Scene.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(SimpleIntegrator, XML_INTEGRATOR_SIMPLE);

SimpleIntegrator::SimpleIntegrator(const PropertyList & PropList)
{
	m_Position = PropList.GetPoint(XML_INTEGRATOR_SIMPLE_POSITION);
	m_Energy = PropList.GetColor(XML_INTEGRATOR_SIMPLE_ENERGY);
}

Color3f SimpleIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	/* Find the surface that is visible in the requested direction */
	Intersection Isect;
	if (!pScene->RayIntersect(Ray, Isect))
	{
		return Color3f(0.0f);
	}

	Ray3f ShadowRay = Isect.SpawnShadowRay(m_Position);
	if (pScene->ShadowRayIntersect(ShadowRay))
	{
		return Color3f(0.0f);
	}

	float CosTheta = Frame::CosTheta(Isect.ShadingFrame.ToLocal(ShadowRay.Direction).normalized());

	/* Return the component-wise absolute value of the shading normal as a color */
	return m_Energy / (4.0f * float(M_PI * M_PI)) * std::max(0.0f, CosTheta) / ShadowRay.Direction.squaredNorm();
}

std::string SimpleIntegrator::ToString() const
{
	return tfm::format(
		"SimpleIntegrator[\n"
		"  position = %s\n"
		"  energy = %s\n"
		"]",
		m_Position.ToString(),
		m_Energy.ToString()
	);
}

NAMESPACE_END